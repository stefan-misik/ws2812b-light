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
 * The configured CAT24C256 has 512 physical pages of 64 bytes. The first 128
 * pages hold rotating table-of-contents (ToC) roots; the remaining pages hold
 * user records and ToC continuations. Validate other device geometries before
 * use. Page IDs are physical page numbers.
 *
 * ## Page format
 *
 * All integers are big-endian. Each page has a four-byte header: magic 0xA5,
 * payload length (uint8), and CRC16 (uint16), followed by the payload. Ignore
 * padding and reject lengths beyond the page. CRC-16/CCITT-FALSE uses
 * polynomial 0x1021, initial value 0xFFFF, no reflection or final XOR. It
 * covers the header and valid payload bytes, with the CRC field set to 0xFFFF.
 *
 * A root payload contains version 1 (uint8), epoch (uint16), last allocated
 * data page (uint16, INVALID_PAGE_ID if none), then ToC entries. Each entry is
 * a serialized page ID (uint16) and record ID (uint8); do not write C++ struct
 * padding. Root entries with IDs 0x00-0x02 point to continuation pages. Other
 * IDs below FIRST_USER_RECORD_ID are reserved.
 *
 * A continuation payload contains its record ID followed by user ToC entries.
 * A user page contains its record ID followed by at most 59 bytes of data.
 * With 64-byte pages, a root holds 18 entries and a continuation holds 19.
 * Three continuations accommodate 64 user records; cache their pointers
 * separately from the 64 user entries. Validate IDs, page ranges and duplicates.
 *
 * ## Commit and recovery
 *
 * Free pages are those not referenced by a valid root, directly or through its
 * continuations, and not reserved by a staged update. Allocate circularly from
 * the page after the last allocated page, persisting that cursor in each root.
 * Write and read-back-verify new data and changed continuation pages first,
 * then write and verify a new root with epoch + 1. The new root is the commit
 * point; keep the old root and its pages intact until then. Deletion is a ToC
 * update.
 *
 * Retire the old root with a whole-page invalidation and verify it before
 * recycling its pages. If retirement fails, block further writes and report
 * that the update committed but recovery is required. After an uncertain write,
 * remount before reusing pages. A retired root is not a long-term backup.
 *
 * At startup, scan and validate roots. Accept one valid root, or two with
 * consecutive epochs modulo 65536; otherwise report blank storage or corruption.
 * Protect both roots until retirement is complete, validate continuations
 * before use, and check each user page's CRC and ID on read. Distinguish blank
 * storage, corruption, no space and I/O errors; formatting must be explicit.
 *
 * Serialize operations around the asynchronous EEPROM driver and its page
 * buffer. Skip unchanged writes and coalesce pending updates; root retirement
 * adds a page write per commit. This is dynamic wear leveling: cold records are
 * not relocated.
 * Assume interrupted writes affect only the targeted page; CRC is not a
 * guarantee against all corruption. Flash requires separate erase-block
 * management and is outside this design.
 *
 * Test resets at every write, including retirement, plus epoch wrap, corrupt
 * pages, full storage, I/O failures and repeated restarts.
 */
class Records
{
public:
    static constexpr std::size_t MAX_RECORDS = 64;

    using RecordId = std::uint8_t;
    using PageId = std::uint16_t;

    static constexpr PageId INVALID_PAGE_ID = 0x8000u;
    static constexpr PageId TOC_PARTITION_SIZE = 128u;  // in pages

    static constexpr RecordId FIRST_USER_RECORD_ID = 0x10u;

    struct TocEntry
    {
        PageId page_id = INVALID_PAGE_ID;
        RecordId record_id = 0;
    };

    void run(driver::i2c::Cat24cx * eeprom);

private:
    enum class State: std::uint8_t
    {
        UNINITIALIZED,
        INITIALIZING,
        READY,
    };

    std::array<TocEntry, MAX_RECORDS> toc_;

    std::uint16_t step_ = 0;
    State state_ = State::UNINITIALIZED;
};


#endif  // APP_RECORDS_HPP_