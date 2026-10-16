#ifndef TOOLS_FIXED_STRING_HPP_
#define TOOLS_FIXED_STRING_HPP_

#include <cstddef>
#include <string_view>
#include <utility>


template <std::size_t N>
struct FixedString
{
    char value[N]{};

    constexpr FixedString() = default;

    constexpr FixedString(const char (&string)[N])
    {
        for (std::size_t i = 0; i < N; ++i)
        {
            value[i] = string[i];
        }
    }

    static constexpr inline std::size_t SIZE = N - 1;

    constexpr std::size_t size() const { return SIZE; }
    constexpr const char * begin() const { return value; }
    constexpr const char * end() const { return value + SIZE; }

    constexpr operator std::string_view() const
    {
        return {value, SIZE};
    }
};

// Helper to join strings at compile time form:
// https://stackoverflow.com/questions/38955940/how-to-concatenate-static-strings-at-compile-time/62823211#62823211
template <FixedString... Strs>
struct ConcatenatedFixedString
{
    static constexpr auto impl() noexcept
    {
        constexpr std::size_t len = (Strs.size() + ... + 0);
        FixedString<len + 1> arr{};
        auto append = [pos = arr.value](auto const& s) mutable
        {
            for (auto c : s)
                *(pos++) = c;
        };
        (append(Strs), ...);
        arr.value[len] = 0;
        return arr;
    }
    static constexpr auto VALUE = impl();
};


#endif  // TOOLS_FIXED_STRING_HPP_