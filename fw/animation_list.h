/**
 * @file
 */

#ifndef ANIMATION_LIST_H_
#define ANIMATION_LIST_H_

#include <stdint.h>

#include "animation.h"
#include "animations/rainbow.h"
#include "animations/color.h"
#include "animations/retro.h"
#include "animations/twinkle.h"


class AnimationList
{
public:
    AnimationList();

    Animation * current() { return current_; }
    uint8_t currentId() const { return current_id_; }

    uint8_t size() const
    {
        return 5;
    }

    void setCurrentId(uint8_t id)
    {
        current_ = getById(id);
        current_id_ = id;
    }

    Animation * getById(uint8_t id);

private:
    Animation * current_;
    uint8_t current_id_;

    ColorAnimation color_;
    RainbowAnimation rainbow_;
    RetroAnimation retro_;
    TwinkleAnimation twinkle_sparks_;
    TwinkleAnimation twinkle_shimmer_;
};




#endif /* ANIMATION_LIST_H_ */
