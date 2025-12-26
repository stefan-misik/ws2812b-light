#include "app/animation_storage.hpp"

#include "app/animation/color.hpp"
#include "app/animation/rainbow.hpp"
#include "app/animation/retro.hpp"
#include "app/animation/twinkle.hpp"
#include "app/animation/shifting_color.hpp"
#include "app/animation.hpp"


namespace
{

enum AnimationName: AnimationStorage::AnimationId
{
    ANIM_COLOR,
    ANIM_RAINBOW,
    ANIM_RETRO,
    ANIM_RETRO_LAST = ANIM_RETRO + (RetroAnimation::VARIANT_CNT - 1),
    ANIM_TWINKLE,
    ANIM_TWINKLE_LAST = ANIM_TWINKLE + (TwinkleAnimation::VARIANT_CNT - 1),
    ANIM_SHIFTING_COLOR,
    ANIM_SHIFTING_COLOR_LAST = ANIM_SHIFTING_COLOR + (ShiftingColorAnimation::VARIANT_CNT - 1),

    ANIMATION_COUNT_,
};

static_assert(AnimationStorage::ANIMATION_COUNT == ANIMATION_COUNT_, "Check animation count");

}  // namespace

AnimationStorage::AnimationStorage():
    storage_{TypeTag<RainbowAnimation>{}}
{
}

void AnimationStorage::change(AnimationId id)
{
    switch (id)
    {
    case ANIM_COLOR:
    default:
        storage_.create<ColorAnimation>();
        break;
    case ANIM_RAINBOW:
        storage_.create<RainbowAnimation>();
        break;
    case ANIM_RETRO ... ANIM_RETRO_LAST:
        storage_.create<RetroAnimation>();
        {
            const std::uint8_t variant_id = id - ANIM_RETRO;
            storage_->setParamater(RetroAnimation::VARIANT, variant_id);
        }
        break;
    case ANIM_TWINKLE ... ANIM_TWINKLE_LAST:
        storage_.create<TwinkleAnimation>();
        {
            const std::uint8_t variant_id = id - ANIM_TWINKLE;
            storage_->setParamater(TwinkleAnimation::VARIANT, variant_id);
        }
        break;
    case ANIM_SHIFTING_COLOR ... ANIM_SHIFTING_COLOR_LAST:
        storage_.create<ShiftingColorAnimation>();
        {
            const std::uint8_t variant_id = id - ANIM_SHIFTING_COLOR;
            storage_->setParamater(ShiftingColorAnimation::VARIANT, variant_id);
        }
        break;
    }
}
