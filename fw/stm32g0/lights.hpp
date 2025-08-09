/**
 * @file
 */

#ifndef LIGHTS_HPP_
#define LIGHTS_HPP_

#include "driver/status_leds.hpp"
#include "event_queue.hpp"
#include "input.hpp"
#include "led_strip.h"


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


/**
 * @brief Class managing main application of the LED lights
 */
class Lights
{
public:
    /**
     * @brief Initialize the application
     *
     * @return Success
     */
    bool initialize();

    /**
     * @brief Led lights application step
     *
     * Call every 8 ms.
     *
     * @param current_time Current time in milliseconds
     */
    void step(std::uint32_t current_time);

    driver::StatusLeds & statusLeds() { return status_leds_; }
    Input & input() { return input_; }
    const AbstractLedStrip * leds() const { return leds_.abstarctPtr(); }

private:
    driver::StatusLeds status_leds_;

    EventQueue event_queue_;
    Input input_;
    Rainbow rainbow_;
    LedStrip<100> leds_;
};



#endif  // LIGHTS_HPP_
