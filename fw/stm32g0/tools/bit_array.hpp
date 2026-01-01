/**
 * @file
 */

#ifndef TOOLS_BIT_ARRAY_HPP_
#define TOOLS_BIT_ARRAY_HPP_

#include <cstddef>
#include <cstdint>



class BitArrayReader
{
public:
    using Storage = std::uint32_t;
    static constexpr const inline std::size_t BITS_PER_WORD = sizeof(Storage) * 8ul;

    BitArrayReader(const Storage * pos):
        pos_(pos),
        current_(*pos),
        remaining_bits_(BITS_PER_WORD)
    { }

    bool isSet() const
    {
        return current_ & static_cast<Storage>(0x1);
    }

    void next()
    {
        if (0 == (--remaining_bits_))
        {
            current_ = *(++pos_);
            remaining_bits_ = BITS_PER_WORD;
        }
        else
        {
            current_ >>= 1;
        }
    }

private:
    const Storage * pos_;
    Storage current_;
    std::uint8_t remaining_bits_;
};


template <std::size_t SZ>
class BitArray
{
public:
    using Storage = BitArrayReader::Storage;
    static constexpr const inline std::size_t SIZE = SZ;
    static constexpr const inline std::size_t FRACTIONAL_BITS = 5ul;
    static constexpr const inline std::size_t FRACTIONAL_BITS_MASK = (1ul << FRACTIONAL_BITS) - 1;
    static constexpr const inline std::size_t BITS_PER_WORD = 1ul << FRACTIONAL_BITS;
    static constexpr const inline std::size_t WORD_SIZE = (SIZE >> FRACTIONAL_BITS) +
            (0 != (SIZE & FRACTIONAL_BITS_MASK) ? 1ul : 0ul);

    static_assert(BITS_PER_WORD == (sizeof(Storage) * 8), "Check FRACTIONAL_BITS value");

    std::size_t size() const
    {
        return SIZE;
    }

    void fill(bool value = false)
    {
        const Storage pattern = value ? ~static_cast<Storage>(0) : static_cast<Storage>(0);
        for (auto & w : storage_)
            w = pattern;
    }

    void set(std::size_t bit_no)
    {
        storage_[bit_no >> FRACTIONAL_BITS] |= static_cast<Storage>(1) << (bit_no & FRACTIONAL_BITS_MASK);
    }

    void reset(std::size_t bit_no)
    {
        storage_[bit_no >> FRACTIONAL_BITS] &= ~(static_cast<Storage>(1) << (bit_no & FRACTIONAL_BITS_MASK));
    }

    void set(std::size_t bit_no, bool value)
    {
        if (value)
            set(bit_no);
        else
            reset(bit_no);
    }

    void get(std::size_t bit_no) const
    {
        return 0 != ((storage_[bit_no >> FRACTIONAL_BITS] >> (bit_no & FRACTIONAL_BITS_MASK)) & 0x1ul);
    }

    BitArrayReader read() const
    {
        return BitArrayReader(storage_);
    }

private:
    Storage storage_[WORD_SIZE];
};


#endif  // TOOLS_BIT_ARRAY_HPP_

