/**
 * @file
 */

#ifndef ANIMATIONS_NONE_H_
#define ANIMATIONS_NONE_H_

#include "animation.h"

class NoneAnimation: public Animation
{
public:
    /** @copydoc Animation::reset() */
    void reset(AbstractLedStrip * led_strip) override;
    /** @copydoc Animation::step() */
    uint8_t step(AbstractLedStrip * led_strip) override;
};

#endif  // ANIMATIONS_NONE_H_
