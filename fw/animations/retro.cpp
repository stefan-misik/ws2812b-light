#include "animations/retro.h"

#include <stdlib.h>

#include <avr/pgmspace.h>

enum Color
{
    C_RED = 0,
    C_GREEN,
    C_YELLOW,
    C_BLUE,

    C_CNT_
};

static const LedState colors[C_CNT_] PROGMEM =
{
        {0xFF, 0x00, 0x00},
        {0x00, 0xFF, 0x00},
        {0xA0, 0x7F, 0x00},
        {0x00, 0x00, 0xFF},
};

inline void copyColors(LedState (&led)[C_CNT_])
{
    for (uint8_t color = 0; color != C_CNT_; ++color)
    {
        const LedState * const pgm_color = colors + color;
        led[color] = {
            pgm_read_byte(&pgm_color->red),
            pgm_read_byte(&pgm_color->green),
            pgm_read_byte(&pgm_color->blue)
        };
    }
}


uint8_t RetroAnimation::handleEvent(Event type, Param param)
{
    switch (type)
    {
    case Event::START:
        reset();
        return Result::IS_OK;

    case Event::UPDATE:
        if (0 != delay_)
        {
            --delay_;
            return Result::IGNORE_DEFAULT;
        }
        delay_ = static_cast<uint16_t>(render(param.ledStrip())) << 5;
        return Result::IS_OK;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                reset();
                if (variant_ != (VARIANT_CNT - 1))
                    ++variant_;
                break;
            case ButtonId::DOWN:
                reset();
                if (variant_ != 0)
                    --variant_;
                break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}

uint8_t RetroAnimation::render(AbstractLedStrip * leds)
{
    LedState ram_colors[C_CNT_];
    copyColors(ram_colors);

    uint8_t pos = 0;

    switch (variant_)
    {
    case 0:
    case 1:
        for (auto & led: *leds)
        {
            uint8_t color = ((pos & 0x01) << 1) | (state_ & 0x01);
            if (state_ & 0x02)
                color ^= 0x02;
            led = ram_colors[color];
            ++pos;
        }
        ++state_;
        return (0 == variant_) ? (rand() & 0x03) + 1 : 4;

    case 2:
    case 3:
        pos = state_++;
        for (auto & led: *leds)
            led = ram_colors[(pos++) & 0x03];
        return (2 == variant_) ? 4 : 255;
    }

    return 1;
}
