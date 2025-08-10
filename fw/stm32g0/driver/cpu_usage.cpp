#include "driver/cpu_usage.hpp"

#include <limits>
#include <algorithm>

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_system.h"


namespace driver
{
namespace
{

::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_6: return TIM6;
    default: break;
    }
    return nullptr;
}

void freezeTimerInCpuStop(TimerId tim_id)
{
#ifdef DEBUG
    switch (tim_id)
    {
    case TimerId::TIM_6: ::LL_DBGMCU_APB1_GRP1_FreezePeriph(LL_DBGMCU_APB1_GRP1_TIM6_STOP); break;
    default: break;
    }
#endif  // DEBUG
}


}  // namespace


struct CpuUsage::Private
{
    ::TIM_TypeDef * tim;
};


CpuUsage::CpuUsage()
{
}

CpuUsage::~CpuUsage()
{
}

bool CpuUsage::initialize(TimerId tim_id)
{
    auto & priv = *p_;

    priv.tim = toTimer(tim_id);
    if (nullptr == priv.tim)
        return false;

    ::LL_TIM_InitTypeDef tim_init;
    ::LL_TIM_StructInit(&tim_init);

    tim_init.Prescaler = 64 - 1;  // Divide clock (64 MHz) by 64
    tim_init.CounterMode = LL_TIM_COUNTERMODE_UP;
    tim_init.Autoreload = 0xFFFFFFFF;
    tim_init.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    if (SUCCESS != ::LL_TIM_Init(priv.tim, &tim_init))
        return false;

    ::LL_TIM_DisableARRPreload(priv.tim);
    ::LL_TIM_SetClockSource(priv.tim, LL_TIM_CLOCKSOURCE_INTERNAL);
    ::LL_TIM_DisableMasterSlaveMode(priv.tim);
    ::LL_TIM_DisableIT_UPDATE(priv.tim);

    // Freeze the trigger timer in CPU stop mode
    freezeTimerInCpuStop(tim_id);

    ::LL_TIM_EnableCounter(priv.tim);

    // Reset the statistics
    stats_.min = std::numeric_limits<TimerType>::max();
    stats_.max = 0;
    stats_.last = 0;

    return true;
}

void CpuUsage::startPeriod()
{
    previous_start_ = ::LL_TIM_GetCounter(p_->tim);
}

void CpuUsage::endPeriod()
{
    const TimerType elapsed = ::LL_TIM_GetCounter(p_->tim) - previous_start_;

    if (clear_stats_)
    {
        stats_.max = elapsed;
        stats_.min = elapsed;
        clear_stats_ = false;
    }
    else
    {
        if (elapsed < stats_.min)
            stats_.min = elapsed;
        if (elapsed > stats_.max)
            stats_.max = elapsed;
    }

    stats_.last = elapsed;
}

}  // namespace driver
