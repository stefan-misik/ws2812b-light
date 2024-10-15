#include "driver/ir_receiver.hpp"

#include <new>

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_cortex.h"


namespace driver
{
namespace
{


::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_3: return TIM3;
    default: break;
    }
    return nullptr;
}

std::uint32_t toWidthChannel(IrReceiver::ChannelPair channel_pair_id)
{
    switch (channel_pair_id)
    {
    case IrReceiver::ChannelPair::CHANNEL_1_2: return LL_TIM_CHANNEL_CH1;
    case IrReceiver::ChannelPair::CHANNEL_3_4: return LL_TIM_CHANNEL_CH3;
    }
    return 0;
}

std::uint32_t toPeriodChannel(IrReceiver::ChannelPair channel_pair_id)
{
    switch (channel_pair_id)
    {
    case IrReceiver::ChannelPair::CHANNEL_1_2: return LL_TIM_CHANNEL_CH2;
    case IrReceiver::ChannelPair::CHANNEL_3_4: return LL_TIM_CHANNEL_CH4;
    }
    return 0;
}

bool initializeTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_InitTypeDef init;
    ::LL_TIM_StructInit(&init);
    init.Prescaler = 2 << 12;
    init.CounterMode = LL_TIM_COUNTERDIRECTION_UP;
    init.Autoreload = 0;
    init.ClockDivision = LL_TIM_CLOCKDIVISION_DIV4;  // 16 MHz as input
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

bool initializeChannels(::TIM_TypeDef * tim, std::uint32_t width_channel, std::uint32_t period_channel)
{
    (void)tim;
    (void)width_channel;
    (void)period_channel;
    return true;
}

inline void startTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_EnableCounter(tim);
}


}  // namespace


struct IrReceiver::Private
{
    ::TIM_TypeDef * tim;
    std::uint32_t width_channel;
    std::uint32_t period_channel;
};


IrReceiver::IrReceiver()
{
    static_assert(sizeof(Private) == sizeof(PrivateStorage), "Check size of the private storage");
    static_assert(alignof(Private) == alignof(PrivateStorage), "Check alignment of the private storage");

    new (&p_) Private();
}

IrReceiver::~IrReceiver()
{
    p().~Private();
}


bool IrReceiver::initialize(TimerId tim_id, ChannelPair channel_pair_id)
{
    auto * const tim = toTimer(tim_id);
    const std::uint32_t width_channel = toWidthChannel(channel_pair_id);
    const std::uint32_t period_channel = toPeriodChannel(channel_pair_id);
    if (nullptr == tim || 0 == width_channel || 0 == period_channel)
        return false;

    if (!initializeTimer(tim))
        return false;

    if (!initializeChannels(tim, width_channel, period_channel))
        return false;

    auto & priv = p();
    priv.tim = tim;
    priv.width_channel = width_channel;
    priv.period_channel = period_channel;

    startTimer(tim);

    return true;
}


}  // namespace driver
