#include "animations/color.h"

#include <avr/pgmspace.h>

static const LedState colors[] PROGMEM =
{
        {0x9A, 0x9A, 0x9A},
        {0xFF, 0xa0, 0x30},
        {0xFF, 0x00, 0x00},
        {0xA0, 0x7F, 0x00},
        {0x00, 0xFF, 0x00},
        {0x00, 0x7F, 0x7F},
        {0x00, 0x00, 0xFF},
        {0x7F, 0x00, 0x7F},
};

static constexpr uint8_t COLOR_CNT = sizeof(colors) / sizeof(colors[0]);

inline void getColor(LedState * led, uint8_t color)
{
    if (color >= COLOR_CNT)
        color = 0;
    const LedState * const pgm_color = colors + color;
    *led = {
        pgm_read_byte(&pgm_color->red),
        pgm_read_byte(&pgm_color->green),
        pgm_read_byte(&pgm_color->blue)
    };
}

uint8_t ColorAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        redraw_ = true;
        return Result::IS_OK;

    case Event::UPDATE:
        if (redraw_)
        {
            LedState color;
            getColor(&color, color_);

            for (auto & led: *param.ledStrip())
                led = color;

            redraw_ = false;
            return Result::IS_OK;
        }
        else
            return Result::IGNORE_DEFAULT;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                color_ = ((COLOR_CNT - 1) == color_) ? 0 : color_ + 1;
                redraw_ = true;
                break;
            case ButtonId::DOWN:
                color_ = (0 == color_) ? COLOR_CNT - 1 : color_ - 1;
                redraw_ = true;
                break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}
