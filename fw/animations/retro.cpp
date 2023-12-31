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
        {0xC0, 0x5F, 0x00},
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


uint8_t RetroAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
{
    auto s = [=]() -> auto & { return *storage->get<Shared>(); };

    switch (type)
    {
    case Event::START:
        storage->create<Shared>();
        return Result::IS_OK;

    case Event::UPDATE:
        if (0 != s().delay)
        {
            --(s().delay);
            return Result::IGNORE_DEFAULT;
        }
        s().delay =
                static_cast<uint16_t>(render(&(param.ledStrip()), &s())) << 5;
        return Result::IS_OK;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP:
                s().reset();
                if (variant_ != (VARIANT_CNT - 1))
                    ++variant_;
                break;
            case ButtonId::DOWN:
                s().reset();
                if (variant_ != 0)
                    --variant_;
                break;
            default: break;
            }
        }
        return Result::IS_OK;

    case Event::SAVE_CONFIG:
        param.saveConfigurationData().data[0] = static_cast<char>(variant_);
        return Result::IS_OK;

    case Event::LOAD_CONFIG:
        variant_ = static_cast<uint8_t>(param.loadConfigurationData().data[0]);
        return Result::IS_OK;
    }
    return Result::IS_OK;
}

uint8_t RetroAnimation::render(AbstractLedStrip * leds, Shared * shared)
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
            uint8_t color = ((pos & 0x01) << 1) | (shared->state & 0x01);
            if (shared->state & 0x02)
                color ^= 0x02;
            led = ram_colors[color];
            ++pos;
        }
        ++shared->state;
        return (0 == variant_) ? (rand() & 0x03) + 1 : 4;

    case 2:
    case 3:
        pos = shared->state++;
        for (auto & led: *leds)
            led = ram_colors[(pos++) & 0x03];
        return (2 == variant_) ? 4 : 255;
    }

    return 1;
}
