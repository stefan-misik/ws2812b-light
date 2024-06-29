#include "animations/rainbow.h"

#include "tools/color.h"

uint8_t RainbowAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
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
        if (2 != s().step)
            return Result::IGNORE_DEFAULT;
        s().step = 0;

        uint16_t hue = s().hue;
        LedState color;
        for (auto & led: param.ledStrip())
        {
            toSaturatedHue(hue, &color);
            hue = incrementHue(hue, space_increment_);
            led = color;
        }

        s().hue = incrementHue(s().hue, -time_increment_);

        return Result::IS_OK;
    }

    case Event::STOP:
        return Result::IS_OK;

    case Event::EVENTS:
        {
            const auto events = param.events();
            if (events.isFlagSet(Animation::Events::SETTINGS_UP))
                ++space_increment_;
            if (events.isFlagSet(Animation::Events::SETTINGS_DOWN))
                ++time_increment_;
        }
        return Result::IS_OK;

    case Event::SAVE_CONFIG:
    {
        auto & data = param.saveConfigurationData();
        data.data[0] = space_increment_;
        data.data[1] = time_increment_;
        return Result::IS_OK;

    }

    case Event::LOAD_CONFIG:
    {
        const auto & data = param.loadConfigurationData();
        space_increment_ = data.data[0];
        time_increment_ = data.data[1];
        return Result::IS_OK;
    }
    }
    return Result::IS_OK;
}
