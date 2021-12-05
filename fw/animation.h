/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "led_strip.h"
#include "buttons.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Class defining a single type of animation
 */
class Animation
{
public:
    using ButtonId = Buttons::ButtonId;
    using ButtonState = ButtonFilter::State;

    /**
     * @brief Start the animation - initialize the strip
     *
     * @return Re-render the strip as soon as possible
     */
    virtual bool start(AbstractLedStrip * leds)
    {
        return false;
    }

    /**
     * @brief Render animation frame
     *
     * @return Re-render the strip
     */
    virtual bool update(AbstractLedStrip * leds)
    {
        return false;
    }

    /** @brief Stop the animation - ??? */
    virtual void stop(AbstractLedStrip * leds) { }

    /**
     * @brief Handle an event of a button
     *
     * @return Perform the default operation
     */
    virtual bool handleButton(ButtonId button, uint8_t state)
    {
        return true;
    }

    virtual ~Animation() = default;
};


#endif  // ANIMATION_H_
