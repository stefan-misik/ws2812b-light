#include "app/records.hpp"

#include <limits>


namespace
{

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

std::pair<const void *, std::size_t> Records::getRecordData(driver::i2c::Cat24cx * eeprom)
{
    if (eeprom->isBusy() || isBusy())
        return {};
    if (current_record_id_ >= MAX_RECORDS)
        return {};
    const TocEntry & entry = toc_[current_record_id_];
    if (entry.page_id == INVALID_PAGE_ID)
        return {};

    // TODO IMPLEMENT
    return {};
}

std::pair<void *, std::size_t> Records::getRecordBuffer(driver::i2c::Cat24cx * eeprom)
{
    if (eeprom->isBusy() || isBusy())
        return {};
    if (current_record_id_ >= MAX_RECORDS)
        return {};

    // TODO IMPLEMENT
    return {};
}

auto Records::run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time) -> Result
{
    // TODO IMPLEMENT
    return Result::SUCCESS; // Placeholder return value
    (void)eeprom; // To avoid unused parameter warning
    (void)current_time; // To avoid unused parameter warning
}