/**
 * @file
 */

#ifndef ANIMATIONS_COLOR_H_
#define ANIMATIONS_COLOR_H_

#include "animation.h"

class ColorAnimation: public Animation
{
public:
    ColorAnimation():
        cr_(255), cg_(0), cb_(0),
        redraw_(false)
    {
    }

    bool start(AbstractLedStrip * leds) override;
    bool update(AbstractLedStrip * leds) override;
    void stop(AbstractLedStrip * leds) override;
    bool handleButton(ButtonId button, uint8_t state) override;

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
