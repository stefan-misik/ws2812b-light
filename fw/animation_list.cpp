#include "animation_list.h"

#include <avr/pgmspace.h>

#include "led_strip.h"


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

AnimationList::AnimationList():
        current_{&color_},
        current_id_{0},
        twinkle_sparks_{twinkle_sparks_seq},
        twinkle_shimmer_{twinkle_shimmer_seq},
        shifting_color_{shifting_color_seqs}
{
}

Animation * AnimationList::getById(uint8_t id)
{
    switch (id)
    {
    default:
    case 0: return &color_;
    case 1: return &rainbow_;
    case 2: return &retro_;
    case 3: return &twinkle_sparks_;
    case 4: return &twinkle_shimmer_;
    case 5: return &shifting_color_;
    }
}
