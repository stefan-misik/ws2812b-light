/**
 * @file
 */

#ifndef TOOLS_NEXT_STATE_HPP_
#define TOOLS_NEXT_STATE_HPP_

#include <cstdint>
#include <type_traits>


template <typename T>
class NextState
{
public:
    using Type = T;

    NextState() = default;
    NextState(const NextState &) = default;
    NextState(NextState &&) = default;
    NextState & operator =(const NextState &) = default;
    NextState & operator =(NextState &&) = default;

    operator Type() const { return static_cast<Type>(value_); }

    void yieldTo(Type value)
    {
        value_ = static_cast<UnderlyingType>(value);
        state_ = State::YIELD;
    }

    void continueTo(Type value)
    {
        value_ = static_cast<UnderlyingType>(value);
        state_ = State::CONTINUE;
    }

    void reset() { state_ = State::EMPTY; }
    bool doContinue() const { return State::CONTINUE == state_; }
    bool hasValue() const { return State::EMPTY != state_; }

private:
    using UnderlyingType = std::underlying_type_t<Type>;

    enum class State: std::uint8_t
    {
        EMPTY,
        CONTINUE,
        YIELD,
    };

    UnderlyingType value_ = 0;
    State state_ = State::EMPTY;
};

#endif  // TOOLS_NEXT_STATE_HPP_