/**
 * @file
 */

#ifndef ANIMATIONS_SPARKS_H_
#define ANIMATIONS_SPARKS_H_

#include "animation.h"

class SparksAnimation: public Animation
{
public:
    SparksAnimation():
        frequency_(3)
    { }

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    uint8_t frequency_;

    struct Shared
    {
        uint8_t step = 0;
    };
};

#endif  // ANIMATIONS_SPARKS_H_
