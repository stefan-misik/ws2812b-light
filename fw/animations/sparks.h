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
        step_(0),
        frequency_(3)
    {
    }

    uint8_t handleEvent(Event type, Param param) override;

private:
    uint8_t step_;
    uint8_t frequency_;
};

#endif  // ANIMATIONS_SPARKS_H_
