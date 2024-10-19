/**
 * @file
 */

#ifndef DRIVER_STATUS_LEDS_HPP_
#define DRIVER_STATUS_LEDS_HPP_

#include "driver/common.hpp"


namespace driver
{


class StatusLeds
{
public:
    enum class LedId
    {
        LED_RED,
        LED_YELLOW,
        LED_GREEN,
    };

    static const inline std::size_t LED_COUNT = 3;

    bool initialize();

    void setLed(LedId led, bool value)
    {
        led_state_[static_cast<std::size_t>(led)] = value;
    }

    /**
     * @brief Update keypad
     *
     * This needs to be called in regular intervals.
     */
    void update();

private:
    bool led_state_[LED_COUNT];
};


}  // driver


#endif  // DRIVER_STATUS_LEDS_HPP_
