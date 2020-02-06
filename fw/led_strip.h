/**
 * @file
 */

#ifndef LED_STRIP_H_
#define LED_STRIP_H_

#include <stdlib.h>
#include <stdint.h>

template <size_t LED_C>
struct LedStrip;

using AbstractLedStrip = LedStrip<1>;

struct LedState
{
    uint8_t green;
    uint8_t red;
    uint8_t blue;
};

/**
 * @brief State of a LED strip
 */
template <size_t LED_C>
struct LedStrip
{
    LedStrip():
        led_count(LED_C)
    { }

    const size_t led_count;
    LedState leds[LED_C];

    /**
     * @brief Operator for easy access to configured LED states
     *
     * @param led Led number
     * @return Reference to the LED state
     */
    LedState & operator[] (size_t led)
    {
        return leds[led];
    }

    /**
     * @brief Begin iteration
     *
     * @return Pointer to first LED state
     */
    LedState * begin()
    {
        return &(leds[0]);
    }

    /**
     * @brief End of iteration
     *
     * @return Pointer to past-end LED state
     */
    LedState * end()
    {
        return &(leds[led_count]);
    }

    /**
     * @brief Make abstract LED strip pointer
     *
     * @return Pointer to an abstract LED strip type
     */
    AbstractLedStrip * abstarctPtr()
    {
        return reinterpret_cast<AbstractLedStrip *>(this);
    }
};




#endif  // LED_STRIP_H_
