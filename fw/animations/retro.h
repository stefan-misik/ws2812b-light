/**
 * @file
 */

#ifndef ANIMATIONS_RETRO_H_
#define ANIMATIONS_RETRO_H_

#include "animation.h"

class RetroAnimation: public Animation
{
public:
    RetroAnimation():
        variant_(0)
    { }

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    static const uint8_t VARIANT_CNT = 4;

    uint8_t variant_: 4;

    struct Shared
    {
        uint16_t delay = 0;
        uint8_t state = 0;

        void reset()
        {
            delay = 0;
            state = 0;
        }
    };

    uint8_t render(AbstractLedStrip * leds, Shared * shared);

};

#endif  // ANIMATIONS_RETRO_H_
