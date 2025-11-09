/**
 * @file
 */

#ifndef APP_TOOLS_ANIMATION_PARAMETER_HPP_
#define APP_TOOLS_ANIMATION_PARAMETER_HPP_

#include <limits>
#include <concepts>

#include "app/animation.hpp"
#include "stm32g070xx.h"


/** @brief Type of set function behavior when reaching limits of the parameter value */
enum class SetParameterType
{
    LIMIT,
    CYCLIC,
};

/**
 * @brief Change the animation parameter in cyclic manner if values are changed relatively
 *
 * @tparam T Actual integral type of the parameter
 * @tparam MAX_V Upper bound of the parameter
 * @tparam MIN_V Lower bound of the parameter
 * @tparam CYCLIC Parameters are cyclic
 *
 * @param[in,out] param_value Pointer to parameter value storage
 * @param value Value passed to Animation::setParamater() function
 * @param type type passed to Animation::setParamater() function
 *
 * @return New value of the parameter
 */
template <std::integral T, T MAX_V = std::numeric_limits<T>::max(), T MIN_V = std::numeric_limits<T>::min(),
    SetParameterType SPT = SetParameterType::LIMIT>
inline T setParameter(T param_value, int value, Animation::ChangeType type)
{
    if (Animation::ChangeType::ABSOLUTE == type)
        return static_cast<T>(value);
    else
    {
        int new_value = static_cast<int>(param_value);
        new_value += value;
        if constexpr (MAX_V != std::numeric_limits<T>::max() || MIN_V != std::numeric_limits<T>::min())
        {
            if constexpr (SetParameterType::CYCLIC == SPT)
            {
                if (new_value > static_cast<int>(MAX_V))
                    new_value = MIN_V;
                else if (new_value < static_cast<int>(MIN_V))
                    new_value = MAX_V;
            }
            else if constexpr (SetParameterType::LIMIT == SPT)
            {
                if (new_value > static_cast<int>(MAX_V))
                    new_value = MAX_V;
                else if (new_value < static_cast<int>(MIN_V))
                    new_value = MIN_V;
            }
        }
        return static_cast<T>(new_value);
    };
}


/** @copydoc setParameter() */
template <std::integral T, T MAX_V = std::numeric_limits<T>::max(), T MIN_V = std::numeric_limits<T>::min()>
inline T setCyclicParameter(T param_value, int value, Animation::ChangeType type)
{
    return setParameter<T, MAX_V, MIN_V, SetParameterType::CYCLIC>(param_value, value, type);
}

/** @copydoc setParameter() */
template <std::integral T, T MAX_V = std::numeric_limits<T>::max(), T MIN_V = std::numeric_limits<T>::min()>
inline T setLimitParameter(T param_value, int value, Animation::ChangeType type)
{
    return setParameter<T, MAX_V, MIN_V, SetParameterType::LIMIT>(param_value, value, type);
}



#endif  // APP_TOOLS_ANIMATION_PARAMETER_HPP_