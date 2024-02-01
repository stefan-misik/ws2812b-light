#include "animations/color.h"

#include "tools/color.h"


inline void getColorByNumber(LedState * led, uint8_t color)
{
    if (color >= COLOR_COUNT)
        color = 0;
    getColor(led, static_cast<ColorId>(color));
}

uint8_t ColorAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
{
    auto s = [=]() -> auto & { return *storage->get<Shared>(); };

    switch (type)
    {
    case Event::START:
        storage->create<Shared>();
        s().redraw = true;
        return Result::IS_OK;

    case Event::UPDATE:
        if (s().redraw)
        {
            LedState color;
            getColorByNumber(&color, color_);

            for (auto & led: param.ledStrip())
                led = color;

            s().redraw = false;
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
                color_ = ((COLOR_COUNT - 1) == color_) ? 0 : color_ + 1;
                s().redraw = true;
                break;
            case ButtonId::DOWN:
                color_ = (0 == color_) ? COLOR_COUNT - 1 : color_ - 1;
                s().redraw = true;
                break;
            default: break;
            }
        }
        return Result::IS_OK;

    case Event::SAVE_CONFIG:
        param.saveConfigurationData().data[0] = static_cast<char>(color_);
        return Result::IS_OK;

    case Event::LOAD_CONFIG:
        color_ = static_cast<uint8_t>(param.loadConfigurationData().data[0]);
        return Result::IS_OK;
    }
    return Result::IS_OK;
}
