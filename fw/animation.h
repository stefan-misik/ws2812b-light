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
    enum class Event: uint8_t
    {
        CREATE,
        INIT,
        DEINIT,
        STEP,
        KEY_PRESS,
        KEY_DOWN,
        KEY_UP,
    };

    enum class Result: uint8_t
    {
        NONE,
        IGNORE_DEFAULT
    };

    /**
     * @brief Handle an event of a key being pressed
     *
     * @param event Event to handle
     * @param parameter Additional data associated with the parameter
     *
     * @return Result of the action
     */
    virtual Result handleEvent(Event event, intptr_t parameter) = 0;

    virtual ~Animation() = default;
};




#endif  // ANIMATION_H_
