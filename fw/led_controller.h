/**
 * @file
 */

#ifndef LEDS_H_
#define LEDS_H_

#include "led_strip.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Object handling the communication with WS2812B LEDs
 */
class LedController
{
public:
    /**
     * @brief Initialize the LED state storage and hardware necessary to
     *        communicate with LEDs
     *
     * @param[in,out] led_strip Led strip to be associated with the LED
     *                controller
     */
    void initialize(AbstractLedStrip * led_strip);

    /**
     * @brief Transfer the values of the LEDs into the strip
     */
    void update() const;

private:
    AbstractLedStrip * strip_;
};


#endif  // LEDS_H_
