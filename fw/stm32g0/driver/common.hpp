/**
 * @file
 */

#ifndef DRIVER_COMMON_HPP_
#define DRIVER_COMMON_HPP_

#include <cstddef>
#include <cstdint>

namespace driver
{

enum class DmaChannelId: std::uint8_t
{
    DMA_1, DMA_2, DMA_3, DMA_4, DMA_5, DMA_6, DMA_7,
};

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
    TIM_6,
    TIM_16,
};

enum class I2cId: std::uint8_t
{
    I2C_1,
    I2C_2,
};

struct GpioPinId
{
    GpioId gpio;
    std::uint8_t pin;
};

static const std::uint32_t INVALID_DMA_CHANNEL = 0xFFFFFFFF;
static const std::uint32_t INVALID_DMAMUX_CHANNEL = 0xFFFFFFFF;

/**
 * @brief Get the DMA channel
 *
 * @param dma_channel_id DMA channel ID
 *
 * @return DMA channel ID
 * @retval INVALID_DMA_CHANNEL Failed to get DMA channel
 */
std::uint32_t toDmaChannel(DmaChannelId dma_channel_id);

/**
 * @brief Get DMAMUX channel from DMA channel
 *
 * @param dma_channel DMA channel returned form @ref toDmaChannel()
 *
 * @return DMAMUX channel ID
 * @retval INVALID_DMAMUX_CHANNEL Failed to get DMAMUX channel
 */
std::uint32_t toDmamuxChannel(std::uint32_t dma_channel);


}  // namespace driver

#endif  // DRIVER_COMMON_HPP_
