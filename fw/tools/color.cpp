#include "tools/color.h"


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

