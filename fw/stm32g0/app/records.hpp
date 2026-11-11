/**
 * @file
 */

#ifndef APP_RECORDS_HPP_
#define APP_RECORDS_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <array>

#include "driver/i2c_bus/cat24cx.hpp"


/**
 * @brief Small copy-on-write, wear-leveling store for CAT24CX EEPROM.
 *
 * EEPROM/Flash is divided into pages of specific number of bytes per page (e.g.
 * 64). Pages are allocated in a circular manner.
 *
 * ## Page format
 *
 * All integers are big-endian. The header consists of a record ID (uint8),
 * payload length (uint8), record epoch counter (uint16), global epoch counter
 * (uint16), and CRC16 (uint16), followed by the payload. Ignore padding and
 * reject lengths beyond the page. CRC-16/CCITT-FALSE uses polynomial 0x1021,
 * initial value 0xFFFF, no reflection or final XOR. It covers the header and
 * valid payload bytes, with the CRC field set to 0xFFFF. The epoch counter is
 * incremented on every write to a record of given ID (the epoch counter needs
 * to be stored per-record ID).
 *
 * ## Initialization
 *
 * Upon initialization, the entire EEPROM is scanned and most recent record IDs
 * (based on epoch counter) are stored in a table of contents (TOC). The TOC is
 * used to access the records and is updated when a new record is written. Use
 * global epoch counter to determine the most recent record to continue writing
 * after.
 *
 * ## New Record Write
 *
 * When locating a new space for a record, start from a next page pointer and
 * verify that the page is not in use by checking it against the TOC. If the
 * page is in use, increment the next page pointer and check again. Once a free
 * page is found, write the record to it and update the TOC. The record ID
 * number is limited in range 0-63.
 *
 * ## Epoch Counter
 *
 * Epoch counter uses a 16-bit unsigned integer and can wrap around when
 * incrementing, therefore, when comparing epoch counters, the difference
 * between them should be calculated as a signed integer. For example, if the
 * current epoch is 0xFFFE and the new epoch is 0x0001, the difference is 3
 * (0x0001 - 0xFFFE = 3).
 */
class Records
{
public:
    using RecordId = std::uint8_t;
    using PageId = std::uint16_t;

    static constexpr std::size_t MAX_RECORDS = 64;
    static constexpr PageId INVALID_PAGE_ID = 0x8000u;
    static constexpr std::size_t RECORD_HEADER_SIZE = 8u;
    static constexpr RecordId INVALID_RECORD_ID = 0xFFu;


    struct TocEntry
    {
        PageId page_id = INVALID_PAGE_ID;
        std::uint16_t epoch = 0u;
    };

    enum Result: std::size_t
    {
        SUCCESS = 0u,
        PENDING,

        ERR_BAD_RECORD_ID,
        NOT_FOUND,
        ERR_NO_SPACE,
        EEPROM_ERROR,
    };

    bool isBusy() const { return State::READY != state_; }
    RecordId getCurrentRecordId() const { return current_record_id_; }

    /**
     * @brief Begin reading a record with given ID
     *
     * @param record_id Record ID to read

     * @return true if the operation was started successfully, false if the
     * record ID is invalid or the driver is busy
     */
    bool beginRead(RecordId record_id);

    /**
     * @brief Begin writing a record with given ID
     *
     * Before calling this method, ensure the data is already written to the
     * buffer obtained from getRecordBuffer(). After calling this method, call
     * run() until it returns SUCCESS or an error code.
     *
     * @param record_id Record ID to write
     * @param data_length Length of the data to write, must not exceed the page
     * size minus the header size
     *
     * @return true if the operation was started successfully, false if the
     * record ID is invalid or the driver is busy
     */
    bool beginWrite(RecordId record_id, std::size_t data_length);

    /**
     * @brief Get the Record Data object
     *
     * Make sure the read operation completed successfully before calling this
     * method by calling @ref beginRead() and then @ref run() until it returns
     * SUCCESS or an error code.
     *
     * @param eeprom Pointer to the Cat24cx EEPROM driver
     *
     * @return Pointer to the record data and its length
     * @retval {} System is busy or record not found
     */
    std::pair<const void *, std::size_t> getRecordData(driver::i2c::Cat24cx * eeprom);

    /**
     * @brief Get the Record Buffer object
     *
     * @param eeprom Pointer to the Cat24cx EEPROM driver
     *
     * @return Pointer to the record buffer and its capacity
     * @retval {} System is busy
     */
    std::pair<void *, std::size_t> getRecordBuffer(driver::i2c::Cat24cx * eeprom);

    Result run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time);

private:
    /**
     * @brief Maximum number of state's sub-steps (e.g. pages looked up when
     * searching an empty page) per @ref run() call
     */
    static constexpr std::size_t MAX_SUB_STEPS = 10u;

    enum class State: std::uint8_t
    {
        UNINITIALIZED = 0u,
        INITIALIZING,
        READY,

        REQUEST_READING,
        READING,

        REQUEST_WRITING,
        SEARCHING_PAGE,
        WRITING,
    };

    std::array<TocEntry, MAX_RECORDS> toc_;
    std::uint16_t global_epoch_ = 0u;

    PageId next_page_ = 0;

    /** @brief State specific sub-state value */
    std::uint32_t sub_state_ = 0;
    /** @brief State of the Records */
    State state_ = State::UNINITIALIZED;

    /**
     * @brief Current record ID being processed
     *
     * If the value here is a valid record ID and the @ref state_ is READY, the
     * EEPROM driver has a valid record data in its buffer.
     */
    RecordId current_record_id_ = INVALID_RECORD_ID;
};


#endif  // APP_RECORDS_HPP_