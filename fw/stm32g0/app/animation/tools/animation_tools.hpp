/**
 * @file
 */

#ifndef APP_ANIMATION_TOOLS_ANIMATION_TOOLS_HPP_
#define APP_ANIMATION_TOOLS_ANIMATION_TOOLS_HPP_

#include <cstddef>
#include <utility>
#include <initializer_list>
#include <concepts>

#include "app/animation.hpp"


template <typename T, typename X>
concept ParameterProtocol = requires(X x)
{
    { T::make(std::forward<X>(x)) } -> std::same_as<int>;
};


struct DefaultParamProtocol
{
    static constexpr int make(const int & value)
    {
        return value;
    }

    static constexpr int parse(int value)
    {
        return value;
    }
};


template <typename T, ParameterProtocol<const T &> PR, std::uint32_t FIRST_P,
    std::uint32_t COUNT_P = Animation::ParamId::RESERVED_>
inline bool setParameterGroup(Animation * animation, std::initializer_list<T> values)
{
    if constexpr (Animation::ParamId::RESERVED_ != COUNT_P)
    {
        if (!animation->setParamater(COUNT_P, static_cast<int>(values.size())))
            return false;
    }

    std::uint32_t param_id = FIRST_P;
    for (const auto & v : values)
    {
        if (!animation->setParamater(param_id, PR::make(v)))
            return false;
        ++param_id;
    }
    return true;
}


#endif  // APP_ANIMATION_TOOLS_ANIMATION_TOOLS_HPP_