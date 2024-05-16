#include "animation_storage.h"

#include <avr/pgmspace.h>

#include "led_strip.h"
#include "animations/rainbow.h"
#include "animations/color.h"
#include "animations/retro.h"
#include "animations/twinkle.h"
#include "animations/shifting_color.h"


static const LedState twinkle_sparks_seq[] PROGMEM =
{
        LedState{0x47300D},
        LedState{0xFFFFFF},
        LedState{0xEDCC9C},
        LedState{0xDEA34A},
        LedState{0xA8711F},
        LedState{0x704B15},
        {0x00, 0x00, 0x00},
};

static const LedState twinkle_shimmer_seq[] PROGMEM =
{
        LedState{0x47300D},

        LedState{0x2F2009},
        LedState{0x1A1105},

        LedState{0x040301},

        LedState{0x1A1105},
        LedState{0x2F2009},

        {0x00, 0x00, 0x00},
};

static const ShiftingColorAnimation::Segment shifting_color_0_seq[] PROGMEM =
{
        {LedState{0xFF, 0x00, 0x00}, 30, 30},
        {LedState{0x55, 0x55, 0x55}, 70, 30},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

static const ShiftingColorAnimation::Segment shifting_color_1_seq[] PROGMEM =
{
        {LedState{0xFF, 0x00, 0x00}, 25, 10},
        {LedState{0x00, 0xFF, 0x00}, 25, 10},
        {LedState{0xC0, 0x5F, 0x00}, 25, 10},
        {LedState{0x00, 0x00, 0xFF}, 25, 10},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

static const ShiftingColorAnimation::Segment shifting_color_2_seq[] PROGMEM =
{
        {LedState{0xFF, 0x8F, 0x00}, 30, 30},
        {LedState{0x3F, 0x1C, 0x00}, 70, 30},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

static const ShiftingColorAnimation::Segment shifting_color_3_seq[] PROGMEM =
{
        {LedState{0xFF, 0x00, 0x00}, 25, 25},
        {LedState{0x00, 0xFF, 0x00}, 25, 25},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

static const ShiftingColorAnimation::Segment shifting_color_4_seq[] PROGMEM =
{
        {LedState{0xC0, 0x5F, 0x00}, 25, 10},
        {LedState{0x00, 0x00, 0xFF}, 25, 10},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

static const ShiftingColorAnimation::Segment * const shifting_color_seqs[] PROGMEM =
{
        shifting_color_0_seq,
        shifting_color_1_seq,
        shifting_color_2_seq,
        shifting_color_3_seq,
        shifting_color_4_seq,
        nullptr
};


template <typename T>
inline void checkType()
{
    static_assert(sizeof(T) <= AnimationStorage::MAX_SIZE, "Check size of the animation object");
    static_assert(alignof(T) <= alignof(AnimationStorage::AlignType), "Check alignment of the animation object");
}


AnimationStorage::AnimationStorage():
    current_id_{0}
{
    checkType<RainbowAnimation>();
    checkType<ColorAnimation>();
    checkType<RetroAnimation>();
    checkType<TwinkleAnimation>();
    checkType<ShiftingColorAnimation>();

    new (storage_) ColorAnimation{};
}

void AnimationStorage::change(uint8_t id)
{
    current()->handleEvent(Animation::Event::DESTROY, Animation::Param{}, nullptr);

    switch (static_cast<AnimationId>(id))
    {
    case AnimationId::ANIM_COUNT_:
    case AnimationId::ANIM_COLOR: new (storage_) ColorAnimation{}; break;
    case AnimationId::ANIM_RAINBOW: new (storage_) RainbowAnimation{}; break;
    case AnimationId::ANIM_RETRO: new (storage_) RetroAnimation{}; break;
    case AnimationId::ANIM_TWINKLE_SPARTKS: new (storage_) TwinkleAnimation{twinkle_sparks_seq}; break;
    case AnimationId::ANIM_TWINKLE_SHIMMER: new (storage_) TwinkleAnimation{twinkle_shimmer_seq}; break;
    case AnimationId::ANIM_SHIFTING_COLOR: new (storage_) ShiftingColorAnimation{shifting_color_seqs}; break;
    }
    if (id >= COUNT)
        new (storage_) ColorAnimation{};
}
