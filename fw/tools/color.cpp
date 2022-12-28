#include "tools/color.h"


void blendColors(LedState * color, const LedState & secondary, uint8_t num, uint8_t den)
{
    const uint16_t secondary_blend = (static_cast<uint16_t>(num) * static_cast<uint16_t>(256)) /
            static_cast<uint16_t>(den);
    const uint16_t primary_blend = 256 - secondary_blend;

    uint16_t acc;

    // Red
    acc = color->red * primary_blend;
    acc += secondary.red * secondary_blend;
    color->red = acc >> 8;
    // Green
    acc = color->green * primary_blend;
    acc += secondary.green * secondary_blend;
    color->green = acc >> 8;
    // Blue
    acc = color->blue * primary_blend;
    acc += secondary.blue * secondary_blend;
    color->blue = acc >> 8;
}

void toSaturatedHue(uint16_t hue, LedState * color)
{
    const uint8_t secondary_value = static_cast<uint8_t>(hue & 0xFF);
    const uint8_t primary_value = 0xFF - secondary_value;

    switch (static_cast<uint8_t>(hue >> 8))
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

