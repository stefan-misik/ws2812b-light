/**
 * @file
 */

#ifndef ANIMATIONS_COLD_LIGHT_H_
#define ANIMATIONS_COLD_LIGHT_H_

#include "animation.h"
#include "buttons.h"

class LightAnimation: public Animation
{
public:
    /** @copydoc Animation::handleEvent() */
    virtual Result handleEvent(Event event, intptr_t parameter) override;

private:
    enum Type: uint8_t
    {
        COLD = 0,
        WARM,
    };

    uint8_t type_;
    uint8_t intensity_;
    bool redraw_;

    Animation::Result handleKeyPress(Buttons::ButtonId button);

    /**
     * @brief Fill the LED strip with rainbow colors
     * @param led_strip
     */
    void fillLedStrip(AbstractLedStrip * led_strip) const;
};

#endif  // ANIMATIONS_COLD_LIGHT_H_
