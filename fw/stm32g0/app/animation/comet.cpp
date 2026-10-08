#include "app/animation/comet.hpp"

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"


void CometAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    const LedSize led_count = strip->led_count;
    if (0u == led_count)
        return;  // Nothing to render on an empty strip

    static const LedState BLACK{0x00u, 0x00u, 0x00u};
    static const LedState HEAD_COLOR{0xFFu, 0xFFu, 0xFFu};

    for (auto & led : *strip)
        led = BLACK;

    const std::uint32_t total = static_cast<std::uint32_t>(led_count) << FRACTION_BITS;
    state_.position += (static_cast<std::uint32_t>(config_.speed) << 4);
    if (state_.position >= total)
        state_.position %= total;

    const LedSize head = static_cast<LedSize>(state_.position >> FRACTION_BITS);
    const LedSize tail_length = (led_count <= TAIL_LENGTH) ?
        static_cast<LedSize>(led_count - 1u) : static_cast<LedSize>(TAIL_LENGTH);

    LedSize pos = head;
    (*strip)[pos] = HEAD_COLOR;

    for (LedSize i = 1u; i <= tail_length; ++i)
    {
        pos = strip->prevId(pos);
        LedState color = HEAD_COLOR;
        blendColors(&color, BLACK, i, static_cast<std::uint16_t>(tail_length + 1u));
        (*strip)[pos] = color;
    }

    (void)flags;
}

bool CometAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> CometAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    default:
        return {};
    }
}

std::size_t CometAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t CometAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
