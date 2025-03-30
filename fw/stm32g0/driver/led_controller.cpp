#include "driver/led_controller.hpp"

#include <algorithm>

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_cortex.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_dmamux.h"


namespace driver
{
namespace
{

// T1H = 0.8 us, (0.8us / 1.25us) * (80 - 1) ~= 51
const std::uint8_t ONE_BIT_LENGTH = 51;
// T1H = 0.4 us, (0.4us / 1.25us) * (80 - 1) ~= 25
const std::uint8_t ZERO_BIT_LENGTH = 25;


class NibbleBitPatternTable
{
public:
    constexpr NibbleBitPatternTable()
    {
        for (std::size_t n = 0; n != 16; ++n)
            values_[n] = makeNibbleBits(n);
    }

    std::uint32_t operator [](std::size_t n) const { return values_[n]; }

private:
     std::uint32_t values_[16];

    static constexpr std::uint32_t makeNibbleBits(std::uint8_t nibble)
    {
        std::uint32_t value = 0;
        for (std::size_t n = 0; n != 4; ++n)
        {
            value <<= 8;
            value |= (nibble & 0x01) ? ONE_BIT_LENGTH : ZERO_BIT_LENGTH;
            nibble >>= 1;
        }
        return value;
    }
};


const NibbleBitPatternTable NIBBLE_BIT_PATTERN;

enum DmaFlags
{
    DMA_COMPLETE,
    DMA_HALF_COMPLETE,
    DMA_ERROR,
};


/**
 * @brief Tool used to write bit patterns into a circular buffer
 */
class BitCircularBuffer
{
public:
    static const inline std::size_t HALF_CAPACITY = LedController::BUFFER_HALF_LENGTH;

    const void * data() const { return buffer_; }
    std::size_t length() const { return sizeof(buffer_); }

    /**
     * @brief Read data to bit buffer
     *
     * @param half Which half of the buffer to fill
     * @param data[in] Data to be written in the buffer
     * @param length Length of the data
     * @param offset Offset in the data to start reading from
     *
     * @return Number of bytes processed from the buffer, the length of the data
     *         in the buffer is 8 times the returned value
     * @retval 0 Done writing or insufficient buffer
     */
    std::size_t read(std::size_t half, const void * data, std::size_t length, std::size_t offset)
    {
        BytePattern * buffer_pos = buffer_ + (half * HALF_CAPACITY);
        BytePattern * const buffer_end = buffer_pos + HALF_CAPACITY;

        if (0 == offset)
        {
            // Synchronization sequence ensures the DMA is synchronized with the
            // timer. This is only added when starting the transmission.
            (buffer_pos++)->blank();
        }

        const std::uint8_t * const data_start = reinterpret_cast<const std::uint8_t *>(data) + offset;
        const std::uint8_t * const data_end = data_start +
                std::min<std::size_t>(length - offset, buffer_end - buffer_pos);
        const std::uint8_t * data_pos = data_start;
        for (; data_pos != data_end; ++data_pos)
            (buffer_pos++)->setByte(*data_pos);

        // Fill the rest of the buffer with blank bits, in case we run out of data to send
        for (; buffer_pos != buffer_end; ++buffer_pos)
            buffer_pos->blank();

        return data_pos - data_start;
    }

    /**
     * @brief Check whether given half-buffer ends with blank bits
     *
     * @param half The required half-buffer
     *
     * @return Is blank-terminated
     */
    bool isBlankTerminated(std::size_t half) const
    {
        return (buffer_ + (half * HALF_CAPACITY))[HALF_CAPACITY - 1].isBlank();
    }

private:
    class BytePattern
    {
    public:
        void setByte(std::uint8_t value)
        {
            upper_ = NIBBLE_BIT_PATTERN[value >> 4];
            lower_ = NIBBLE_BIT_PATTERN[value & 0xF];
        }

        void blank() { upper_ = 0; lower_ = 0; }
        bool isBlank() const { return 0 == lower_; }

    private:
        std::uint32_t upper_, lower_;
    };

    BytePattern buffer_[HALF_CAPACITY * 2];
};


/**
 * @brief Simple object keeping track of data to be pushed into LEDs
 */
class LedDataBuffer
{
public:
    static const inline std::size_t SIZE = LedController::MAX_DATA_LENGTH;

    void start(const LedState * leds, std::size_t count, const LedCorrection * correction)
    {
        length_ = correction->correct(leds, count, buffer_, SIZE);
        pos_ = 0;
    }

    bool readInto(std::size_t half, BitCircularBuffer * buffer)
    {
        const std::size_t done = buffer->read(half, buffer_, length_, pos_);
        if (0 == done)
            return false;
        pos_ += done;
        return true;
    }

private:
    std::uint8_t buffer_[SIZE];
    std::size_t length_ = 0;
    std::size_t pos_ = 0;
};


::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_1: return TIM1;
    case TimerId::TIM_3: return TIM3;
    default: break;
    }
    return nullptr;
}

std::uint32_t toChannel(std::uint32_t channel_id)
{
    switch (channel_id)
    {
    case 1: return LL_TIM_CHANNEL_CH1;
    case 2: return LL_TIM_CHANNEL_CH2;
    case 3: return LL_TIM_CHANNEL_CH3;
    case 4: return LL_TIM_CHANNEL_CH4;
    }
    return 0;
}

std::uint32_t toDmaChannel(std::uint8_t dma_channel_id)
{
    switch (dma_channel_id)
    {
    case 0: return LL_DMA_CHANNEL_1;
    case 1: return LL_DMA_CHANNEL_2;
    case 2: return LL_DMA_CHANNEL_3;
    case 3: return LL_DMA_CHANNEL_4;
    case 4: return LL_DMA_CHANNEL_5;
    case 5: return LL_DMA_CHANNEL_6;
    case 6: return LL_DMA_CHANNEL_7;
    }
    return 0xFFFFFFFF;
}

std::uint32_t toDmamuxChannel(std::uint8_t dma_channel)
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
    return 0xFFFFFFFF;
}

std::uint32_t toDmaRequest(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_1: return LL_DMAMUX_REQ_TIM1_UP;
    case TimerId::TIM_3: return LL_DMAMUX_REQ_TIM3_UP;
    default: break;
    }

    return 0;
}

std::uint32_t timerRegisterAddress(::TIM_TypeDef * tim, std::uint32_t channel)
{
    switch (channel)
    {
    case LL_TIM_CHANNEL_CH1: return reinterpret_cast<std::uint32_t>(&(tim->CCR1));
    case LL_TIM_CHANNEL_CH2: return reinterpret_cast<std::uint32_t>(&(tim->CCR2));
    case LL_TIM_CHANNEL_CH3: return reinterpret_cast<std::uint32_t>(&(tim->CCR3));
    case LL_TIM_CHANNEL_CH4: return reinterpret_cast<std::uint32_t>(&(tim->CCR4));
    }
    return 0;
}


bool initializeTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_InitTypeDef init;
    ::LL_TIM_StructInit(&init);
    init.Prescaler = 0;  // No pre-scaler
    init.CounterMode = LL_TIM_COUNTERDIRECTION_UP;
    init.Autoreload = 80 - 1;  // 64 MHz / 800 kHz = 80
    init.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    init.RepetitionCounter = 0;

    if (::ErrorStatus::SUCCESS != ::LL_TIM_Init(tim, &init))
        return false;

    ::LL_TIM_DisableARRPreload(tim);
    ::LL_TIM_SetClockSource(tim, LL_TIM_CLOCKSOURCE_INTERNAL);
    ::LL_TIM_SetTriggerOutput(tim, LL_TIM_TRGO_RESET);
    ::LL_TIM_SetTriggerOutput2(tim, LL_TIM_TRGO2_RESET);
    ::LL_TIM_DisableMasterSlaveMode(tim);

    // Break- and dead-time
    {
        LL_TIM_BDTR_InitTypeDef break_deat_time_init;

        ::LL_TIM_BDTR_StructInit(&break_deat_time_init);
        break_deat_time_init.OSSRState = LL_TIM_OSSR_DISABLE;
        break_deat_time_init.OSSIState = LL_TIM_OSSI_DISABLE;
        break_deat_time_init.LockLevel = LL_TIM_LOCKLEVEL_OFF;
        break_deat_time_init.DeadTime = 0;
        break_deat_time_init.BreakState = LL_TIM_BREAK_DISABLE;
        break_deat_time_init.BreakPolarity = LL_TIM_BREAK_POLARITY_HIGH;
        break_deat_time_init.BreakFilter = LL_TIM_BREAK_FILTER_FDIV1;
        break_deat_time_init.Break2State = LL_TIM_BREAK2_DISABLE;
        break_deat_time_init.Break2Polarity = LL_TIM_BREAK2_POLARITY_HIGH;
        break_deat_time_init.Break2Filter = LL_TIM_BREAK2_FILTER_FDIV1;
        break_deat_time_init.AutomaticOutput = LL_TIM_AUTOMATICOUTPUT_DISABLE;
        if (::ErrorStatus::SUCCESS != ::LL_TIM_BDTR_Init(tim, &break_deat_time_init))
        {
            return false;
        }
    }

    ::LL_TIM_EnableAllOutputs(tim);
    return true;
}

bool initializeChannel(::TIM_TypeDef * tim, std::uint32_t channel)
{
    ::LL_TIM_OC_EnablePreload(tim, channel);

    ::LL_TIM_OC_InitTypeDef oc_init;

    ::LL_TIM_OC_StructInit(&oc_init);
    oc_init.OCMode = LL_TIM_OCMODE_PWM1;
    oc_init.OCState = LL_TIM_OCSTATE_DISABLE;
    oc_init.OCNState = LL_TIM_OCSTATE_DISABLE;
    oc_init.CompareValue = 0;
    oc_init.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
    oc_init.OCNPolarity = LL_TIM_OCPOLARITY_HIGH;
    oc_init.OCIdleState = LL_TIM_OCIDLESTATE_LOW;
    oc_init.OCNIdleState = LL_TIM_OCIDLESTATE_LOW;
    if (::ErrorStatus::SUCCESS != ::LL_TIM_OC_Init(tim, channel, &oc_init))
    {
        return false;
    }

    ::LL_TIM_EnableDMAReq_UPDATE(tim);
    ::LL_TIM_OC_DisableFast(tim, channel);
    ::LL_TIM_CC_EnableChannel(tim, channel);
    return true;
}

inline void startTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_EnableCounter(tim);
}

void forcePwm(::TIM_TypeDef * tim, std::uint32_t channel, std::uint8_t value)
{
    switch (channel)
    {
    case LL_TIM_CHANNEL_CH1: ::LL_TIM_OC_SetCompareCH1(tim, value); break;
    case LL_TIM_CHANNEL_CH2: ::LL_TIM_OC_SetCompareCH2(tim, value); break;
    case LL_TIM_CHANNEL_CH3: ::LL_TIM_OC_SetCompareCH3(tim, value); break;
    case LL_TIM_CHANNEL_CH4: ::LL_TIM_OC_SetCompareCH4(tim, value); break;
    }
}

bool initializeDma(std::uint32_t dma_channel, std::uint32_t request, ::TIM_TypeDef * tim, std::uint32_t channel)
{
    {
        const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
        ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, request);
        ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
        ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);
        ::LL_DMAMUX_DisableRequestGen(DMAMUX1, dmamux_channel);
    }

    ::LL_DMA_DisableChannel(DMA1, dma_channel);
    ::LL_DMA_ConfigTransfer(DMA1, dma_channel, LL_DMA_DIRECTION_MEMORY_TO_PERIPH | LL_DMA_MODE_CIRCULAR |
            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_BYTE |
            LL_DMA_PRIORITY_HIGH);
    ::LL_DMA_SetPeriphAddress(DMA1, dma_channel, timerRegisterAddress(tim, channel));
    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, 0);
    ::LL_DMA_SetDataLength(DMA1, dma_channel, 0);

    ::LL_DMA_EnableIT_TC(DMA1, dma_channel);
    ::LL_DMA_EnableIT_HT(DMA1, dma_channel);
    ::LL_DMA_EnableIT_TE(DMA1, dma_channel);
    return true;
}

void enableDmaIrq(std::uint32_t dma_channel)
{
    switch (dma_channel)
    {
    case LL_DMA_CHANNEL_1:
        ::NVIC_EnableIRQ(::DMA1_Channel1_IRQn);
        break;
    case LL_DMA_CHANNEL_2:
    case LL_DMA_CHANNEL_3:
        ::NVIC_EnableIRQ(::DMA1_Channel2_3_IRQn);
        break;
    case LL_DMA_CHANNEL_4:
    case LL_DMA_CHANNEL_5:
    case LL_DMA_CHANNEL_6:
    case LL_DMA_CHANNEL_7:
        ::NVIC_EnableIRQ(::DMA1_Ch4_7_DMAMUX1_OVR_IRQn);
        break;
    }
}

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

std::uint32_t readDmaFlags(std::uint32_t dma_channel)
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
            flags |= (1 << DmaFlags::DMA_COMPLETE);  \
            ::LL_DMA_ClearFlag_TC ## channel(dma);  \
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

    return flags;
}

}  // namespace


struct LedController::Private
{
    ::TIM_TypeDef * tim;
    std::uint32_t channel;
    std::uint32_t dma_channel;

    BitCircularBuffer dma_buffer;
    LedDataBuffer data;
};

LedController::LedController():
    correction_(TypeTag<CommonLedCorrection<DimmingLedWriter<>>>{}, 0x60)
{
}

LedController::~LedController()
{
}


bool LedController::initialize(TimerId tim_id, uint8_t channel_id, std::uint8_t dma_channel_id)
{
    auto * const tim = toTimer(tim_id);
    const std::uint32_t channel = toChannel(channel_id);
    if (nullptr == tim || 0 == channel)
        return false;

    if (!initializeTimer(tim))
        return false;

    if (!initializeChannel(tim, channel))
        return false;

    const std::uint32_t dma_request = toDmaRequest(tim_id);
    const std::uint32_t dma_channel = toDmaChannel(dma_channel_id);
    if (0 == dma_request || 0xFFFFFFFF == dma_channel)
        return false;

    if (!initializeDma(dma_channel, dma_request, tim, channel))
        return false;

    enableDmaIrq(dma_channel);

    auto & priv = *p_;
    priv.tim = tim;
    priv.channel = channel;
    priv.dma_channel = dma_channel;

    forcePwm(tim, channel, 0);
    startTimer(tim);
    return true;
}

bool LedController::update(const AbstractLedStrip * led_strip)
{
    auto & priv = *p_;

    if (isDmaOngoing(priv.dma_channel))
        return false;

    priv.data.start(led_strip->leds, led_strip->led_count, correction_.get());

    if (!priv.data.readInto(0, &priv.dma_buffer))
        return true;
    priv.data.readInto(1, &priv.dma_buffer);

    startDma(priv.dma_channel, priv.dma_buffer.data(), priv.dma_buffer.length());

    return true;
}

void LedController::maybeHandleDmaInterrupt()
{
    auto & priv = *p_;

    const std::uint32_t dma_flags = readDmaFlags(priv.dma_channel);
    if (0 == dma_flags)
        return;

    if (dma_flags & (1 << DmaFlags::DMA_ERROR))
    {
        // There is not error handling for now, just stop the DMA
        stopDma(priv.dma_channel);
        return;
    }

    const std::size_t half = (dma_flags & (1 << DmaFlags::DMA_HALF_COMPLETE)) ? 0 : 1;

    if (priv.dma_buffer.isBlankTerminated(half))
    {
        // Terminate DMA only after the half-buffer that just finished
        // transmitting was terminated by zeros, i.e. there was no more data to
        // be transmitted.
        stopDma(priv.dma_channel);
        return;
    }

    priv.data.readInto(half, &priv.dma_buffer);
}

}  // namespace driver
