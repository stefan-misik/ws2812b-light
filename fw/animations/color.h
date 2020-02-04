/**
 * @file
 */

#ifndef ANIMATIONS_COLOR_H_
#define ANIMATIONS_COLOR_H_

#include "animation.h"

class ColorAnimation: public Animation
{
public:
    /** @copydoc Animation::handleEvent() */
    virtual Result handleEvent(Event event, intptr_t parameter) override;

private:
    uint8_t cr_;
    uint8_t cg_;
    uint8_t cb_;
    bool redraw_;

    /**
     * @brief Calculate next rainbow color
     */
    void stepRainbowColor();

    /**
     * @brief Fill the LED strip with rainbow colors
     * @param led_strip
     */
    void fillLedStrip(AbstractLedStrip * led_strip) const;
};

#endif  // ANIMATIONS_COLOR_H_
