#include "app/animation_storage.hpp"

#include <utility>

#include "app/tools/color.hpp"
#include "app/animation.hpp"
#include "app/animation/tools/animation_tools.hpp"
#include "app/animation/color.hpp"
#include "app/animation/rainbow.hpp"
#include "app/animation/retro.hpp"
#include "app/animation/twinkle.hpp"
#include "app/animation/shifting_color.hpp"
#include "app/animation/lights.hpp"


namespace
{

using AnimationId = AnimationStorage::Register::AnimationId;
using AnimationSlotId = AnimationStorage::AnimationSlotId;
using AnimationSlotName = AnimationStorage::AnimationSlotName;

enum AnimationName: AnimationId
{
    ANIM_COLOR,
    ANIM_RAINBOW,
    ANIM_RETRO,
    ANIM_TWINKLE,
    ANIM_SHIFTING_COLOR,
    ANIM_LIGHTS,
};

static_assert(
    RetroAnimation::VARIANT_CNT ==
    (static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_RETRO_LAST) -
    static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_RETRO) + 1u), "Check Retro Animation variant count");


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

bool applyKeyFrames(Animation * animation, std::uint32_t variant)
{
    using KeyFrame = TwinkleAnimation::KeyFrame;
    auto apply = [](Animation * animation, std::initializer_list<KeyFrame> lengths)
    {
        static const auto FIRST_PARAM = TwinkleAnimation::ParamId::KEY_FRAME_FIRST;
        static const auto PARAM_COUNT = TwinkleAnimation::ParamId::KEY_FRAME_COUNT;
        return setParameterGroup<KeyFrame, TwinkleAnimation::KeyFrameParam, FIRST_PARAM, PARAM_COUNT>(animation,
            lengths);
    };

    switch (variant)
    {
    default:
    case 0:
        return apply(animation, {
            KeyFrame{LedState{0x47300D}, 0u},
            KeyFrame{LedState{0xFFFFFF}, 5u},
            KeyFrame{LedState{0xEDCC9C}, 5u},
            KeyFrame{LedState{0x47300D}, 20u}
        });

    case 1:
        return apply(animation, {
            KeyFrame{LedState{0x47300D}, 0u},
            KeyFrame{LedState{0x040301}, 15u},
            KeyFrame{LedState{0x47300D}, 15u}
        });

    case 2:
        return apply(animation, {
            KeyFrame{getColor(ColorId::BLUE), 0u},
            KeyFrame{getColor(ColorId::YELLOW), 15u},
            KeyFrame{getColor(ColorId::YELLOW), 25u},
            KeyFrame{getColor(ColorId::BLUE), 50u},
        });
    }
}

bool applyThemeLengths(Animation * animation, ColorTheme theme)
{
    using Lengths = ShiftingColorAnimation::Lengths;
    auto apply = [](Animation * animation, std::initializer_list<Lengths> lengths)
    {
        static const auto FIRST_PARAM = ShiftingColorAnimation::ParamId::LENGTHS_FIRST;
        return setParameterGroup<Lengths, ShiftingColorAnimation::LengthsParam, FIRST_PARAM>(animation, lengths);
    };

    switch (theme)
    {
        case ColorTheme::CANDY_CANE:
            return apply(animation, {
                {70, 30},
                {30, 30},
            });

        case ColorTheme::BASIC_FOUR_COLORS:
            return apply(animation, {
                {25, 10},
                {25, 10},
                {25, 10},
                {25, 10},
            });

        case ColorTheme::GOLDEN:
            return apply(animation, {
                {70, 30},
                {30, 30},
            });

        case ColorTheme::GREEN_AND_RED:
            return apply(animation, {
                {25, 25},
                {25, 25},
            });

        case ColorTheme::NIGHT_SKY:
            return apply(animation, {
                {25, 10},
                {25, 10},
            });

        case ColorTheme::ICE_AND_MAGENTA:
            return apply(animation, {
                {50, 25},
                {50, 25},
            });

        case ColorTheme::COUNT_:
            break;
    }
    return false;
}

AnimationId makeDefaultSlot(AnimationStorage::Storage * storage, AnimationSlotId slot_id)
{
    switch (static_cast<AnimationSlotName>(slot_id))
    {
    case AnimationSlotName::ANIM_SLOT_COLOR ... AnimationSlotName::ANIM_SLOT_COLOR_LAST:
    default:
        makeAnimation(storage, ANIM_COLOR);
        return ANIM_COLOR;

    case AnimationSlotName::ANIM_SLOT_RAINBOW ... AnimationSlotName::ANIM_SLOT_RAINBOW_LAST:
        makeAnimation(storage, ANIM_RAINBOW);
        return ANIM_RAINBOW;

    case AnimationSlotName::ANIM_SLOT_RETRO ... AnimationSlotName::ANIM_SLOT_RETRO_LAST:
        makeAnimation(storage, ANIM_RETRO);
        {
            const std::uint8_t variant_id = slot_id -
                static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_RETRO);
            (*storage)->setParamater(RetroAnimation::VARIANT, variant_id);
        }
        return ANIM_RETRO;

    case AnimationSlotName::ANIM_SLOT_TWINKLE ... AnimationSlotName::ANIM_SLOT_TWINKLE_LAST:
        makeAnimation(storage, ANIM_TWINKLE);
        {
            const std::uint8_t variant_id = slot_id -
                static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_TWINKLE);
            applyKeyFrames(storage->get(), variant_id);
        }
        return ANIM_TWINKLE;

    case AnimationSlotName::ANIM_SLOT_SHIFTING_COLOR ... AnimationSlotName::ANIM_SLOT_SHIFTING_COLOR_LAST:
        makeAnimation(storage, ANIM_SHIFTING_COLOR);
        {
            const auto theme_id = static_cast<ColorTheme>(slot_id -
                static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_SHIFTING_COLOR));
            applyColorTheme(storage->get(), theme_id);
            applyThemeLengths(storage->get(), theme_id);
        }
        return ANIM_SHIFTING_COLOR;

    case AnimationSlotName::ANIM_SLOT_LIGHTS ... AnimationSlotName::ANIM_SLOT_LIGHTS_LAST:
        makeAnimation(storage, ANIM_LIGHTS);
        {
            const std::uint8_t modifier = slot_id - static_cast<AnimationSlotId>(AnimationSlotName::ANIM_SLOT_LIGHTS);
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
