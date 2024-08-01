#include "driver/base.hpp"

#include "support/cpu_pins.h"

namespace
{

void sleep()
{
    for (unsigned n = 0; n != 1000000; ++n)
    {
        asm volatile("nop");
    }
}

}  // namespace

int main()
{
    driver::Base::init();

    while (true)
    {
        ::LL_GPIO_ResetOutputPin(STATUS_LED3_GPIO_Port, STATUS_LED3_Pin);
        ::LL_GPIO_SetOutputPin(STATUS_LED1_GPIO_Port, STATUS_LED1_Pin);
        sleep();

        ::LL_GPIO_ResetOutputPin(STATUS_LED1_GPIO_Port, STATUS_LED1_Pin);
        ::LL_GPIO_SetOutputPin(STATUS_LED2_GPIO_Port, STATUS_LED2_Pin);
        sleep();

        ::LL_GPIO_ResetOutputPin(STATUS_LED2_GPIO_Port, STATUS_LED2_Pin);
        ::LL_GPIO_SetOutputPin(STATUS_LED3_GPIO_Port, STATUS_LED3_Pin);
        sleep();
    }

    return 0;
}
