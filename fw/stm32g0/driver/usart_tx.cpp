#include "driver/usart_tx.hpp"

#include <limits>

#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_dmamux.h"
#include "stm32g0xx_ll_usart.h"

#include "driver/tools/irq.hpp"
#include "driver/tools/dma.hpp"


namespace driver
{
namespace
{

struct Hardware
{
    ::USART_TypeDef * usart;
    std::uint32_t dma_channel;
};

::USART_TypeDef * toUsart(UsartId usart_id)
{
    switch (usart_id)
    {
    case UsartId::USART_1: return USART1;
    case UsartId::USART_2: return USART2;
    case UsartId::USART_3: return USART3;
    case UsartId::USART_4: return USART4;
    default: break;
    }
    return nullptr;
}

std::uint32_t toDmaTxRequest(UsartId usart_id)
{
    switch (usart_id)
    {
    case UsartId::USART_1: return LL_DMAMUX_REQ_USART1_TX;
    case UsartId::USART_2: return LL_DMAMUX_REQ_USART2_TX;
    case UsartId::USART_3: return LL_DMAMUX_REQ_USART3_TX;
    case UsartId::USART_4: return LL_DMAMUX_REQ_USART4_TX;
    default: break;
    }
    return 0;
}

bool initializeUsart(::USART_TypeDef * usart, std::uint32_t baud_rate)
{
    ::LL_USART_Disable(usart);

    ::LL_USART_InitTypeDef init;
    ::LL_USART_StructInit(&init);
    init.BaudRate = baud_rate;
    init.DataWidth = LL_USART_DATAWIDTH_8B;
    init.StopBits = LL_USART_STOPBITS_1;
    init.Parity = LL_USART_PARITY_NONE;
    init.TransferDirection = LL_USART_DIRECTION_TX;
    init.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
    init.OverSampling = LL_USART_OVERSAMPLING_16;
    if (::ErrorStatus::SUCCESS != ::LL_USART_Init(usart, &init))
        return false;

    ::LL_USART_ConfigAsyncMode(usart);
    ::LL_USART_Enable(usart);
    return true;
}

void initializeDma(std::uint32_t dma_channel, std::uint32_t dmamux_channel, std::uint32_t dma_tx_request,
    ::USART_TypeDef * usart)
{
    ::LL_DMA_DisableChannel(DMA1, dma_channel);

    ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, dma_tx_request);
    ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
    ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);
    ::LL_DMAMUX_DisableRequestGen(DMAMUX1, dmamux_channel);

    ::LL_DMA_ConfigTransfer(DMA1, dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_NORMAL |
            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_PDATAALIGN_BYTE | LL_DMA_MDATAALIGN_BYTE |
            LL_DMA_PRIORITY_LOW);
    ::LL_DMA_SetPeriphAddress(DMA1, dma_channel,
            ::LL_USART_DMA_GetRegAddr(usart, LL_USART_DMA_REG_DATA_TRANSMIT));
    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, 0);
    ::LL_DMA_SetDataLength(DMA1, dma_channel, 0);

    ::LL_DMA_EnableIT_TC(DMA1, dma_channel);
    ::LL_DMA_EnableIT_TE(DMA1, dma_channel);

    ::LL_USART_EnableDMAReq_TX(usart);
    ::LL_USART_DisableDMAReq_RX(usart);
}

inline std::size_t startDmaTransfer(const Hardware & hw, const std::uint8_t * buffer,
    const BipBufferController<std::uint16_t> & buffer_ctrl)
{
    const auto [offset, length] = buffer_ctrl.getContiguousBlock();
    if (0 == length)
    {
        stopDma(hw.dma_channel);
        return 0;
    }

    startDma(hw.dma_channel, buffer + offset, length);

    return length;
}

}  // namespace

struct UsartTx::Private
{
    Hardware hw;
};

UsartTx::UsartTx() = default;
UsartTx::~UsartTx()
{ }

bool UsartTx::initialize(UsartId usart_id, DmaChannelId dma_channel_id, std::uint32_t baud_rate,
    std::size_t buffer_size)
{
    if (0 == baud_rate || buffer_size < 2 || buffer_size > std::numeric_limits<SizeType>::max())
        return false;

    auto * const usart = toUsart(usart_id);
    if (nullptr == usart)
        return false;

    const std::uint32_t dma_tx_request = toDmaTxRequest(usart_id);
    const std::uint32_t dma_channel = toDmaChannel(dma_channel_id);
    const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
    if (0 == dma_tx_request || INVALID_DMA_CHANNEL == dma_channel || INVALID_DMAMUX_CHANNEL == dmamux_channel)
        return false;

    if (!initializeUsart(usart, baud_rate))
        return false;

    initializeDma(dma_channel, dmamux_channel, dma_tx_request, usart);

    auto & priv = *p_;
    priv.hw.usart = usart;
    priv.hw.dma_channel = dma_channel;

    auto buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[buffer_size]);
    if (!buffer)
        return false;

    buffer_ = std::move(buffer);
    buffer_size_ = buffer_size;
    buffer_ctrl_.clear(buffer_size_);

    enableDmaIrq(dma_channel_id);
    return true;
}

std::pair<void *, std::size_t> UsartTx::reserveBuffer(std::size_t size)
{
    std::pair<SizeType, SizeType> region;
    {
        IrqGuard guard_;
        region = buffer_ctrl_.reserve(static_cast<SizeType>(size), buffer_size_);
    }

    return {buffer_.get() + region.first, region.second};
}

bool UsartTx::commitBuffer(void * buffer, std::size_t size)
{
    const SizeType offset = static_cast<SizeType>(static_cast<std::uint8_t *>(buffer) - buffer_.get());
    {
        IrqGuard guard_;
        if (!buffer_ctrl_.commit(offset, static_cast<SizeType>(size), buffer_size_))
            return false;
    }

    auto & priv = *p_;
    {
        IrqGuard guard_;
        if (isDmaOngoing(priv.hw.dma_channel))
            return true;
        dma_active_size_ = startDmaTransfer(priv.hw, buffer_.get(), buffer_ctrl_);
        return true;
    }
    return true;
}

void UsartTx::maybeHandleDmaInterrupt()
{
    const auto dma_flags = readDmaFlags(p_->hw.dma_channel);
    if (0 == (dma_flags & (1 << DmaFlags::DMA_COMPLETE)))
        return;

    auto & priv = *p_;

    buffer_ctrl_.decommit(dma_active_size_, buffer_size_);
    dma_active_size_ = startDmaTransfer(priv.hw, buffer_.get(), buffer_ctrl_);
}

}  // namespace driver