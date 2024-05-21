#include "animations/retro.h"

#include <stdlib.h>

#include "tools/color.h"

inline void copyColors(LedState (&led)[4])
{
    for (uint8_t color = 0; color != 4; ++color)
    {
        getColor(&(led[color]), static_cast<ColorId>(static_cast<uint8_t>(ColorId::RED) + color));
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

    case Event::EVENTS:
        {
            const auto events = param.events();
            if (events.isFlagSet(Animation::Events::SETTINGS_UP))
            {
                s().reset();
                if (variant_ != (VARIANT_CNT - 1))
                    ++variant_;
            }
            if (events.isFlagSet(Animation::Events::SETTINGS_DOWN))
            {
                s().reset();
                if (variant_ != 0)
                    --variant_;
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
    LedState ram_colors[4];
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
