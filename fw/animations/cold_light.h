/**
 * @file
 */

#ifndef ANIMATIONS_COLD_LIGHT_H_
#define ANIMATIONS_COLD_LIGHT_H_

#include "animation.h"

class ColdLightAnimation: public Animation
{
public:
    ColdLightAnimation():
        intensity_(0xff),
        redraw_(false)
    { }

    /** @copydoc Animation::handleEvent() */
    virtual Result handleEvent(Event event, intptr_t parameter) override;

private:
    uint8_t intensity_;
    bool redraw_;

    /**
     * @brief Fill the LED strip with rainbow colors
     * @param led_strip
     */
    void fillLedStrip(AbstractLedStrip * led_strip) const;
};

#endif  // ANIMATIONS_COLD_LIGHT_H_
