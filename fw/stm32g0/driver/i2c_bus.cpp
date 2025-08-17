#include "driver/i2c_bus.hpp"

#include <atomic>
#include <memory>

#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_i2c.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_dmamux.h"


namespace driver
{
namespace
{

using Transaction = I2cBus::Transaction;
using TransactionNode = I2cBus::TransactionNode;


class IrqGuard
{
public:
    IrqGuard():
        primask_(disableAndGetPrimask())
    { }

    ~IrqGuard()
    {
        __DSB();
        __set_PRIMASK(primask_);
    }

private:
    const std::uint32_t primask_;

    static std::uint32_t disableAndGetPrimask()
    {
        __DSB();
        const auto primask = __get_PRIMASK();
        __disable_irq();
        return primask;
    }
};


struct Hardware
{
    ::I2C_TypeDef * i2c;
    std::uint32_t dma_channel;
    std::uint32_t dmamux_channel;
    std::uint32_t dma_rx_request;
    std::uint32_t dma_tx_request;
};



inline Transaction * toTransaction(List::Node * node)
{
    return static_cast<TransactionNode *>(node);
}

inline List::Node * toNode(Transaction * transaction)
{
    return static_cast<TransactionNode *>(transaction);
}


::I2C_TypeDef * toI2c(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return I2C1;
    case I2cId::I2C_2: return I2C2;
    default: break;
    }
    return nullptr;
}

std::uint32_t toDmaRxRequest(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return LL_DMAMUX_REQ_I2C1_RX;
    case I2cId::I2C_2: return LL_DMAMUX_REQ_I2C2_RX;
    default: break;
    }

    return 0;
}

std::uint32_t toDmaTxRequest(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1: return LL_DMAMUX_REQ_I2C1_TX;
    case I2cId::I2C_2: return LL_DMAMUX_REQ_I2C2_TX;
    default: break;
    }

    return 0;
}

void enableI2cIrq(I2cId i2c_id)
{
    switch (i2c_id)
    {
    case I2cId::I2C_1:
        ::NVIC_EnableIRQ(::I2C1_IRQn);
        break;
    case I2cId::I2C_2:
        ::NVIC_EnableIRQ(::I2C2_IRQn);
        break;
    }
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

    ::LL_I2C_DisableSMBusTimeout(i2c, LL_I2C_SMBUS_ALL_TIMEOUT);

    ::LL_I2C_DisableAutoEndMode(i2c);
    ::LL_I2C_DisableReloadMode(i2c);
    ::LL_I2C_SetTransferSize(i2c, 0);

    ::LL_I2C_EnableIT_NACK(i2c);
    ::LL_I2C_EnableIT_TC(i2c);
    ::LL_I2C_EnableIT_ERR(i2c);

    return true;
}

bool initializeDma(std::uint32_t dma_channel, std::uint32_t dmamux_channel, ::I2C_TypeDef * i2c)
{
    ::LL_DMA_DisableChannel(DMA1, dma_channel);

    // Some of the following parameters are dummy, and will be re-configured when starting RX or TX transaction

    ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, LL_DMAMUX_REQ_MEM2MEM);
    ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
    ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);

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

    ::LL_I2C_DisableDMAReq_RX(i2c);
    ::LL_I2C_DisableDMAReq_TX(i2c);
    return true;
}

inline void startRxDmaTransaction(const Hardware & hw, void * buffer, std::size_t size)
{
    ::LL_DMA_DisableChannel(DMA1, hw.dma_channel);
    {
        ::LL_DMAMUX_SetRequestID(DMAMUX1, hw.dmamux_channel, hw.dma_rx_request);
        ::LL_DMA_SetDataTransferDirection(DMA1, hw.dma_channel, LL_DMA_DIRECTION_PERIPH_TO_MEMORY);
        ::LL_DMA_SetPeriphAddress(DMA1, hw.dma_channel, ::LL_I2C_DMA_GetRegAddr(hw.i2c, LL_I2C_DMA_REG_DATA_RECEIVE));
        ::LL_DMA_SetMemoryAddress(DMA1, hw.dma_channel, reinterpret_cast<std::uintptr_t>(buffer));
        ::LL_DMA_SetDataLength(DMA1, hw.dma_channel, size);
    }
    ::LL_I2C_EnableDMAReq_RX(hw.i2c);
    ::LL_I2C_DisableDMAReq_TX(hw.i2c);

    ::LL_DMA_EnableChannel(DMA1, hw.dma_channel);
}

inline void startTxDmaTransaction(const Hardware & hw, const void * buffer, std::size_t size)
{
    ::LL_DMA_DisableChannel(DMA1, hw.dma_channel);
    {
        ::LL_DMAMUX_SetRequestID(DMAMUX1, hw.dmamux_channel, hw.dma_tx_request);
        ::LL_DMA_SetDataTransferDirection(DMA1, hw.dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH);
        ::LL_DMA_SetPeriphAddress(DMA1, hw.dma_channel, ::LL_I2C_DMA_GetRegAddr(hw.i2c, LL_I2C_DMA_REG_DATA_TRANSMIT));
        ::LL_DMA_SetMemoryAddress(DMA1, hw.dma_channel, reinterpret_cast<std::uintptr_t>(buffer));
        ::LL_DMA_SetDataLength(DMA1, hw.dma_channel, size);
    }
    ::LL_I2C_DisableDMAReq_RX(hw.i2c);
    ::LL_I2C_EnableDMAReq_TX(hw.i2c);

    ::LL_DMA_EnableChannel(DMA1, hw.dma_channel);
}

inline void finishDmaTransaction(const Hardware & hw)
{
    ::LL_DMA_DisableChannel(DMA1, hw.dma_channel);
}


class State
{
public:
    bool enqueueTransaction(Transaction * transaction)
    {
        const bool start_new = (nullptr == pending_.peek());
        pending_.push(toNode(transaction));
        return start_new;
    }

    Transaction * getDone()
    {
        return toTransaction(done_.pop());
    }

private:
    List pending_;
    List done_;

    Transaction * currentTransaction() const
    {
        return toTransaction(pending_.peek());
    }
};


}  // namespace

struct I2cBus::Private
{
    Hardware hw;

    State state;

    std::unique_ptr<TransactionNode[]> transactions;
};

I2cBus::I2cBus() = default;
I2cBus::~I2cBus()
{ }

bool I2cBus::initialize(I2cId i2c_id, DmaChannelId dma_channel_id, std::size_t transaction_cnt)
{
    auto * const i2c = toI2c(i2c_id);
    if (nullptr == i2c)
        return false;

    const std::uint32_t dma_rx_request = toDmaRxRequest(i2c_id);
    const std::uint32_t dma_tx_request = toDmaTxRequest(i2c_id);
    const std::uint32_t dma_channel = toDmaChannel(dma_channel_id);
    const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
    if (0 == dma_rx_request || 0 == dma_tx_request || INVALID_DMA_CHANNEL == dma_channel ||
            INVALID_DMAMUX_CHANNEL == dmamux_channel)
        return false;

    if (!initializeI2c(i2c))
        return false;
    if (!initializeDma(dma_channel, dmamux_channel, i2c))
        return false;

    enableI2cIrq(i2c_id);

    auto & priv = *p_;

    priv.transactions.reset(new TransactionNode[transaction_cnt]);
    if (!priv.transactions)
        return false;

    priv.hw.i2c = i2c;
    priv.hw.dma_channel = dma_channel;
    priv.hw.dmamux_channel = dmamux_channel;
    priv.hw.dma_rx_request = dma_rx_request;
    priv.hw.dma_tx_request = dma_tx_request;

    // Fill list of free transactions
    {
        auto * const end = priv.transactions.get() + transaction_cnt;
        for (auto * pos = priv.transactions.get(); pos != end; ++pos)
        {
            pos->reset();
            free_.push(pos);
        }
    }

    return true;
}

void I2cBus::tick()
{
    auto & priv = *p_;
    ::LL_I2C_ReceiveData8(priv.hw.i2c);
    ::LL_I2C_HandleTransfer(priv.hw.i2c, 0b10100000, LL_I2C_ADDRSLAVE_7BIT, 1, LL_I2C_MODE_SOFTEND,
            LL_I2C_GENERATE_START_READ);
}

Transaction * I2cBus::get()
{
    auto & priv = *p_;

    // Try to get received transaction
    Transaction * transaction;
    {
        IrqGuard guard_;
        transaction = priv.state.getDone();
    }

    return transaction;
}

void I2cBus::enqueue(Transaction * transaction)
{
    auto & priv = *p_;
    if (transaction->isEmpty())
    {
        releaseRaw(transaction);
    }
    else
    {
        bool start_new;
        {
            IrqGuard guard_;
            start_new = priv.state.enqueueTransaction(transaction);
        }
        if (start_new)
        {
            // TODO initiate new transaction
        }
    }
}

void I2cBus::maybeHandleI2cInterrupt()
{
    auto & priv = *p_;

    // Handle NACK
    if (::LL_I2C_IsActiveFlag_NACK(priv.hw.i2c))
    {
        ::LL_I2C_ClearFlag_NACK(priv.hw.i2c);
    }

    // Handle TC
    if (::LL_I2C_IsActiveFlag_TC(priv.hw.i2c))
    {
        ::LL_I2C_GenerateStopCondition(priv.hw.i2c);
    }

    // Handle Arbitration Loss
    if (::LL_I2C_IsActiveFlag_ARLO(priv.hw.i2c))
    {
        ::LL_I2C_ClearFlag_ARLO(priv.hw.i2c);
    }
    // Handle Bus error
    if (::LL_I2C_IsActiveFlag_BERR(priv.hw.i2c))
    {
        ::LL_I2C_ClearFlag_BERR(priv.hw.i2c);
    }
}

}  // namespace driver
