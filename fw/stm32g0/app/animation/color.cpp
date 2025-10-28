#include "app/animation/color.hpp"

#include "tools/serdes.hpp"
#include "app/color_tools.hpp"

namespace
{

const inline std::size_t MY_COLOR_COUNT = COLOR_COUNT - ColorAnimation::FIRST_COLOR;

}  // namespace


void ColorAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    LedState color;
    getColor(&color, static_cast<ColorId>(FIRST_COLOR + config_.color));
    for (auto & led: *strip)
        led = color;

    (void)flags;
}

bool ColorAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case ParamId::SECONDARY:
        if (ChangeType::ABSOLUTE == type)
            config_.color = value >= static_cast<int>(MY_COLOR_COUNT) ? 0 : value;
        else
        {
            int color_id = config_.color;
            color_id += value;
            if (color_id >= static_cast<int>(MY_COLOR_COUNT))
                color_id = 0;
            else if (color_id < 0)
                color_id = MY_COLOR_COUNT - 1;
            config_.color = color_id;
        }
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> ColorAnimation::getParameter(std::uint32_t param_id)
{
    return {};
    (void)param_id;
}

std::size_t ColorAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t ColorAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer deser(buffer, max_size);
    deser.deserialize(&config_);
    if (type == DataType::BOTH)
        deser.deserialize(&state_);
    return deser.processed(buffer);
}
