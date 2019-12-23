/**
 * @file
 */

#ifndef ANIMATIONS_RAINBOW_H_
#define ANIMATIONS_RAINBOW_H_

#include "animation.h"

#include <stdint.h>

class RainbowAnimation: public Animation
{
public:
    /** @copydoc Animation::reset() */
    void reset(AbstractLedStrip * led_strip) override;
    /** @copydoc Animation::step() */
    uint8_t step(AbstractLedStrip * led_strip) override;

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
