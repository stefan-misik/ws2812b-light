/**
 * @file
 */

#ifndef ANIMATIONS_RAINBOW_H_
#define ANIMATIONS_RAINBOW_H_

#include "animation.h"

class RainbowAnimation: public Animation
{
public:
    bool start(AbstractLedStrip * leds) override;
    bool update(AbstractLedStrip * leds) override;
    void stop(AbstractLedStrip * leds) override;
    bool handleButton(Buttons::ButtonId button, uint8_t state) override;

private:
    uint8_t ca_;
    uint8_t cb_;
    uint8_t cc_;

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

#endif  // ANIMATIONS_RAINBOW_H_
