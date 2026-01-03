#include "app/animation_storage.hpp"

#include "app/animation/tools/color_themes.hpp"
#include "app/animation/color.hpp"
#include "app/animation/rainbow.hpp"
#include "app/animation/retro.hpp"
#include "app/animation/twinkle.hpp"
#include "app/animation/shifting_color.hpp"
#include "app/animation/lights.hpp"
#include "app/animation.hpp"


namespace
{

using AnimationId = AnimationStorage::Register::AnimationId;
using AnimationSlotId = AnimationStorage::AnimationSlotId;

enum AnimationName: AnimationId
{
    ANIM_COLOR,
    ANIM_RAINBOW,
    ANIM_RETRO,
    ANIM_TWINKLE,
    ANIM_SHIFTING_COLOR,
    ANIM_LIGHTS,
};

enum AnimationSlotName: AnimationSlotId
{
    ANIM_SLOT_COLOR,
    ANIM_SLOT_RAINBOW,
    ANIM_SLOT_RETRO,
    ANIM_SLOT_RETRO_LAST = ANIM_SLOT_RETRO + (RetroAnimation::VARIANT_CNT - 1),
    ANIM_SLOT_TWINKLE,
    ANIM_SLOT_TWINKLE_LAST = ANIM_SLOT_TWINKLE + (TwinkleAnimation::VARIANT_CNT - 1),
    ANIM_SLOT_SHIFTING_COLOR,
    ANIM_SLOT_SHIFTING_COLOR_LAST = ANIM_SLOT_SHIFTING_COLOR + (ShiftingColorAnimation::VARIANT_CNT - 1),
    ANIM_SLOT_LIGHTS,
    ANIM_SLOT_LIGHTS_LAST = ANIM_SLOT_LIGHTS + ((COLOR_THEME_COUNT * 2) - 1),

    ANIM_SLOT_COUNT_,
};

static_assert(AnimationStorage::SLOT_COUNT == AnimationSlotName::ANIM_SLOT_COUNT_, "Check animation count");

void makeAnimation(AnimationStorage::Storage * storage, AnimationId id)
{
    switch (id)
    {
        case ANIM_COLOR: storage->create<ColorAnimation>(); return;
        case ANIM_RAINBOW: storage->create<RainbowAnimation>(); return;
        case ANIM_RETRO: storage->create<RetroAnimation>(); return;
        case ANIM_TWINKLE: storage->create<TwinkleAnimation>(); return;
        case ANIM_SHIFTING_COLOR: storage->create<ShiftingColorAnimation>(); return;
        case ANIM_LIGHTS: storage->create<LightsAnimation>(); return;
    }
    // Default
    storage->create<ColorAnimation>();
}

AnimationId makeDefaultSlot(AnimationStorage::Storage * storage, AnimationSlotId slot_id)
{
    switch (slot_id)
    {
    case ANIM_SLOT_COLOR:
    default:
        makeAnimation(storage, ANIM_COLOR);
        return ANIM_COLOR;

    case ANIM_SLOT_RAINBOW:
        makeAnimation(storage, ANIM_RAINBOW);
        return ANIM_RAINBOW;

    case ANIM_SLOT_RETRO ... ANIM_SLOT_RETRO_LAST:
        makeAnimation(storage, ANIM_RETRO);
        {
            const std::uint8_t variant_id = slot_id - ANIM_SLOT_RETRO;
            (*storage)->setParamater(RetroAnimation::VARIANT, variant_id);
        }
        return ANIM_RETRO;

    case ANIM_SLOT_TWINKLE ... ANIM_SLOT_TWINKLE_LAST:
        makeAnimation(storage, ANIM_TWINKLE);
        {
            const std::uint8_t variant_id = slot_id - ANIM_SLOT_TWINKLE;
            (*storage)->setParamater(TwinkleAnimation::VARIANT, variant_id);
        }
        return ANIM_TWINKLE;

    case ANIM_SLOT_SHIFTING_COLOR ... ANIM_SLOT_SHIFTING_COLOR_LAST:
        makeAnimation(storage, ANIM_SHIFTING_COLOR);
        {
            const std::uint8_t variant_id = slot_id - ANIM_SLOT_SHIFTING_COLOR;
            (*storage)->setParamater(ShiftingColorAnimation::VARIANT, variant_id);
        }
        return ANIM_SHIFTING_COLOR;

    case ANIM_SLOT_LIGHTS ... ANIM_SLOT_LIGHTS_LAST:
        makeAnimation(storage, ANIM_LIGHTS);
        {
            const std::uint8_t modifier = slot_id - ANIM_SLOT_LIGHTS;
            const auto theme_id = static_cast<ColorTheme>(modifier >= COLOR_THEME_COUNT ?
                modifier - COLOR_THEME_COUNT : modifier);
            const bool is_synchronized = modifier >= COLOR_THEME_COUNT;
            applyColorTheme(storage->get(), theme_id);
            (*storage)->setParamater(LightsAnimation::SYNCHRONIZED, is_synchronized);
        }
        return ANIM_LIGHTS;
    }
}

}  // namespace

AnimationStorage::AnimationStorage():
    slot_id_(0),
    storage_{TypeTag<ColorAnimation>{}}  // Whatever
{
    initialize();

    // Load the animation
    auto & reg = slots_[slot_id_];
    makeAnimation(&storage_, reg.animationId());
    reg.restore(storage_.get(), Animation::DataType::BOTH);
}

void AnimationStorage::initialize()
{
    Storage tmp_storage(TypeTag<ColorAnimation>{});
    for (AnimationSlotId slot_id = 0; slot_id != SLOT_COUNT; ++slot_id)
    {
        const auto anim_id = makeDefaultSlot(&tmp_storage, slot_id);
        auto & slot = slots_[slot_id];
        slot.setAnimationId(anim_id);
        slot.store(tmp_storage.get(), Animation::DataType::BOTH);
    }
}

void AnimationStorage::initializeCurrentSlot()
{
    makeDefaultSlot(&storage_, slot_id_);
    slots_[slot_id_].store(storage_.get(), Animation::DataType::BOTH);
}

bool AnimationStorage::change(AnimationSlotId new_slot_id)
{
    if (new_slot_id == slot_id_)
        return true;
    if (new_slot_id >= SLOT_COUNT)
        return false;

    slots_[slot_id_].store(storage_.get(), Animation::DataType::BOTH);

    auto & reg = slots_[new_slot_id];
    makeAnimation(&storage_, reg.animationId());
    reg.restore(storage_.get(), Animation::DataType::BOTH);
    slot_id_ = new_slot_id;
    return true;
}
