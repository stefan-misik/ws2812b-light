
#ifndef TOOLS_COLOR_H_
#define TOOLS_COLOR_H_

#include <stdint.h>

#include <avr/pgmspace.h>

#include "led_strip.h"

static const uint16_t MAX_HUE = 0x02FF;

enum class ColorId: uint8_t
{
    BLACK = 0,
    WHITE,
    WARM_WHITE,

    RED,
    GREEN,
    YELLOW,
    BLUE,

    TEAL,
    PINK,

    COUNT_
};
static const uint8_t COLOR_COUNT = static_cast<uint8_t>(ColorId::COUNT_);

extern const LedState standard_colors[] PROGMEM;

/**
 * @brief Get the standard color value
 *
 * @param[out] value Variable to obtain color with given ID
 * @param id ID of the color to get
 */
inline void getColor(LedState * value, ColorId id)
{
    const LedState & pgm_color = standard_colors[static_cast<uint8_t>(id)];
    *value = {
        pgm_read_byte(&pgm_color.red),
        pgm_read_byte(&pgm_color.green),
        pgm_read_byte(&pgm_color.blue)
    };
}

/**
 * @brief Blend two colors
 *
 * @param[in,out] color
 * @param[in] secondary
 * @param num Numerator
 * @param den Denominator
 */
void blendColors(LedState * color, const LedState & secondary, uint8_t num, uint8_t den);

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
    else if (new_value > static_cast<int16_t>(MAX_HUE))
        return new_value - (MAX_HUE + 1);
    else
        return new_value;
}



#endif /* TOOLS_COLOR_H_ */
