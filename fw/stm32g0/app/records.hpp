/**
 * @file
 */

#ifndef APP_RECORDS_HPP_
#define APP_RECORDS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

#include "driver/i2c_bus/cat24cx.hpp"


/**
 * @brief Asynchronous, copy-on-write record store for a CAT24CX EEPROM.
 *
 * Each page contains a record ID (one byte), payload length (one byte),
 * per-record epoch (two bytes), CRC (two bytes), and the payload. Multibyte
 * fields are big-endian. The CRC is CRC-16/CCITT-FALSE over the header and
 * payload, with the CRC field treated as 0xFFFF; padding is ignored.
 * IDs range from 0 to 63. Empty payloads are valid.
 *
 * Call run() until initialization completes. It scans every page and builds
 * the table of contents; invalid pages are ignored. Epochs wrap at 65536 and
 * are ordered using modular differences. Ordering is ambiguous when surviving
 * versions of the same ID differ by 32768 or more writes; equal and exactly
 * half-range epochs retain the first scanned page.
 *
 * The next-page cursor initially points to page zero. After initialization an
 * application may restore a saved cursor with setNextPage(). It is only an
 * allocation hint: pages referenced by the table of contents are never
 * overwritten. A successful, verified write advances it to the following page.
 * Persisting the cursor is the application's responsibility.
 *
 * Use one configured EEPROM exclusively with this object for its lifetime.
 * Do not reconfigure it or use its page buffer or operations independently.
 * Service the EEPROM and I2C bus separately while repeatedly calling run().
 * EEPROM write errors and verification errors have an uncertain outcome:
 * run() returns an error and a later call starts a full rescan before further
 * operations can begin. A failed write may be found committed during rescan.
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

        ERR_BAD_RECORD_ID,
        NOT_FOUND,
        ERR_NO_SPACE,
        EEPROM_ERROR,
        ERR_BAD_CONFIG,
        ERR_BAD_DATA_LENGTH,
        ERR_CORRUPT_RECORD,
        ERR_WRITE_VERIFY,
    };

    Records() = default;
    Records(const Records &) = delete;
    Records & operator =(const Records &) = delete;
    Records(Records &&) = delete;
    Records & operator =(Records &&) = delete;

    bool isBusy() const { return State::READY != state_; }
    RecordId getCurrentRecordId() const { return current_record_id_; }

    /** @brief Get the committed allocation hint, including while busy. */
    PageId getNextPage() const { return next_page_; }

    /**
     * @brief Restore an allocation hint after initialization.
     * @return False if busy, uninitialized, or the page is out of range.
     */
    bool setNextPage(PageId page);

    /**
     * @brief Begin reading a record. Call run() until it is no longer PENDING.
     * @return False if the ID is invalid or the store or EEPROM is busy.
     */
    bool beginRead(RecordId record_id);

    /**
     * @brief Begin writing a record from the payload buffer.
     *
     * Obtain and fill the buffer with getRecordBuffer() before calling this.
     * Call run() until it is no longer PENDING. A length larger than the
     * configured page payload is reported by run() as ERR_BAD_DATA_LENGTH.
     *
     * @return False if the ID or absolute length is invalid, or either device
     * is busy.
     */
    bool beginWrite(RecordId record_id, std::size_t data_length);

    /**
     * @brief Get the payload from the last successful read or verified write.
     *
     * Returns {} when unavailable or busy. An empty record has a non-null
     * pointer and zero length. Pointers remain valid only until the next
     * operation or mutable buffer access and must not outlive the EEPROM.
     */
    std::pair<const void *, std::size_t> getRecordData(driver::i2c::Cat24cx * eeprom);

    /**
     * @brief Get the writable payload area and its capacity while idle.
     *
     * Returns {} when unavailable or busy. This invalidates previous record
     * data; the pointer must not be accessed once an operation has started.
     */
    std::pair<void *, std::size_t> getRecordBuffer(driver::i2c::Cat24cx * eeprom);

    /** @brief Advance initialization or an operation without servicing the bus. */
    Result run(driver::i2c::Cat24cx * eeprom, std::uint32_t current_time);

private:
    /** @brief Maximum candidate pages examined per run() call. */
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
        VERIFYING,
    };

    bool matchesEeprom(const driver::i2c::Cat24cx * eeprom) const;

    std::array<TocEntry, MAX_RECORDS> toc_{};
    std::array<std::uint8_t, driver::i2c::Cat24cx::MAX_PAGE_SIZE> expected_page_{};
    driver::i2c::Cat24cx * eeprom_ = nullptr;
    driver::I2cBus::Address eeprom_address_;
    std::uint32_t page_count_ = 0;
    std::size_t page_size_ = 0;
    std::size_t data_length_ = 0;
    std::uint32_t scan_page_ = 0;
    std::uint32_t pages_examined_ = 0;
    PageId next_page_ = 0;
    PageId candidate_page_ = 0;
    std::uint16_t pending_epoch_ = 0;
    State state_ = State::UNINITIALIZED;
    RecordId current_record_id_ = INVALID_RECORD_ID;
};

#endif  // APP_RECORDS_HPP_