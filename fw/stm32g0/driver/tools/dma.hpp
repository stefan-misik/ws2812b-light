/**
 * @file
 */

#ifndef DRIVER_TOOLS_DMA_HPP_
#define DRIVER_TOOLS_DMA_HPP_

#include <cstdint>

#include "stm32g0xx_ll_dma.h"


namespace driver
{

enum DmaFlags
{
    DMA_COMPLETE,
    DMA_HALF_COMPLETE,
    DMA_ERROR,
};


inline void startDma(std::uint32_t dma_channel, const void * data, std::size_t length)
{
    ::LL_DMA_DisableChannel(DMA1, dma_channel);

    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, reinterpret_cast<std::uint32_t>(data));
    ::LL_DMA_SetDataLength(DMA1, dma_channel, length);

    ::LL_DMA_EnableChannel(DMA1, dma_channel);
}

inline bool isDmaOngoing(std::uint32_t dma_channel)
{
    return ::LL_DMA_IsEnabledChannel(DMA1, dma_channel);
}

inline void stopDma(std::uint32_t dma_channel)
{
    ::LL_DMA_DisableChannel(DMA1, dma_channel);
}

inline std::size_t getDmaDataLength(std::uint32_t dma_channel)
{
    return ::LL_DMA_GetDataLength(DMA1, dma_channel);
}

inline std::uint32_t readDmaFlags(std::uint32_t dma_channel)
{
    #define READ_DMA_FLAGS_CHANNEL(dma, channel)  \
        if (::LL_DMA_IsActiveFlag_TC ## channel(dma))  \
        {  \
            flags |= (1 << DmaFlags::DMA_COMPLETE);  \
            ::LL_DMA_ClearFlag_TC ## channel(dma);  \
        }  \
        if (::LL_DMA_IsActiveFlag_HT ## channel(dma))  \
        {  \
            flags |= (1 << DmaFlags::DMA_HALF_COMPLETE);  \
            ::LL_DMA_ClearFlag_HT ## channel(dma);  \
        }  \
        if (::LL_DMA_IsActiveFlag_TE ## channel(dma))  \
        {  \
            flags |= (1 << DmaFlags::DMA_ERROR);  \
            ::LL_DMA_ClearFlag_TE ## channel(dma);  \
        }

    std::uint32_t flags = 0;
    switch (dma_channel)
    {
    case LL_DMA_CHANNEL_1: READ_DMA_FLAGS_CHANNEL(DMA1, 1); break;
    case LL_DMA_CHANNEL_2: READ_DMA_FLAGS_CHANNEL(DMA1, 2); break;
    case LL_DMA_CHANNEL_3: READ_DMA_FLAGS_CHANNEL(DMA1, 3); break;
    case LL_DMA_CHANNEL_4: READ_DMA_FLAGS_CHANNEL(DMA1, 4); break;
    case LL_DMA_CHANNEL_5: READ_DMA_FLAGS_CHANNEL(DMA1, 5); break;
    case LL_DMA_CHANNEL_6: READ_DMA_FLAGS_CHANNEL(DMA1, 6); break;
    case LL_DMA_CHANNEL_7: READ_DMA_FLAGS_CHANNEL(DMA1, 7); break;
    }
    #undef READ_DMA_FLAGS_CHANNEL

    return flags;
}


}  // namespace driver

#endif  // DRIVER_TOOLS_DMA_HPP_

