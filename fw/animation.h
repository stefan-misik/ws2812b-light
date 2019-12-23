/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "led_strip.h"

#include <stdlib.h>
#include <stdint.h>

/**
 * @brief Class defining a single type of animation
 */
class Animation
{
public:
    /**
     * @brief Reset the animation
     *
     * @param[out] led_strip Pointer to LED strip on which to perform the
     *             animation
     */
    virtual void reset(AbstractLedStrip * led_strip) = 0;

    /**
     * @brief Perform a step in the animation
     *
     * @param[out] led_strip Pointer to LED strip on which to perform the
     *             animation
     *
     * @return Number of milliseconds to wait before another animation step is
     *         performed
     * @retval 0 The animation has ended, reset the animation in order to loop
     */
    virtual uint8_t step(AbstractLedStrip * led_strip) = 0;

    virtual ~Animation() = default;
};




#endif  // ANIMATION_H_
