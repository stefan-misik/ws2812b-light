/**
 * @file
 */

#ifndef APP_RECORDS_HEADER_HPP_
#define APP_RECORDS_HEADER_HPP_

#include <cstddef>
#include <cstdint>


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

inline std::uint8_t readRecordId(const std::uint8_t * page)
{
    return static_cast<std::uint8_t>(page[ID_OFFSET]);
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

inline void writeRecordId(std::uint8_t * page, std::uint8_t record_id)
{
    page[ID_OFFSET] = record_id;
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


#endif  // APP_RECORDS_HEADER_HPP_
