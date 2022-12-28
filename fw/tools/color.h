
#ifndef TOOLS_COLOR_H_
#define TOOLS_COLOR_H_

#include <stdint.h>

#include "led_strip.h"

static const uint16_t MAX_HUE = 0x02FF;

/**
 * @brief Blend two colors
 *
 * @param[in,out] color
 * @param[in] secondary
 * @param blend Amount of the secondary color to blend
 */
void blendColors(LedState * color, const LedState & secondary, uint8_t blend);

/**
 * @brief Generate color with given hue value
 *
 * @param hue
 * @param[out] color Generated color
 */
void toSaturatedHue(uint16_t hue, LedState * color);

inline uint16_t incrementHue(uint16_t hue, int8_t value = 1)
{
    int16_t new_value = value + static_cast<int16_t>(hue);
    if (new_value < 0)
        return (MAX_HUE + 1) + new_value;
    else if (new_value > MAX_HUE)
        return new_value - (MAX_HUE + 1);
    else
        return new_value;
}



#endif /* TOOLS_COLOR_H_ */
