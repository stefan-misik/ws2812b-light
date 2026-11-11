/**
 * @file
 */

#ifndef APP_RECORDS_HPP_
#define APP_RECORDS_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <span>
#include <array>

#include "driver/i2c_bus/cat24cx.hpp"


/**
 * @brief Small copy-on-write, wear-leveling store for CAT24CX EEPROM.
 *
 * Each record occupies one EEPROM page, with IDs in the range 0-63. The
 * Cat24cx driver owns the page buffer and must be configured with a page size
 * of at least RECORD_HEADER_SIZE bytes. The same driver and its configuration
 * must be used throughout initialization and subsequent operations. Do not
 * modify its buffer except through writeRecordBuffer() or start independent
 * page operations while using this store.
 *
 * ## Page format
 *
 * All integers are big-endian. The header consists of a record ID (uint8),
 * payload length (uint8), record epoch counter (uint16), and CRC16 (uint16),
 * followed by the payload. Padding is ignored; records with invalid IDs,
 * lengths beyond the page, or invalid CRCs are rejected.
 * CRC-16/CCITT-FALSE uses polynomial 0x1021, initial value 0xFFFF, no
 * reflection or final XOR. It covers the ID, length, epoch, and valid payload
 * bytes; the CRC field and padding are excluded.
 *
 * ## Initialization
 *
 * Upon initialization, the entire EEPROM is scanned and most recent record IDs
 * (based on epoch counter) are stored in a table of contents (TOC). The TOC is
 * used to access the records and is updated when a new record is written.
 *
 * ## New Record Write
 *
 * The search starts at nextPage() and skips pages referenced by the TOC,
 * wrapping while skipping occupied pages. After a free page is written, it is
 * read back and checked before the TOC is updated. Its epoch is the stored TOC
 * epoch plus one modulo 65536. A successful write increments nextPage()
 * wrapping it, if necessary. If every page is referenced, the result is
 * NO_SPACE. Verification checks the record ID, page validity, and that the
 * read-back CRC matches the CRC saved before the read; it is not a byte-for-byte
 * comparison.
 *
 * ## Operation
 *
 * Call run() until initialization or a requested operation completes. The
 * caller must also service the asynchronous EEPROM driver and its I2C bus.
 * Operation errors leave the store busy until reinitialize() is called and the
 * scan completes again. A null pointer passed to run() is an exception: it
 * returns EEPROM_ERROR without changing the state.
 */
class Records
{
public:
    using RecordId = std::uint8_t;
    using PageId = std::uint16_t;

    static constexpr std::size_t MAX_RECORDS = 64;
    static constexpr PageId INVALID_PAGE_ID = 0x8000u;
    static constexpr std::size_t RECORD_HEADER_SIZE = 6u;
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

        FIRST_ERROR_,
        ERR_BAD_RECORD_ID = FIRST_ERROR_,
        NOT_FOUND,
        NO_SPACE,
        BAD_LENGTH,
        EEPROM_ERROR,
    };

    /** @brief True during initialization, operations, and error states */
    bool isBusy() const { return State::IDLE != state_; }
    bool isError() const { return state_ >= State::FIRST_ERROR_; }
    /** @brief Return the requested or current record ID, or INVALID_RECORD_ID */
    RecordId getCurrentRecordId() const { return current_record_id_; }
    /** @brief Return the stored error, or SUCCESS when not in an error state */
    Result error() const { return toResult(state_); }

    /** @brief Return the starting page for the next write search */
    PageId nextPage() const { return next_page_; }
    /**
     * @brief Set the next write search page without range checking
     *
     * Call only while idle and pass a page less than the EEPROM page count.
     */
    void setNextPage(PageId page) { next_page_ = page; }

    /**
     * @brief Get the page ID where given record is currently stored
     *
     * @param record_id Record ID to query
     *
     * @return ID of the page where the record is stored
     * @retval INVALID_PAGE_ID Record ID is invalid or the record is not found
     */
    PageId getRecordPage(RecordId record_id) const
    {
        if (record_id >= MAX_RECORDS)
            return INVALID_PAGE_ID;
        return toc_[record_id].page_id;
    }

    /**
     * @brief Begin reading a record with given ID
     *
     * Call run() until it returns SUCCESS or an error. A missing or invalid
     * record results in NOT_FOUND.
     *
     * @param record_id Record ID to read
     *
     * @return true if the request was accepted; false if the ID is invalid or
     * the store is busy (including initialization and error states)
     */
    bool beginRead(RecordId record_id);

    /**
     * @brief Begin writing a record with given ID
     *
     * Before calling this method, ensure the data is already written to the
     * buffer obtained from writeRecordBuffer(). After calling this method, call
     * run() until it returns SUCCESS or an error code.
     *
     * @param record_id Record ID to write
     * @param data_length Length of the data to write, must not exceed the page
     * size minus the header size. Zero-length payloads are allowed.
     *
     * @return true if the request was accepted; false if the ID is invalid,
     * the store is busy, or data_length exceeds 65535. A length exceeding the
     * page payload capacity is reported later by run() as BAD_LENGTH.
     */
    bool beginWrite(RecordId record_id, std::size_t data_length);

    /**
     * @brief Begin formatting the EEPROM
     *
     * Writes 0xFF to every page and invalidates the TOC page IDs. The TOC epoch
     * values and next page pointer are not reset. Call run() until it returns
     * SUCCESS or an error code.
     *
     * @return true if the operation was started successfully, false if the
     * store is busy
     */
    bool beginFormat();

    /**
     * @brief Get the Record Data object
     *
     * Available after a successful read or verified write, until the buffer
     * is invalidated or another operation starts. The span refers to the
     * EEPROM driver's buffer and must not be accessed while it is busy or
     * outlive the driver.
     *
     * @param eeprom Non-null pointer to the Cat24cx EEPROM driver
     *
     * @return Pointer to the record data and its length
     * @retval {} Store or EEPROM driver is busy, or no record is current
     */
    std::span<const std::uint8_t> getRecordData(const driver::i2c::Cat24cx * eeprom);

    /**
     * @brief Get the Record Buffer object
     *
     * Invalidates the current record and the driver's cached page. The span
     * refers to the driver's buffer; do not access it while either the store or
     * driver is busy, and do not let it outlive the driver.
     *
     * @param eeprom Non-null pointer to the Cat24cx EEPROM driver
     *
     * @return Pointer to the record buffer and its capacity
     * @retval {} Store or EEPROM driver is busy
     */
    std::span<std::uint8_t> writeRecordBuffer(driver::i2c::Cat24cx * eeprom);

    /**
     * @brief Request a new scan, including after an error
     */
    void reinitialize()
    {
        state_ = State::UNINITIALIZED;
        sub_state_ = 0;
    }

    /**
     * @brief Advance initialization or the current request
     *
     * @param eeprom Pointer to the Cat24cx EEPROM driver, or nullptr
     * @param current_time Current time in milliseconds
     *
     * @return SUCCESS when idle, PENDING while working, or an error. A null
     * EEPROM pointer returns EEPROM_ERROR without changing the store state.
     * While the driver is busy, Error result is returned even in an error
     * state.
     */
    Result run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time);

private:
    enum class State: std::uint8_t
    {
        UNINITIALIZED = 0u,
        INITIALIZING,
        IDLE,

        // Reading
        REQUEST_READING,
        READING,

        // Writing
        REQUEST_WRITING,
        SEARCHING_PAGE,
        WRITING,
        VERIFYING,

        // Formatting
        REQUEST_FORMATTING,
        PREPARE_FORMATTING,
        FORMATTING,

        FIRST_ERROR_ = 0x80u,
    };

    std::array<TocEntry, MAX_RECORDS> toc_;

    PageId next_page_ = 0;

    /** @brief State specific sub-state value */
    std::uint32_t sub_state_ = 0;
    /** @brief State of the Records */
    State state_ = State::UNINITIALIZED;

    /**
     * @brief Current record ID being processed
     *
     * A valid ID in the IDLE state identifies the record in the EEPROM buffer.
     */
    RecordId current_record_id_ = INVALID_RECORD_ID;

    static State toState(Result error)
    {
        return static_cast<State>(
            static_cast<std::uint32_t>(State::FIRST_ERROR_) + static_cast<std::uint32_t>(error)
        );
    }

    static Result toResult(State state)
    {
        if (state < State::FIRST_ERROR_)
            return SUCCESS;
        return static_cast<Result>(
            static_cast<std::uint32_t>(state) - static_cast<std::uint32_t>(State::FIRST_ERROR_)
        );
    }
};


#endif  // APP_RECORDS_HPP_