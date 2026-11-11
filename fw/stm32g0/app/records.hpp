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
 * @brief Class that implements minimal wear-leveling file system on top of CAT24CX EEPROM.
 *
 * The records accesses the EEPROOM in its natural page size (64 bytes for
 * CAT24C256). It splits the EEPROM into two partitions: ToC partition and data
 * partition.
 *
 *
 * ## Common Header
 *
 * Each record in both partitions contains a header that contains:
 *
 * - UIN8 magic number (0xA5) to identify valid records
 * - UIN8 length of valid data in the record
 * - UIN16 CRC16 of the header and valid data in the record (crc is calculated
 *   as 0xFFFF when calculating CRC16)
 *
 *
 * ## ToC partition
 *
 * Table of contents partition contains the first ToC entry record which is
 * rotated on each write. The record within this partition contains header with
 * the a 2-byte unsigned epoch counter that is incremented on each write and is
 * used to identify the latest ToC entry. The rest of the record contains the
 * list of ToC entries that point to the data partition.
 *
 * First several records are reserved for continuation of the ToC, these records
 * are stored in the data partition and do not contain the epoch counter.
 *
 * All ToC entries consist of array of 3 byte structures:
 * - UIN16 page_id - page id of the record in the data partition
 * - UIN8 record_id - id of the record
 *
 *
 * ## Data partition
 *
 * Contains the mix of user data and continuation of the ToC entries. Each
 * record in this partition contains.
 *
 *
 * ## Algorithm
 *
 * When the application starts the first ToC entry is located by scanning the
 * ToC partition. When the latest ToC entry is found, the rest of the ToC
 * entries are read from the data partition. When the application asks for a
 * record, it is located using the cached ToC. When application asks the record,
 * a new unused page is located using the cached ToC. The class holds the
 * pointer to the most recently written page, and it always starts searching for
 * the next free page from the next page.
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