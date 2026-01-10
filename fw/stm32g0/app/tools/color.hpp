/**
 * @file
 */

#ifndef APP_TOOLS_COLOR_HPP_
#define APP_TOOLS_COLOR_HPP_

#include "led_strip.hpp"

static const inline std::uint16_t MAX_HUE = 0x02FF;

enum class ColorId: std::uint8_t
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
static const inline std::uint8_t COLOR_COUNT = static_cast<std::uint8_t>(ColorId::COUNT_);

/** @brief Color values storage */
struct StandardColors
{
    static constexpr const LedState values[COLOR_COUNT] =
    {
        {0x00, 0x00, 0x00},
        {0x9A, 0x9A, 0x9A},
        {0xFF, 0xa0, 0x30},

        {0xFF, 0x00, 0x00},
        {0x00, 0xFF, 0x00},
        {0xC0, 0x5F, 0x00},
        {0x00, 0x00, 0xFF},

        {0x00, 0x7F, 0x7F},
        {0x7F, 0x00, 0x7F},
    };
};


/**
 * @brief Get the standard color value
 *
 * @param id ID of the color to get

 * @return Color with given ID
 */
inline constexpr const LedState & getColor(ColorId id)
{
    return StandardColors::values[static_cast<std::size_t>(id)];
}

/**
 * @brief Get the standard color value
 *
 * @param[out] value Variable to obtain color with given ID
 * @param id ID of the color to get
 */
inline void getColor(LedState * value, ColorId id)
{
    *value = StandardColors::values[static_cast<std::size_t>(id)];
}

/**
 * @brief Blend two colors
 *
 * @param[in,out] color
 * @param[in] secondary
 * @param num Numerator
 * @param den Denominator
 */
void blendColors(LedState * color, const LedState & secondary, std::uint16_t num, std::uint16_t den);

/**
 * @brief Generate color with given hue value
 *
 * @param hue
 * @param[out] color Generated color
 */
void toSaturatedHue(std::uint16_t hue, LedState * color);


inline std::uint16_t incrementHue(std::uint16_t hue, std::int8_t value = 1)
{
    std::int16_t new_value = value + static_cast<std::int16_t>(hue);
    if (new_value < 0)
        return (MAX_HUE + 1) + new_value;
    else if (new_value > static_cast<std::int16_t>(MAX_HUE))
        return new_value - (MAX_HUE + 1);
    else
        return new_value;
}

#endif  // APP_TOOLS_COLOR_HPP_
