/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "tools/flags.hpp"
#include "led_strip.h"

#include <stdlib.h>
#include <stdint.h>


/**
 * @brief Class defining a single type of animation
 */
class Animation
{
public:
    enum class ChangeType
    {
        ABSOLUTE,
        RELATIVE,
    };

    enum class RenderFlag
    {
        INITIALIZE = 0,
        MUSIC_STOPPED,
        NOTE_CHANGED,
    };

    virtual void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) = 0;

    virtual void setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) = 0;
    virtual int getParameter(std::uint32_t param_id) = 0;

    virtual std::size_t storeConfiguration(void * buffer, std::size_t capacity) = 0;
    virtual bool restoreConfiguration(const void * buffer, std::size_t size) = 0;

    virtual ~Animation() = default;
};

#endif  // ANIMATION_H_
