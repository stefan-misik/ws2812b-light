#include "driver/base.hpp"
#include "driver/systick.hpp"
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

driver::Systick system_time;
driver::LedController led_controller;
LedStrip<100> leds;


int main()
{
    driver::Base::init();
    driver::Systick::initialize();
    driver::LedController::initializeTimer(driver::TimerId::TIM_1);
    led_controller.initialize(driver::TimerId::TIM_1, 2, 0);
    driver::LedController::startTimer(driver::TimerId::TIM_1);

    while (true)
    {
        std::uint8_t b = 0;
        for (std::size_t n = 0; n != leds.led_count; ++n)
        {
            leds[n] = {
                    static_cast<std::uint8_t>(b),
                    static_cast<std::uint8_t>(b + 1),
                    static_cast<std::uint8_t>(b + 2)
            };
            b += 3;
        }
        led_controller.update(leds.abstarctPtr());

        sleep();
    }

    return 0;
}


extern "C" void SysTick_Handler()
{
    system_time.handleInterrupt();
}

extern "C" void DMA1_Channel1_IRQHandler()
{
    led_controller.maybeHandleDmaInterrupt();
}
