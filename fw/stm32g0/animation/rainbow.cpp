#include "animation/rainbow.hpp"

#include "tools/serdes.hpp"


void RainbowAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    LedState color;
    std::uint16_t hue = state_.hue;
    for (auto & led: *strip)
    {
        toSaturatedHue(hue, &color);
        hue = incrementHue(hue, config_.space_increment);
        led = color;
    }

    ++step_;
    if (2 == step_)
    {
        state_.hue = incrementHue(state_.hue, -config_.time_increment);
        step_ = 0;
    }
    (void)flags;
}

bool RainbowAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    return false;
    (void)param_id; (void)value; (void)type;
}

bool RainbowAnimation::getParameter(std::uint32_t param_id, int * value)
{
    return false;
    (void)param_id; (void)value;
}

std::size_t RainbowAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t RainbowAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer deser(buffer, max_size);
    deser.deserialize(&config_);
    if (type == DataType::BOTH)
        deser.deserialize(&state_);
    return deser.processed(buffer);
}
