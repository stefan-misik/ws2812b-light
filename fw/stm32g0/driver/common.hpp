/**
 * @file
 */

#ifndef DRIVER_COMMON_HPP_
#define DRIVER_COMMON_HPP_

#include <cstddef>
#include <cstdint>

namespace driver
{

enum class TimerId: std::uint8_t
{
    TIM_1,
    TIM_3,
};

enum class DmaRequestId: std::uint8_t
{
    TIM1_CH2 = 21,
};

}  // namespace driver




#endif  // DRIVER_COMMON_HPP_
