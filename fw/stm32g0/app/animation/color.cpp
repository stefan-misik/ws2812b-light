#include "app/animation/color.hpp"

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"

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
    case Animation::ParamId::SECONDARY:
    case ParamId::COLOR_ID:
        config_.color = setCyclicParameter<decltype(config_.color), MY_COLOR_COUNT - 1>(
            config_.color, value, type);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> ColorAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::COLOR_ID:
        return static_cast<int>(config_.color);

    default:
        return {};
    }
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
