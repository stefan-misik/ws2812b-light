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
        frequency_(8)
    {
    }

    uint8_t handleEvent(Event type, Param param) override;

private:
    static constexpr uint8_t STEPS = 10;

    uint8_t step_;
    int frequency_;
};

#endif  // ANIMATIONS_SPARKS_H_
