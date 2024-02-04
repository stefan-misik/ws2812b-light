/**
 * @file
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdlib.h>
#include <stdint.h>

#include "tools/color.h"
#include "led_strip.h"


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
     */
    static void initialize();

    /**
     * @brief Update status LED only
     *
     * @param status Color to be used for the status LED
     */
    static void updateStatus(ColorId status);

    /**
     * @brief Transfer the values of the LEDs into the strip
     *
     * @param[in] led_strip Led strip to be associated with the LED controller
     * @param status Color to be used for the status LED
     */
    static void update(const AbstractLedStrip * led_strip, ColorId status);
};


#endif  // LEDS_H_
