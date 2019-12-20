/**
 * @file
 */

#ifndef LEDS_H_
#define LEDS_H_

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Object handling the communication with WS2812B LEDs
 */
class Leds
{
public:
    struct LedState
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    /**
     * @brief Initialize the LED state storage and hardware necessary to
     *        communicate with LEDs
     *
     * @param[in,out] leds Buffer holding states of the LEDs
     * @param count Count of the led states in the buffer
     */
    void initialize(LedState * leds, size_t count);

    /**
     * @brief Operator for easy access to configured LED states
     *
     * @param led Led number
     * @return Reference to the LED state
     */
    LedState & operator[] (size_t led)
    {
        return leds_[led];
    }

private:
    LedState * leds_;
    size_t count_;
};


#endif  // LEDS_H_
