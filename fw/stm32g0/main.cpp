#include "driver/base.hpp"
#include "driver/led_controller.hpp"

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

driver::LedController led_controller;

int main()
{
    driver::Base::init();
    driver::LedController::initializeTimer(driver::TimerId::TIM_1);
    led_controller.initialize(driver::TimerId::TIM_1, 2);
    driver::LedController::startTimer(driver::TimerId::TIM_1);

    uint8_t state = 0;
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

        switch (state)
        {
        case 0: led_controller.forcePwm(51); state = 1; break;
        case 1: led_controller.forcePwm(0); state = 2; break;
        case 2: led_controller.forcePwm(25); state = 3; break;
        case 3: led_controller.forcePwm(0); state = 0; break;
        }
    }

    return 0;
}
