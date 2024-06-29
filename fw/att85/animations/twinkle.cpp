#include "animations/twinkle.h"

#include <stdlib.h>

#include <avr/pgmspace.h>

inline void readPgmColor(LedState * led, const LedState & pgm_led)
{
    led->red = pgm_read_byte(&pgm_led.red);
    led->green = pgm_read_byte(&pgm_led.green);
    led->blue = pgm_read_byte(&pgm_led.blue);
}

uint8_t TwinkleAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
{
    auto s = [=]() -> auto & { return *storage->get<Shared>(); };

    switch (type)
    {
    case Event::START:
        storage->create<Shared>();
        return Result::IS_OK;

    case Event::UPDATE:
    {
        ++(s().step);
        if (5 != s().step)
            return Result::IGNORE_DEFAULT;
        s().step = 0;

        paintBackground(&param.ledStrip());
        paintTwinkles(&param.ledStrip(), &(s()));

        return Result::IS_OK;
    }

    case Event::STOP:
        return Result::IS_OK;

    case Event::EVENTS:
        {
            const auto events = param.events();
            if (events.isFlagSet(Animation::Events::SETTINGS_UP))
            {
                if (7 != frequency_)
                    ++frequency_;
            }
            if (events.isFlagSet(Animation::Events::SETTINGS_DOWN))
            {
                if (0 != frequency_)
                    --frequency_;
            }
            if (events.isFlagSet(Animation::Events::NOTE_CHANGED))
                s().beat = true;
        }
        return Result::IS_OK;

    case Event::SAVE_CONFIG:
    {
        auto & data = param.saveConfigurationData();
        data.data[0] = frequency_;
        return Result::IS_OK;

    }

    case Event::LOAD_CONFIG:
    {
        const auto & data = param.loadConfigurationData();
        frequency_ = data.data[0];
        return Result::IS_OK;
    }
    }
    return Result::IS_OK;
}

void TwinkleAnimation::paintBackground(AbstractLedStrip * leds)
{
    LedState color;
    readPgmColor(&color, sequence_[0]);
    for (auto & led: *leds)
        led = color;
}

void TwinkleAnimation::paintTwinkles(AbstractLedStrip * leds, Shared * shared)
{
    const uint16_t mask = shared->processBeat() ? 0x7F : (0x7FFF >> frequency_);

    for (uint8_t pos = 0; pos != Shared::BLINK_CNT; ++pos)
    {
        auto & blink = shared->blinks[pos];
        if (blink.isDone())
        {
            const uint16_t num = (static_cast<uint16_t>(rand()) & mask);
            if (num < leds->led_count)
                blink.start(num);
            else
                continue;
        }
        else
            blink.next();

        LedState color;
        readPgmColor(&color, sequence_[blink.position() + 1]);
        if (isSentinel(color))
            blink.done();
        else
            (*leds)[blink.led()] = color;
    }
}
