/**
 * @file
 */

#ifndef TOOLS_FLAGS_HPP_
#define TOOLS_FLAGS_HPP_

#include <cstddef>
#include <cstdint>
#include <type_traits>


/**
 * @brief A type-safe flags class
 *
 * @tparam FLG The enum type to use as flags
 * @tparam T The underlying type to use for the flags (default: std::uint32_t)
 */
template <typename FLG, typename T = std::uint32_t>
class Flags
{
public:
    using Flag = FLG;
    using UnderlyingType = T;

    constexpr Flags() = default;
    constexpr Flags(const Flags &) = default;
    constexpr Flags & operator =(const Flags &) = default;

    template <typename... FLGs>
        requires (std::is_same_v<FLGs, Flag> && ...)
    constexpr Flags(FLGs... flags):
        flags_(toFlagBits(flags...))
    { }

    template <typename... FLGs>
        requires (std::is_same_v<FLGs, Flag> && ...)
    constexpr bool isFlagSet(FLGs... flags) const
    {
        return 0 != (flags_ & toFlagBits(flags...));
    }

    template <typename... FLGs>
        requires (std::is_same_v<FLGs, Flag> && ...)
    constexpr void setFlag(FLGs... flags)
    {
        flags_ |= toFlagBits(flags...);
    }

    template <typename... FLGs>
        requires (std::is_same_v<FLGs, Flag> && ...)
    constexpr void resetFlag(FLGs... flags)
    {
        flags_ &= ~(toFlagBits(flags...));
    }

    constexpr void reset() { flags_ = 0; }

    constexpr Flags & operator |=(const Flags & rhs)
    {
        flags_ |= rhs.flags_;
        return *this;
    }

    constexpr Flags & operator &=(const Flags & rhs)
    {
        flags_ &= rhs.flags_;
        return *this;
    }

    constexpr friend Flags operator |(const Flags & lhs, const Flags & rhs)
    {
        return Flags{lhs.flags_ | rhs.flags_};
    }

    constexpr friend Flags operator &(const Flags & lhs, const Flags & rhs)
    {
        return Flags{lhs.flags_ & rhs.flags_};
    }

private:
    UnderlyingType flags_ = 0;

    constexpr Flags(UnderlyingType flags):
        flags_(flags)
    { }

    template <typename... FLGs>
        requires (std::is_same_v<FLGs, Flag> && ...)
    static constexpr UnderlyingType toFlagBits(FLGs... flags)
    {
        return (UnderlyingType{0} | ... | (UnderlyingType{1} << static_cast<std::size_t>(flags)));
    }
};




#endif  // TOOLS_FLAGS_HPP_
