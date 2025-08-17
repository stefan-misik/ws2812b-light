#include "driver/common.hpp"

#include "stm32g0xx.h"
#include "stm32g0xx_ll_dma.h"


namespace driver
{

std::uint32_t toDmaChannel(DmaChannelId dma_channel_id)
{
    switch (dma_channel_id)
    {
    case DmaChannelId::DMA_1: return LL_DMA_CHANNEL_1;
    case DmaChannelId::DMA_2: return LL_DMA_CHANNEL_2;
    case DmaChannelId::DMA_3: return LL_DMA_CHANNEL_3;
    case DmaChannelId::DMA_4: return LL_DMA_CHANNEL_4;
    case DmaChannelId::DMA_5: return LL_DMA_CHANNEL_5;
    case DmaChannelId::DMA_6: return LL_DMA_CHANNEL_6;
    case DmaChannelId::DMA_7: return LL_DMA_CHANNEL_7;
    }
    return INVALID_DMA_CHANNEL;
}

std::uint32_t toDmamuxChannel(std::uint32_t dma_channel)
{
    switch (dma_channel)
    {
    case LL_DMA_CHANNEL_1: return LL_DMAMUX_CHANNEL_0;
    case LL_DMA_CHANNEL_2: return LL_DMAMUX_CHANNEL_1;
    case LL_DMA_CHANNEL_3: return LL_DMAMUX_CHANNEL_2;
    case LL_DMA_CHANNEL_4: return LL_DMAMUX_CHANNEL_3;
    case LL_DMA_CHANNEL_5: return LL_DMAMUX_CHANNEL_4;
    case LL_DMA_CHANNEL_6: return LL_DMAMUX_CHANNEL_5;
    case LL_DMA_CHANNEL_7: return LL_DMAMUX_CHANNEL_6;
    }
    return INVALID_DMAMUX_CHANNEL;
}

}  // namespace driver
