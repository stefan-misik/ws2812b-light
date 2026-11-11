/**
 * @file
 */


#ifndef APP_RECORDS_CRC_HPP_
#define APP_RECORDS_CRC_HPP_

#include <cstddef>
#include <cstdint>



/**
 * @brief Class to calculate CRC-16/CCITT-FALSE checksum
 */
class Crc
{
public:
    using Type = std::uint_fast16_t;

    static constexpr Type INITIAL_VALUE = 0xFFFFu;

    constexpr Crc(const Crc &) = default;
    constexpr Crc(Crc &&) = default;
    constexpr Crc & operator =(const Crc &) = default;
    constexpr Crc & operator =(Crc &&) = default;

    constexpr Crc(Type initial_value = INITIAL_VALUE):
        value_(initial_value)
    { }
    constexpr operator Type() const { return value_; }

    void update(const void * data, std::size_t data_len);

private:
    Type value_;
};

#endif  // APP_RECORDS_CRC_HPP_
