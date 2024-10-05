#include "driver/base.hpp"
#include "driver/led_controller.hpp"

#include "support/cpu_pins.h"

#include "led_strip.h"


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
LedStrip<100> leds;


int main()
{
    driver::Base::init();
    driver::LedController::initializeTimer(driver::TimerId::TIM_1);
    led_controller.initialize(driver::TimerId::TIM_1, 2, 0);
    driver::LedController::startTimer(driver::TimerId::TIM_1);

    leds[0] = {0xF0, 0xF1, 0xF2};
    leds[1] = {0xF3, 0xF4, 0xF5};
    led_controller.update(leds.abstarctPtr());

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
