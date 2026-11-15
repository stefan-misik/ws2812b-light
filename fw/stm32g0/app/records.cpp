#include "app/records.hpp"

#include <algorithm>
#include <span>

#include "app/records/crc.hpp"


namespace
{

constexpr std::size_t ID_OFFSET = 0;
constexpr std::size_t LENGTH_OFFSET = 1;
constexpr std::size_t EPOCH_OFFSET = 2;
constexpr std::size_t CRC_OFFSET = 4;

std::uint16_t readBigEndian(const std::uint8_t * bytes)
{
    return static_cast<std::uint16_t>((static_cast<std::uint16_t>(bytes[0]) << 8) | bytes[1]);
}

void writeBigEndian(std::uint8_t * bytes, std::uint16_t value)
{
    bytes[0] = static_cast<std::uint8_t>(value >> 8);
    bytes[1] = static_cast<std::uint8_t>(value);
}

std::uint16_t pageCrc(std::span<const std::uint8_t> page, std::size_t length)
{
    static constexpr std::uint8_t EMPTY_CRC[] = {0xffu, 0xffu};
    Crc crc;
    crc.update(page.data(), CRC_OFFSET);
    crc.update(EMPTY_CRC, sizeof(EMPTY_CRC));
    crc.update(page.data() + Records::RECORD_HEADER_SIZE, length);
    return static_cast<std::uint16_t>(static_cast<Crc::Type>(crc));
}

bool validPage(std::span<const std::uint8_t> page)
{
    if (page.size() < Records::RECORD_HEADER_SIZE || page[ID_OFFSET] >= Records::MAX_RECORDS)
        return false;
    const std::size_t length = page[LENGTH_OFFSET];
    if (length > page.size() - Records::RECORD_HEADER_SIZE)
        return false;
    return readBigEndian(page.data() + CRC_OFFSET) == pageCrc(page, length);
}

bool isNewer(std::uint16_t epoch, std::uint16_t previous)
{
    const auto difference = static_cast<std::uint16_t>(epoch - previous);
    return difference != 0 && difference < 0x8000u;
}

}  // namespace


bool Records::matchesEeprom(const driver::i2c::Cat24cx * eeprom) const
{
    return nullptr != eeprom && eeprom == eeprom_ && eeprom->pageSize() == page_size_ &&
            eeprom->pageCount() == page_count_ && eeprom->address() == eeprom_address_;
}

bool Records::setNextPage(PageId page)
{
    if (isBusy() || !matchesEeprom(eeprom_) || eeprom_->isBusy() || page >= page_count_)
        return false;
    next_page_ = page;
    return true;
}

bool Records::beginRead(RecordId record_id)
{
    if (record_id >= MAX_RECORDS || isBusy() || !matchesEeprom(eeprom_) || eeprom_->isBusy())
        return false;
    current_record_id_ = record_id;
    state_ = State::REQUEST_READING;
    return true;
}

bool Records::beginWrite(RecordId record_id, std::size_t data_length)
{
    if (record_id >= MAX_RECORDS || isBusy() || !matchesEeprom(eeprom_) || eeprom_->isBusy() ||
            data_length > driver::i2c::Cat24cx::MAX_PAGE_SIZE - RECORD_HEADER_SIZE)
        return false;
    current_record_id_ = record_id;
    data_length_ = data_length;
    state_ = State::REQUEST_WRITING;
    return true;
}

std::pair<const void *, std::size_t> Records::getRecordData(driver::i2c::Cat24cx * eeprom)
{
    if (isBusy() || !matchesEeprom(eeprom) || eeprom->isBusy() || current_record_id_ >= MAX_RECORDS)
        return {};
    const TocEntry & entry = toc_[current_record_id_];
    const auto page = eeprom->buffer();
    if (entry.page_id == INVALID_PAGE_ID || !validPage(page) || page[ID_OFFSET] != current_record_id_ ||
            readBigEndian(page.data() + EPOCH_OFFSET) != entry.epoch)
        return {};
    return {page.data() + RECORD_HEADER_SIZE, page[LENGTH_OFFSET]};
}

std::pair<void *, std::size_t> Records::getRecordBuffer(driver::i2c::Cat24cx * eeprom)
{
    if (isBusy() || !matchesEeprom(eeprom) || eeprom->isBusy())
        return {};
    current_record_id_ = INVALID_RECORD_ID;
    const auto page = eeprom->buffer();
    return {page.data() + RECORD_HEADER_SIZE, page.size() - RECORD_HEADER_SIZE};
}

auto Records::run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time) -> Result
{
    (void)current_time;  // The caller independently services the EEPROM driver.

    if (nullptr == eeprom || (nullptr != eeprom_ && !matchesEeprom(eeprom)))
        return ERR_BAD_CONFIG;

    if (nullptr == eeprom_)
    {
        const std::size_t page_size = eeprom->pageSize();
        if (page_size < RECORD_HEADER_SIZE || page_size > driver::i2c::Cat24cx::MAX_PAGE_SIZE)
            return ERR_BAD_CONFIG;
        const std::uint32_t page_count = eeprom->pageCount();
        if (0 == page_count || page_count > INVALID_PAGE_ID)
            return ERR_BAD_CONFIG;
        eeprom_ = eeprom;
        eeprom_address_ = eeprom->address();
        page_size_ = page_size;
        page_count_ = page_count;
    }

    // Nothing may inspect or modify the driver's buffer while it is busy.
    if (eeprom->isBusy())
        return PENDING;

    switch (state_)
    {
    case State::UNINITIALIZED:
        toc_.fill(TocEntry{});
        current_record_id_ = INVALID_RECORD_ID;
        scan_page_ = 0;
        if (!eeprom->readPage(scan_page_))
            return EEPROM_ERROR;
        state_ = State::INITIALIZING;
        return PENDING;

    case State::INITIALIZING:
    {
        if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
        {
            state_ = State::UNINITIALIZED;
            return EEPROM_ERROR;
        }
        const auto page = eeprom->buffer();
        if (validPage(page))
        {
            const RecordId id = page[ID_OFFSET];
            const std::uint16_t epoch = readBigEndian(page.data() + EPOCH_OFFSET);
            TocEntry & entry = toc_[id];
            if (entry.page_id == INVALID_PAGE_ID || isNewer(epoch, entry.epoch))
                entry = {static_cast<PageId>(scan_page_), epoch};
        }
        ++scan_page_;
        if (scan_page_ == page_count_)
        {
            state_ = State::READY;
            return SUCCESS;
        }
        if (!eeprom->readPage(scan_page_))
        {
            state_ = State::UNINITIALIZED;
            return EEPROM_ERROR;
        }
        return PENDING;
    }

    case State::READY:
        return SUCCESS;

    case State::REQUEST_READING:
    {
        const TocEntry & entry = toc_[current_record_id_];
        if (entry.page_id == INVALID_PAGE_ID)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return NOT_FOUND;
        }
        if (!eeprom->readPage(entry.page_id))
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return EEPROM_ERROR;
        }
        state_ = State::READING;
        return PENDING;
    }

    case State::READING:
    {
        if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return EEPROM_ERROR;
        }
        const auto page = eeprom->buffer();
        if (!validPage(page) || page[ID_OFFSET] != current_record_id_ ||
                readBigEndian(page.data() + EPOCH_OFFSET) != toc_[current_record_id_].epoch)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return ERR_CORRUPT_RECORD;
        }
        state_ = State::READY;
        return SUCCESS;
    }

    case State::REQUEST_WRITING:
        if (data_length_ > page_size_ - RECORD_HEADER_SIZE)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return ERR_BAD_DATA_LENGTH;
        }
        candidate_page_ = next_page_;
        pages_examined_ = 0;
        state_ = State::SEARCHING_PAGE;
        [[fallthrough]];

    case State::SEARCHING_PAGE:
    {
        for (std::size_t step = 0; step < MAX_SUB_STEPS && pages_examined_ < page_count_; ++step)
        {
            const auto occupied = std::any_of(toc_.begin(), toc_.end(), [this](const TocEntry & entry)
            {
                return entry.page_id == candidate_page_;
            });
            if (!occupied)
            {
                auto page = eeprom->buffer();
                const TocEntry & entry = toc_[current_record_id_];
                pending_epoch_ = entry.page_id == INVALID_PAGE_ID ? 1u :
                        static_cast<std::uint16_t>(entry.epoch + 1u);
                page[ID_OFFSET] = current_record_id_;
                page[LENGTH_OFFSET] = static_cast<std::uint8_t>(data_length_);
                writeBigEndian(page.data() + EPOCH_OFFSET, pending_epoch_);
                writeBigEndian(page.data() + CRC_OFFSET, pageCrc(page, data_length_));
                std::copy_n(page.begin(), RECORD_HEADER_SIZE + data_length_, expected_page_.begin());
                if (!eeprom->writePage(candidate_page_))
                {
                    current_record_id_ = INVALID_RECORD_ID;
                    state_ = State::READY;
                    return EEPROM_ERROR;
                }
                state_ = State::WRITING;
                return PENDING;
            }
            ++pages_examined_;
            candidate_page_ = static_cast<PageId>((static_cast<std::uint32_t>(candidate_page_) + 1u) % page_count_);
        }
        if (pages_examined_ == page_count_)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::READY;
            return ERR_NO_SPACE;
        }
        return PENDING;
    }

    case State::WRITING:
        if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS || !eeprom->readPage(candidate_page_))
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::UNINITIALIZED;
            return EEPROM_ERROR;
        }
        state_ = State::VERIFYING;
        return PENDING;

    case State::VERIFYING:
    {
        if (eeprom->result() != driver::i2c::Cat24cx::Result::SUCCESS)
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::UNINITIALIZED;
            return EEPROM_ERROR;
        }
        const auto page = eeprom->buffer();
        if (!validPage(page) || !std::equal(expected_page_.begin(),
                expected_page_.begin() + RECORD_HEADER_SIZE + data_length_, page.begin()))
        {
            current_record_id_ = INVALID_RECORD_ID;
            state_ = State::UNINITIALIZED;
            return ERR_WRITE_VERIFY;
        }
        toc_[current_record_id_] = {candidate_page_, pending_epoch_};
        next_page_ = static_cast<PageId>((static_cast<std::uint32_t>(candidate_page_) + 1u) % page_count_);
        state_ = State::READY;
        return SUCCESS;
    }
    }
    return ERR_BAD_CONFIG;
}