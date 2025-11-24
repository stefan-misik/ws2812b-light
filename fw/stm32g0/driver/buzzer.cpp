#include "driver/buzzer.hpp"

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_cortex.h"


namespace driver
{
namespace
{

const std::uint8_t NOTES[16] = {
        239 - 1,  //  0: C
        226 - 1,  //  1: Cs
        213 - 1,  //  2: D
        201 - 1,  //  3: Ds
        190 - 1,  //  4: E
        179 - 1,  //  5: F
        169 - 1,  //  6: Fs
        160 - 1,  //  7: G
        151 - 1,  //  8: Gs
        142 - 1,  //  9: A
        134 - 1,  // 10: As
        127 - 1   // 11: B
};


::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_16: return TIM16;
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

    ::LL_TIM_EnableARRPreload(tim);
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
    oc_init.OCMode = LL_TIM_OCMODE_TOGGLE;
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

    ::LL_TIM_DisableDMAReq_UPDATE(tim);
    ::LL_TIM_OC_DisableFast(tim, channel);
    ::LL_TIM_CC_EnableChannel(tim, channel);
    return true;
}

inline void startTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_EnableCounter(tim);
}

void setCompare(::TIM_TypeDef * tim, std::uint32_t channel, std::uint8_t value)
{
    switch (channel)
    {
    case LL_TIM_CHANNEL_CH1: ::LL_TIM_OC_SetCompareCH1(tim, value); break;
    case LL_TIM_CHANNEL_CH2: ::LL_TIM_OC_SetCompareCH2(tim, value); break;
    case LL_TIM_CHANNEL_CH3: ::LL_TIM_OC_SetCompareCH3(tim, value); break;
    case LL_TIM_CHANNEL_CH4: ::LL_TIM_OC_SetCompareCH4(tim, value); break;
    }
}

inline void resetOutput(::TIM_TypeDef * tim, std::uint32_t channel)
{
    ::LL_TIM_OC_SetMode(tim, channel, LL_TIM_OCMODE_FORCED_INACTIVE);
    ::LL_TIM_OC_SetMode(tim, channel, LL_TIM_OCMODE_TOGGLE);
}


}  // namespace


struct Buzzer::Private
{
    ::TIM_TypeDef * tim;
    std::uint32_t channel;
};


Buzzer::Buzzer()
{
}

Buzzer::~Buzzer()
{
}


bool Buzzer::initialize(TimerId tim_id, uint8_t channel_id)
{
    auto * const tim = toTimer(tim_id);
    const std::uint32_t channel = toChannel(channel_id);
    if (nullptr == tim || 0 == channel)
        return false;

    if (!initializeTimer(tim))
        return false;

    if (!initializeChannel(tim, channel))
        return false;

    auto & priv = *p_;
    priv.tim = tim;
    priv.channel = channel;

    setCompare(tim, channel, 0);

    startTimer(tim);

    return true;
}

void Buzzer::playNote(MusicNote note)
{
    auto & priv = *p_;

    if (note.isValid())
    {
        const std::uint32_t previous_auto_reload = ::LL_TIM_GetAutoReload(priv.tim);

        ::LL_TIM_SetPrescaler(priv.tim, 2 << (12 - note.octave()));
        ::LL_TIM_SetAutoReload(priv.tim, NOTES[note.tone()]);

        // In case the timer is stopped, generate update event to transfer new
        // auto-reload value into shadow register
        if (0 == previous_auto_reload)
            ::LL_TIM_GenerateEvent_UPDATE(priv.tim);
    }
    else
    {
        ::LL_TIM_SetPrescaler(priv.tim, 2 << 12);
        ::LL_TIM_SetAutoReload(priv.tim, 0);

        // Force toggling output-compare output into low state, otherwise ther's
        // 50:50 chance that outputs stays high, causing buzzer to whine
        resetOutput(priv.tim, priv.channel);
    }
}

}  // namespace driver
