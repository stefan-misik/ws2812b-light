/**
 * @file
 */

#ifndef ANIMATION_LIST_H_
#define ANIMATION_LIST_H_

#include <stdint.h>

#include "animation.h"


class AnimationStorage
{
public:
    static const uint8_t MAX_SIZE = 8;  //TODO: lower when individual animations are refactored
    using AlignType = char;

    enum AnimationId: uint8_t
    {
        ANIM_COLOR = 0,
        ANIM_RAINBOW,
        ANIM_RETRO,
        ANIM_TWINKLE_SPARTKS,
        ANIM_TWINKLE_SHIMMER,
        ANIM_SHIFTING_COLOR,

        // Should be last
        ANIM_COUNT_
    };
    static const uint8_t COUNT = static_cast<uint8_t>(AnimationId::ANIM_COUNT_);

    AnimationStorage();

    Animation * current() { return reinterpret_cast<Animation *>(storage_); }
    const Animation * current() const { return reinterpret_cast<const Animation *>(storage_); }
    uint8_t currentId() const { return current_id_; }

    void change(uint8_t id);

private:
    alignas (AlignType)
    char storage_[MAX_SIZE];

    uint8_t current_id_;
};


#endif /* ANIMATION_LIST_H_ */
