/**
 * @file
 */

#ifndef ANIMATIONS_DRIP_H_
#define ANIMATIONS_DRIP_H_

#include "animation.h"

#include <stdint.h>

class DripAnimation: public Animation
{
public:
    /** @copydoc Animation::handleEvent() */
    virtual Result handleEvent(Event event, intptr_t parameter) override;

private:
    size_t position_;
    uint8_t sub_;

    void fillLedStrip(AbstractLedStrip * led_strip) const;
};

#endif  // ANIMATIONS_RAINBOW_H_
