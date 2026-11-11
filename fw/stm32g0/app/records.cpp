#include "app/records.hpp"

#include <cstring>
#include <limits>
#include <span>
#include <algorithm>

#include "tools/next_state.hpp"
#include "app/records/crc.hpp"
#include "app/main.hpp"


#define MODULE "Records: "


namespace
{

using RecordId = Records::RecordId;
using PageId = Records::PageId;
using TocEntry = Records::TocEntry;

constexpr auto INVALID_RECORD_ID = Records::INVALID_RECORD_ID;

constexpr std::size_t ID_OFFSET = 0;
constexpr std::size_t LENGTH_OFFSET = 1;
constexpr std::size_t EPOCH_OFFSET = 2;
constexpr std::size_t CRC_OFFSET = 4;

inline std::uint16_t readBigEndian(const std::uint8_t * bytes)
{
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

inline void writeBigEndian(std::uint8_t * bytes, std::uint16_t value)
{
    bytes[0] = static_cast<std::uint8_t>(value >> 8);
    bytes[1] = static_cast<std::uint8_t>(value);
}

inline RecordId readRecordId(const std::uint8_t * page)
{
    return static_cast<RecordId>(page[ID_OFFSET]);
}

inline std::size_t readRecordLength(const std::uint8_t * page)
{
    return static_cast<std::size_t>(page[LENGTH_OFFSET]);
}

inline std::uint16_t readRecordEpoch(const std::uint8_t * page)
{
    return readBigEndian(page + EPOCH_OFFSET);
}

inline std::uint16_t readRecordCrc(const std::uint8_t * page)
{
    return readBigEndian(page + CRC_OFFSET);
}

inline void writeRecordId(std::uint8_t * page, RecordId record_id)
{
    page[ID_OFFSET] = static_cast<std::uint8_t>(record_id);
}

inline void writeRecordLength(std::uint8_t * page, std::size_t length)
{
    page[LENGTH_OFFSET] = static_cast<std::uint8_t>(length);
}

inline void writeRecordEpoch(std::uint8_t * page, std::uint16_t epoch)
{
    writeBigEndian(page + EPOCH_OFFSET, epoch);
}

inline void writeRecordCrc(std::uint8_t * page, std::uint16_t crc)
{
    writeBigEndian(page + CRC_OFFSET, crc);
}

inline std::uint16_t pageCrc(std::span<const std::uint8_t> page, std::size_t length)
{
    Crc crc;
    crc.update(page.data(), CRC_OFFSET);
    crc.update(page.data() + Records::RECORD_HEADER_SIZE, length);
    return static_cast<std::uint16_t>(static_cast<Crc::Type>(crc));
}

inline bool isNewer(std::uint16_t epoch, std::uint16_t previous)
{
    // Lets assume that the higher epoch in some reasonable range is the newer
    // one, not wrapped around stale epochs.
    const auto difference = static_cast<std::uint16_t>(epoch - previous);
    return difference < 4096u;
}

inline bool isPageTaken(std::span<const TocEntry> toc, PageId page_id)
{
    return std::any_of(toc.begin(), toc.end(),
        [page_id](const TocEntry & entry)
        {
            return entry.page_id == page_id;
        });
}

RecordId isValidRecord(std::span<const std::uint8_t> page)
{
    if (page.size() < Records::RECORD_HEADER_SIZE)
        return INVALID_RECORD_ID;
    const auto record_id = readRecordId(page.data());
    if (record_id >= Records::MAX_RECORDS)
        return INVALID_RECORD_ID;
    const std::size_t length = readRecordLength(page.data());
    if ((length + Records::RECORD_HEADER_SIZE) > page.size())
        return INVALID_RECORD_ID;
    if (readRecordCrc(page.data()) != pageCrc(page, length))
        return INVALID_RECORD_ID;
    return record_id;
}

inline PageId incrementPage(const driver::i2c::Cat24cx * eeprom, PageId current)
{
    const PageId next = current + 1;
    return next >= eeprom->pageCount() ? 0 : next;
}

}  // namespace

bool Records::beginRead(RecordId record_id)
{
    if (record_id >= MAX_RECORDS)
        return false;
    if (isBusy())
        return false;
    current_record_id_ = record_id;
    sub_state_ = 0;
    state_ = State::REQUEST_READING;
    return true;
}

bool Records::beginWrite(RecordId record_id, std::size_t data_length)
{
    if (record_id >= MAX_RECORDS)
        return false;
    if (isBusy())
        return false;
    if (data_length > std::numeric_limits<std::uint16_t>::max())
        return false;
    current_record_id_ = record_id;
    sub_state_ = data_length;
    state_ = State::REQUEST_WRITING;
    return true;
}

bool Records::beginFormat()
{
    if (isBusy())
        return false;
    current_record_id_ = INVALID_RECORD_ID;
    sub_state_ = 0;
    state_ = State::REQUEST_FORMATTING;
    return true;
}

std::span<const std::uint8_t> Records::getRecordData(const driver::i2c::Cat24cx * eeprom)
{
    if (eeprom->isBusy() || isBusy())
        return {};
    if (current_record_id_ >= MAX_RECORDS)
        return {};

    // Page in the EEPROM buffer is already verified, which is implied by
    // current_record_id_ being valid and the state being READY.
    const auto page = eeprom->buffer();
    return {page.data() + RECORD_HEADER_SIZE, readRecordLength(page.data())};
}

std::span<std::uint8_t> Records::writeRecordBuffer(driver::i2c::Cat24cx * eeprom)
{
    if (eeprom->isBusy() || isBusy())
        return {};

    current_record_id_ = INVALID_RECORD_ID;
    const auto page = eeprom->writeBuffer();
    return {page.data() + RECORD_HEADER_SIZE, page.size() - RECORD_HEADER_SIZE};
}

auto Records::run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time) -> Result
{
    if (nullptr == eeprom)
        return Result::EEPROM_ERROR;
    if (State::IDLE == state_)
        return Result::SUCCESS;
    if (state_ >= State::FIRST_ERROR_)
        return toResult(state_);

    // Only allow state transitions when the EEPROM is not busy.
    if (eeprom->isBusy())
        return Result::PENDING;

    Result result = Result::PENDING;
    while (true)
    {
        NextState<State> next_state{};
        switch (state_)
        {
        case State::UNINITIALIZED:
            for (auto & entry : toc_)
                entry.page_id = INVALID_PAGE_ID;
            if (!eeprom->resetPage())
            {
                logger().log<Logger::Level::ERROR, MODULE "Failed to reset EEPROM pages">();
                next_state.yieldTo(toState(Result::EEPROM_ERROR));
                break;
            }
            current_record_id_ = INVALID_RECORD_ID;
            sub_state_ = 0;
            next_state.yieldTo(State::INITIALIZING);
            break;

        case State::INITIALIZING:
            if (sub_state_ != eeprom->page())
            {
                if (!eeprom->readPage(static_cast<std::uint16_t>(sub_state_)))
                {
                    logger().log<Logger::Level::ERROR, MODULE "Failed to read P# %u">(sub_state_);
                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
            }
            else
            {
                if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
                {
                    logger().log<Logger::Level::ERROR, MODULE "Failed to read P# %u due %u">(
                        sub_state_, static_cast<std::uint32_t>(eeprom->result()));

                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
                const auto page = eeprom->buffer();
                const auto record_id = isValidRecord(page);
                if (record_id != INVALID_RECORD_ID)
                {
                    const std::uint16_t epoch = readRecordEpoch(page.data());
                    const bool is_more_recent = INVALID_PAGE_ID == toc_[record_id].page_id ||
                        isNewer(epoch, toc_[record_id].epoch);
                    if (is_more_recent)
                        toc_[record_id] = {static_cast<PageId>(sub_state_), epoch};
                    logger().log<Logger::Level::TRACE, MODULE "Found %s R# %u on P# %u, E# %u">(
                        is_more_recent ? "new" : "stale", record_id, sub_state_, epoch);
                }
                else
                    logger().log<Logger::Level::TRACE, MODULE "Empty P# %u">(sub_state_);
                ++sub_state_;
                if (sub_state_ < eeprom->pageCount())
                    next_state.continueTo(State::INITIALIZING);
                else
                {
                    logger().log<Logger::Level::TRACE, MODULE "Init done">();
                    sub_state_ = 0;
                    current_record_id_ = INVALID_RECORD_ID;
                    next_state.yieldTo(State::IDLE);
                }
            }
            break;

        case State::IDLE:
            // Wait for a new request
            break;

        case State::REQUEST_READING:
            if (current_record_id_ >= MAX_RECORDS)
            {
                next_state.yieldTo(toState(Result::ERR_BAD_RECORD_ID));
                break;
            }
            if (toc_[current_record_id_].page_id == INVALID_PAGE_ID)
            {
                next_state.yieldTo(toState(Result::NOT_FOUND));
                break;
            }
            {
                const auto page_id = toc_[current_record_id_].page_id;
                if (eeprom->page() != page_id)
                {
                    if (!eeprom->readPage(page_id))
                    {
                        logger().log<Logger::Level::ERROR, MODULE "Failed to read P# %u">(page_id);
                        next_state.yieldTo(toState(Result::EEPROM_ERROR));
                        break;
                    }
                }
            }
            next_state.yieldTo(State::READING);
            break;

        case State::READING:
            if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
            {
                logger().log<Logger::Level::ERROR, MODULE "Failed to read P# %u due %u">(
                    eeprom->page(), static_cast<std::uint32_t>(eeprom->result()));
                next_state.yieldTo(toState(Result::EEPROM_ERROR));
                break;
            }
            {
                const auto page = eeprom->buffer();
                const auto record_id = isValidRecord(page);
                if (record_id != current_record_id_)
                {
                    logger().log<Logger::Level::ERROR, MODULE "Invalid record %u on P# %u">(
                        current_record_id_, eeprom->page());
                    current_record_id_ = INVALID_RECORD_ID;
                    next_state.yieldTo(toState(Result::NOT_FOUND));
                    break;
                }
            }
            next_state.yieldTo(State::IDLE);
            break;

        case State::REQUEST_WRITING:
            if (current_record_id_ >= MAX_RECORDS)
            {
                next_state.yieldTo(toState(Result::ERR_BAD_RECORD_ID));
                break;
            }
            if (sub_state_ > (eeprom->pageSize() - RECORD_HEADER_SIZE))
            {
                next_state.yieldTo(toState(Result::BAD_LENGTH));
                break;
            }
            // Update page header
            {
                auto page = eeprom->writeBuffer();
                writeRecordId(page.data(), current_record_id_);
                writeRecordLength(page.data(), sub_state_);
                writeRecordEpoch(page.data(), toc_[current_record_id_].epoch + 1u);
                writeRecordCrc(page.data(), pageCrc(page, sub_state_));
            }
            sub_state_ = next_page_;
            next_state.yieldTo(State::SEARCHING_PAGE);
            break;

        case State::SEARCHING_PAGE:
            if (isPageTaken(toc_, next_page_))
            {
                next_page_ = incrementPage(eeprom, next_page_);
                if (next_page_ == sub_state_)
                {
                    logger().log<Logger::Level::ERROR, MODULE "No free page found">();
                    next_state.yieldTo(toState(Result::NO_SPACE));
                    break;
                }
                next_state.yieldTo(State::SEARCHING_PAGE);
            }
            else
            {
                if (!eeprom->writePage(next_page_))
                {
                    logger().log<Logger::Level::ERROR, MODULE "Failed to write P# %u">(next_page_);
                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
                sub_state_ = 0;
                next_state.yieldTo(State::WRITING);
            }
            break;

        case State::WRITING:
            if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
            {
                logger().log<Logger::Level::ERROR, MODULE "Failed to write P# %u due %u">(
                    eeprom->page(), static_cast<std::uint32_t>(eeprom->result()));
                next_state.yieldTo(toState(Result::EEPROM_ERROR));
                break;
            }
            // Store CRC of the page for later verification
            sub_state_ = readRecordCrc(eeprom->buffer().data());
            if (!eeprom->readPage(next_page_))
            {
                logger().log<Logger::Level::ERROR, MODULE "Failed to verify P# %u">(next_page_);
                next_state.yieldTo(toState(Result::EEPROM_ERROR));
                break;
            }
            next_state.yieldTo(State::VERIFYING);
            break;

        case State::VERIFYING:
            if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
            {
                logger().log<Logger::Level::ERROR, MODULE "Failed to verify P# %u due %u">(
                    eeprom->page(), static_cast<std::uint32_t>(eeprom->result()));
                next_state.yieldTo(toState(Result::EEPROM_ERROR));
                break;
            }
            {
                const auto page = eeprom->buffer();
                const auto record_id = isValidRecord(page);
                if (record_id != current_record_id_ || readRecordCrc(page.data()) != sub_state_)
                {
                    logger().log<Logger::Level::ERROR, MODULE "Invalid record %u written to P# %u">(
                        current_record_id_, eeprom->page());
                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
            }
            // Update TOC entry
            {
                auto & entry = toc_[current_record_id_];
                entry.page_id = next_page_;
                entry.epoch = entry.epoch + 1;
            }
            next_page_ = incrementPage(eeprom, next_page_);
            logger().log<Logger::Level::TRACE, MODULE "Record %u written to P# %u">(
                current_record_id_, eeprom->page());
            next_state.yieldTo(State::IDLE);
            break;

        case State::REQUEST_FORMATTING:
            for (auto & entry : toc_)
                entry.page_id = INVALID_PAGE_ID;
            next_state.yieldTo(State::PREPARE_FORMATTING);
            break;

        case State::PREPARE_FORMATTING:
            {
                auto b = eeprom->writeBuffer();
                std::memset(b.data(), 0xFFu, b.size());
            }
            sub_state_ = 0;
            next_state.yieldTo(State::FORMATTING);
            break;

        case State::FORMATTING:
            if (sub_state_ != eeprom->page())
            {
                if (!eeprom->writePage(static_cast<std::uint16_t>(sub_state_)))
                {
                    logger().log<Logger::Level::ERROR, MODULE "Failed to write P# %u">(sub_state_);
                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
            }
            else
            {
                if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
                {
                    logger().log<Logger::Level::ERROR, MODULE "Failed to write P# %u due %u">(
                        sub_state_, static_cast<std::uint32_t>(eeprom->result()));
                    next_state.yieldTo(toState(Result::EEPROM_ERROR));
                    break;
                }
                ++sub_state_;
                if (sub_state_ < eeprom->pageCount())
                    next_state.continueTo(State::FORMATTING);
                else
                {
                    logger().log<Logger::Level::TRACE, MODULE "Format done">();
                    sub_state_ = 0;
                    current_record_id_ = INVALID_RECORD_ID;
                    next_state.yieldTo(State::IDLE);
                }
            }

        default:
            // This covers mainly error states
            break;
        }

        // Handle transitions
        if (next_state.hasValue())
            state_ = next_state;
        if (!next_state.doContinue())
        {
            if (State::IDLE == state_)
                result = Result::SUCCESS;
            else
            {
                const Result error = toResult(state_);
                result = error >= Result::FIRST_ERROR_ ? error : Result::PENDING;
            }
            break;
        }
    }

    return result;
    (void)current_time; // To avoid unused parameter warning
}