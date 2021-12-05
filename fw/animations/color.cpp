#include "animations/color.h"

#include <avr/pgmspace.h>

static const LedState colors[] PROGMEM =
{
        {0x9A, 0x9A, 0x9A},
        {0xFF, 0xa0, 0x30},
        {0xFF, 0x00, 0x00},
        {0x7F, 0x7F, 0x00},
        {0x00, 0xFF, 0x00},
        {0x00, 0x7F, 0x7F},
        {0x00, 0x00, 0xFF},
        {0x7F, 0x00, 0x7F},
};

static constexpr uint8_t COLOR_CNT = sizeof(colors) / sizeof(colors[0]);

bool ColorAnimation::start(AbstractLedStrip * leds)
{
    redraw_ = true;
    return false;
}

bool ColorAnimation::update(AbstractLedStrip * leds)
{
    if (redraw_)
    {
        const LedState * const pgm_color = colors + color_;
        const LedState color{
            pgm_read_byte(&pgm_color->red),
            pgm_read_byte(&pgm_color->green),
            pgm_read_byte(&pgm_color->blue)
        };

        for (auto & led: *leds)
            led = color;

        redraw_ = false;
        return true;
    }
    else
        return false;
}

void ColorAnimation::stop(AbstractLedStrip * leds)
{
}

bool ColorAnimation::handleButton(ButtonId button, uint8_t state)
{
    if (state & ButtonState::PRESS)
    {
        switch (button)
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
    return true;
}
