/**
 * @file
 */

#ifndef TOOLS_FLAGS_HPP_
#define TOOLS_FLAGS_HPP_

#include <cstddef>
#include <cstdint>


template <typename FLG, typename T = std::uint32_t>
class Flags
{
public:
    using Flag = FLG;
    using UnderlyingType = T;

    constexpr Flags() = default;
    constexpr Flags(const Flags &) = default;
    constexpr Flags & operator =(const Flags &) = default;

    constexpr Flags(Flag flag):
        flags_(toFlagBit(flag))
    { }

    constexpr bool isFlagSet(Flag flag) const { return 0 != (flags_ & toFlagBit(flag)); }

    constexpr void setFlag(Flag flag) { flags_ |= toFlagBit(flag); }
    constexpr void resetFlag(Flag flag) { flags_ &= ~(toFlagBit(flag)); }
    constexpr void reset() { flags_ = 0; }

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

    static constexpr UnderlyingType toFlagBit(Flag flag)
    {
        return static_cast<UnderlyingType>(1) << static_cast<std::size_t>(flag);
    }
};




#endif  // TOOLS_FLAGS_HPP_
