#include "driver/base.hpp"
#include "driver/systick.hpp"
#include "driver/led_controller.hpp"
#include "tools/periodic_timer.hpp"

#include "support/cpu_pins.h"

#include "led_strip.h"


driver::Systick system_time;
driver::LedController led_controller;

PeriodicTimer led_update_timer;

LedStrip<100> leds;


class Rainbow
{
public:
    void update(AbstractLedStrip * led_strip)
    {
        ++step_;
        if (2 != step_)
            return;
        step_ = 0;

        LedState color;
        std::uint16_t hue = hue_;
        for (auto & led: *led_strip)
        {
            toSaturatedHue(hue, &color);
            hue = incrementHue(hue, 8);
            led = color;
        }

        hue_ = incrementHue(hue_, -4);
    }

private:
    std::uint16_t step_ = 0;
    std::uint16_t hue_ = 0;

    static const inline uint16_t MAX_HUE = 0x02FF;

    static void toSaturatedHue(uint16_t hue, LedState * color)
    {
        const uint8_t secondary_value = static_cast<uint8_t>(hue & 0xFF);
        const uint8_t primary_value = 0xFF - secondary_value;

        switch (static_cast<uint8_t>(hue >> 8))
        {
        case 0:
            color->red = primary_value;
            color->green = secondary_value;
            color->blue = 0;
            break;

        case 1:
            color->red = 0;
            color->green = primary_value;
            color->blue = secondary_value;
            break;

        case 2:
            color->red = secondary_value;
            color->green = 0;
            color->blue = primary_value;
            break;

        default:
            color->red = 0;
            color->green = 0;
            color->blue = 0;
            break;
        }
    }

    static uint16_t incrementHue(uint16_t hue, int8_t value = 1)
    {
        int16_t new_value = value + static_cast<int16_t>(hue);
        if (new_value < 0)
            return (MAX_HUE + 1) + new_value;
        else if (new_value > static_cast<int16_t>(MAX_HUE))
            return new_value - (MAX_HUE + 1);
        else
            return new_value;
    }
};

int main()
{
    driver::Base::init();
    driver::Systick::initialize();
    driver::LedController::initializeTimer(driver::TimerId::TIM_1);
    led_controller.initialize(driver::TimerId::TIM_1, 2, 0);
    driver::LedController::startTimer(driver::TimerId::TIM_1);

    Rainbow rainbow;
    while (true)
    {
        const std::uint32_t current_time = system_time.currentTime();

        if (led_update_timer.shouldRun(current_time, 8))
        {
            rainbow.update(leds.abstarctPtr());
            led_controller.update(leds.abstarctPtr());
        }
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
