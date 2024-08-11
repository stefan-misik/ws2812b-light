#include "driver/led_controller.hpp"

#include <new>

#include "stm32g0xx_ll_tim.h"



namespace driver
{
namespace
{

// T1H = 0.8 us, (0.8us / 1.25us) * (80 - 1) ~= 51
const std::uint16_t ONE_BIT_LENGTH = 51;
// T1H = 0.4 us, (0.4us / 1.25us) * (80 - 1) ~= 25
const std::uint16_t ZERO_BIT_LENGTH = 25;

::TIM_TypeDef * to_timer(LedController::TimerId tim_id)
{
    switch (tim_id)
    {
    case LedController::TimerId::TIM_1: return TIM1;
    }
    return nullptr;
}

std::uint32_t to_channel(std::uint32_t channel_id)
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

}  // namespace


struct LedController::Private
{
    ::TIM_TypeDef * tim;
    std::uint32_t channel;
};

bool LedController::initializeTimer(TimerId tim_id)
{
    auto * const tim = to_timer(tim_id);
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
    auto * const tim = to_timer(tim_id);
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


bool LedController::initialize(TimerId tim_id, uint8_t channel_id)
{
    auto * const tim = to_timer(tim_id);
    const std::uint32_t channel = to_channel(channel_id);
    if (nullptr == tim || 0 == channel)
        return false;

    if (!initializeChannel(tim, channel))
        return false;


    auto & priv = p();
    priv.tim = tim;
    priv.channel = channel;

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
