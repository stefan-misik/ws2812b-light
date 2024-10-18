/**
 * @file
 */

#ifndef DRIVER_COMMON_HPP_
#define DRIVER_COMMON_HPP_

#include <cstddef>
#include <cstdint>

namespace driver
{

enum class GpioId: std::uint8_t
{
    GPIO_A,
    GPIO_B,
    GPIO_C,
};

enum class TimerId: std::uint8_t
{
    TIM_1,
    TIM_3,
    TIM_16,
};

struct GpioPinId
{
    GpioId gpio;
    std::uint8_t pin;
};

}  // namespace driver

#endif  // DRIVER_COMMON_HPP_
