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

static const ShiftingColorAnimation::Segment shifting_color_seq[] PROGMEM =
{
        {LedState{0xFF, 0x00, 0x00}, 50},
        {LedState{0xFF, 0xFF, 0xFF}, 50},
        {LedState{0x00, 0x00, 0x00}, 0},
};

AnimationList::AnimationList():
        current_{&color_},
        current_id_{0},
        twinkle_sparks_{twinkle_sparks_seq},
        twinkle_shimmer_{twinkle_shimmer_seq},
        shifting_color_{shifting_color_seq}
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
