#include "driver/led_controller.hpp"

#include <new>

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_dma.h"
#include "stm32g0xx_ll_dmamux.h"



namespace driver
{
namespace
{

// T1H = 0.8 us, (0.8us / 1.25us) * (80 - 1) ~= 51
const std::uint16_t ONE_BIT_LENGTH = 51;
// T1H = 0.4 us, (0.4us / 1.25us) * (80 - 1) ~= 25
const std::uint16_t ZERO_BIT_LENGTH = 25;

::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_1: return TIM1;
    case TimerId::TIM_3: return TIM3;
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
    case 0: return LL_DMAMUX_CHANNEL_0;
    case 1: return LL_DMAMUX_CHANNEL_1;
    case 2: return LL_DMAMUX_CHANNEL_2;
    case 3: return LL_DMAMUX_CHANNEL_3;
    case 4: return LL_DMAMUX_CHANNEL_4;
    case 5: return LL_DMAMUX_CHANNEL_5;
    case 6: return LL_DMAMUX_CHANNEL_6;
    }
    return 0xFFFFFFFF;
}

std::uint32_t toDmaRequest(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_1: return LL_DMAMUX_REQ_TIM1_UP;
    case TimerId::TIM_3: return LL_DMAMUX_REQ_TIM3_UP;
    }

    return 0;
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

    ::LL_TIM_OC_DisableFast(tim, channel);
    ::LL_TIM_CC_EnableChannel(tim, channel);
    return true;
}

bool initializeDma(std::uint32_t channel, std::uint32_t request)
{
    ::LL_DMAMUX_SetRequestID(DMAMUX1, channel, request);
    ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, channel);
    ::LL_DMAMUX_DisableSync(DMAMUX1, channel);
    ::LL_DMAMUX_DisableRequestGen(DMAMUX1, channel);

    return true;
}


class BitReader
{
public:
    void start()
    {
        pos_ = 0;
    }

    /**
     * @brief Read data to bit buffer
     *
     * @param[out] buffer Buffer to receive bit pattern
     * @param capacity Capacity of the bit pattern buffer (for ideal operation,
     *        capacity should be multiples of 8)
     *
     * @return Number of bytes written in the buffer
     * @retval 0 Done writing or insufficient buffer
     */
    std::size_t read(std::uint8_t * buffer, std::size_t capacity, const void * data, std::size_t length)
    {
        const std::uint8_t * data_pos = reinterpret_cast<const std::uint8_t *>(data) + pos_;
        const std::uint8_t * const data_end = reinterpret_cast<const std::uint8_t *>(data) + length;
        std::uint8_t * buffer_pos = buffer;
        std::size_t remaining = capacity >> 3;

        for (; data_pos != data_end; ++data_pos)
        {
            if (remaining < 8)
                break;
            remaining -= 8;
            std::uint8_t data_byte = *data_pos;
            std::uint8_t * const buffer_end = buffer_pos + 8;
            for (; buffer_pos != buffer_end; ++buffer_pos)
            {
                *buffer_pos = (data_byte & 1u) ? ONE_BIT_LENGTH : ZERO_BIT_LENGTH;
                data_byte >>= 1;
            }
        }

        pos_ = data_pos - reinterpret_cast<const std::uint8_t *>(data);
        return capacity - remaining;
    }

private:
    std::size_t pos_;
};

}  // namespace


struct LedController::Private
{
    ::TIM_TypeDef * tim;
    std::uint32_t channel;
    std::uint32_t dma_channel;
};

bool LedController::initializeTimer(TimerId tim_id)
{
    auto * const tim = toTimer(tim_id);
    if (nullptr == tim)
        return false;

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

bool LedController::startTimer(TimerId tim_id)
{
    auto * const tim = toTimer(tim_id);
    if (nullptr == tim)
        return false;

    ::LL_TIM_EnableCounter(tim);
    return true;
}

LedController::LedController()
{
    static_assert(sizeof(Private) == sizeof(PrivateStorage), "");
    static_assert(alignof(Private) == alignof(PrivateStorage), "");

    new (&p_) Private();
}

LedController::~LedController()
{
    p().~Private();
}


bool LedController::initialize(TimerId tim_id, uint8_t channel_id, std::uint8_t dma_channel_id)
{
    auto * const tim = toTimer(tim_id);
    const std::uint32_t channel = toChannel(channel_id);
    if (nullptr == tim || 0 == channel)
        return false;

    if (!initializeChannel(tim, channel))
        return false;

    const std::uint32_t dma_request = toDmaRequest(tim_id);
    const std::uint32_t dma_channel = toDmaChannel(dma_channel_id);
    if (0 == dma_request || 0XFFFFFFFF == dma_channel)
        return false;

    if (!initializeDma(dma_channel, dma_request))
        return false;

    auto & priv = p();
    priv.tim = tim;
    priv.channel = channel;
    priv.dma_channel = dma_channel;

    forcePwm(0);
    return true;
}

void LedController::forcePwm(std::uint16_t value)
{
    switch (p().channel)
    {
    case LL_TIM_CHANNEL_CH1: ::LL_TIM_OC_SetCompareCH1(p().tim, value); break;
    case LL_TIM_CHANNEL_CH2: ::LL_TIM_OC_SetCompareCH2(p().tim, value); break;
    case LL_TIM_CHANNEL_CH3: ::LL_TIM_OC_SetCompareCH3(p().tim, value); break;
    case LL_TIM_CHANNEL_CH4: ::LL_TIM_OC_SetCompareCH4(p().tim, value); break;
    }
}

}  // namespace driver
