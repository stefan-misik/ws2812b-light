#include "driver/systick.hpp"


#include "stm32g0xx_ll_cortex.h"


namespace driver
{


bool Systick::initialize()
{
    // Set SysTick to fire each millisecond
    return 0 == SysTick_Config(SystemCoreClock / 1000);
}

}  // namespace driver
