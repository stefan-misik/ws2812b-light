#include "app/animation_storage.hpp"

#include "animation/color.hpp"
#include "animation/rainbow.hpp"
#include "animation/retro.hpp"
#include "animation/twinkle.hpp"
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
};

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
    }
}
