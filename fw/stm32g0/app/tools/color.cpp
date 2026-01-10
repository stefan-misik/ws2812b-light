#include "app/tools/color.hpp"


const LedState StandardColors::values[COLOR_COUNT];


void blendColors(LedState * color, const LedState & secondary, std::uint16_t num, std::uint16_t den)
{
    static const std::uint32_t POINT_POS = 16;
    const std::uint32_t secondary_blend = (static_cast<std::uint32_t>(num) << POINT_POS) /
            static_cast<std::uint32_t>(den);
    const std::uint32_t primary_blend = (1u << POINT_POS) - secondary_blend;

    std::uint32_t acc;

    // Red
    acc = color->red * primary_blend;
    acc += secondary.red * secondary_blend;
    color->red = acc >> POINT_POS;
    // Green
    acc = color->green * primary_blend;
    acc += secondary.green * secondary_blend;
    color->green = acc >> POINT_POS;
    // Blue
    acc = color->blue * primary_blend;
    acc += secondary.blue * secondary_blend;
    color->blue = acc >> POINT_POS;
}


void toSaturatedHue(std::uint16_t hue, LedState * color)
{
    const std::uint8_t secondary_value = static_cast<std::uint8_t>(hue & 0xFF);
    const std::uint8_t primary_value = 0xFF - secondary_value;

    switch (static_cast<std::uint8_t>(hue >> 8))
    {
    case 0:
        color->red = primary_value;
        color->green = secondary_value;
        color->blue = 0;
        break;

    case 1:
        color->red = 0;
        color->green = primary_value;
        color->blue = secondary_value;
        break;

    case 2:
        color->red = secondary_value;
        color->green = 0;
        color->blue = primary_value;
        break;

    default:
        color->red = 0;
        color->green = 0;
        color->blue = 0;
        break;
    }
}
