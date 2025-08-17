#include "driver/i2c_bus.hpp"

#include <atomic>

#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_dma.h"


namespace driver
{
namespace
{

::I2C_TypeDef * toI2c(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return I2C1;
    default: break;
    }
    return nullptr;
}

std::uint32_t toDmaRxRequest(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return LL_DMAMUX_REQ_I2C1_RX;
    default: break;
    }

    return 0;
}

std::uint32_t toDmaTxRequest(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return LL_DMAMUX_REQ_I2C1_TX;
    default: break;
    }

    return 0;
}

bool initializeI2c(::I2C_TypeDef * i2c)
{
    ::LL_I2C_InitTypeDef init;
    ::LL_I2C_StructInit(&init);
    init.PeripheralMode = LL_I2C_MODE_I2C;
    // Fi2cclk = 64 MHz, Fscl = 400 kHz
    init.Timing = __LL_I2C_CONVERT_TIMINGS(
            /* PRESC */ 7,
            /* SCLDEL */ 3,
            /* SDADEL */ 4,
            /* SCLH */ 3,
            /* SCLL */ 9);
    init.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
    init.DigitalFilter   = 0U;
    init.OwnAddress1     = 0U;
    init.TypeAcknowledge = LL_I2C_NACK;
    init.OwnAddrSize     = LL_I2C_OWNADDRESS1_7BIT;
    if (::ErrorStatus::SUCCESS != ::LL_I2C_Init(i2c, &init))
        return false;

    ::LL_I2C_SetMasterAddressingMode(i2c, LL_I2C_ADDRESSING_MODE_7BIT);
    ::LL_I2C_DisableOwnAddress2(i2c);

    ::LL_I2C_DisableAutoEndMode(i2c);
    ::LL_I2C_DisableReloadMode(i2c);
    ::LL_I2C_SetTransferSize(i2c, 0);
    return true;
}

bool initializeRxDma(std::uint32_t dma_channel, std::uint32_t request, ::I2C_TypeDef * i2c)
{
    {
        const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
        ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, request);
        ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
        ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);
    }

    ::LL_DMA_DisableChannel(DMA1, dma_channel);
    ::LL_DMA_ConfigTransfer(DMA1, dma_channel, LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_MODE_NORMAL |
            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_BYTE |
            LL_DMA_PRIORITY_MEDIUM);
    ::LL_DMA_SetPeriphAddress(DMA1, dma_channel, ::LL_I2C_DMA_GetRegAddr(i2c, LL_I2C_DMA_REG_DATA_RECEIVE));
    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, 0);
    ::LL_DMA_SetDataLength(DMA1, dma_channel, 0);

    ::LL_DMA_DisableIT_TC(DMA1, dma_channel);
    ::LL_DMA_DisableIT_HT(DMA1, dma_channel);
    ::LL_DMA_DisableIT_TE(DMA1, dma_channel);

    ::LL_I2C_EnableDMAReq_RX(i2c);
    return true;
}

bool initializeTxDma(std::uint32_t dma_channel, std::uint32_t request, ::I2C_TypeDef * i2c)
{
    {
        const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
        ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, request);
        ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
        ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);
    }

    ::LL_DMA_DisableChannel(DMA1, dma_channel);
    ::LL_DMA_ConfigTransfer(DMA1, dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_NORMAL |
            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_PDATAALIGN_WORD | LL_DMA_MDATAALIGN_BYTE |
            LL_DMA_PRIORITY_MEDIUM);
    ::LL_DMA_SetPeriphAddress(DMA1, dma_channel, ::LL_I2C_DMA_GetRegAddr(i2c, LL_I2C_DMA_REG_DATA_TRANSMIT));
    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, 0);
    ::LL_DMA_SetDataLength(DMA1, dma_channel, 0);

    ::LL_DMA_DisableIT_TC(DMA1, dma_channel);
    ::LL_DMA_DisableIT_HT(DMA1, dma_channel);
    ::LL_DMA_DisableIT_TE(DMA1, dma_channel);

    ::LL_I2C_EnableDMAReq_TX(i2c);
    return true;
}


class State
{
public:
private:
    std::atomic<bool> is_busy_ = false;
};

}  // namespace

struct I2cBus::Private
{
    ::I2C_TypeDef * i2c;
    std::uint32_t dma_rx_channel;
    std::uint32_t dma_tx_channel;

    State state;
};

I2cBus::I2cBus() = default;
I2cBus::~I2cBus()
{ }

bool I2cBus::initialize(I2cId i2c_id, DmaChannelId dma_channel_id_rx, DmaChannelId dma_channel_id_tx)
{
    auto * const i2c = toI2c(i2c_id);
    if (nullptr == i2c)
        return false;

    const std::uint32_t dma_rx_request = toDmaRxRequest(i2c_id);
    const std::uint32_t dma_rx_channel = toDmaChannel(dma_channel_id_rx);
    const std::uint32_t dma_tx_request = toDmaTxRequest(i2c_id);
    const std::uint32_t dma_tx_channel = toDmaChannel(dma_channel_id_tx);
    if (0 == dma_rx_request || INVALID_DMA_CHANNEL == dma_rx_channel ||
            0 == dma_tx_request || INVALID_DMA_CHANNEL == dma_tx_channel)
        return false;

    if (!initializeI2c(i2c))
        return false;
    if (!initializeRxDma(dma_rx_channel, dma_rx_request, i2c))
        return false;
    if (!initializeTxDma(dma_tx_channel, dma_tx_request, i2c))
        return false;

    auto & priv = *p_;
    priv.i2c = i2c;
    priv.dma_rx_channel = dma_rx_channel;
    priv.dma_tx_channel = dma_tx_channel;

    return true;
}

}  // namespace driver
