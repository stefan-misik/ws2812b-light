#include "app/animation/bounce.hpp"

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"


void BounceAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    const LedSize led_count = strip->led_count;
    if (0u == led_count)
        return;  // Nothing to render on an empty strip

    static const LedState BLACK{0x00u, 0x00u, 0x00u};

    for (auto & led : *strip)
        led = BLACK;

    LedState head_color;
    toSaturatedHue(config_.hue, &head_color);

    if (1u == led_count)
    {
        // A single LED has nowhere to bounce to, just show the head color
        (*strip)[0] = head_color;
        (void)flags;
        return;
    }

    const std::int32_t total = static_cast<std::int32_t>(led_count - 1u) << FRACTION_BITS;
    std::int32_t delta = static_cast<std::int32_t>(static_cast<std::uint32_t>(config_.speed) << 4);
    if (0u == state_.direction)
        delta = -delta;

    std::int32_t new_position = static_cast<std::int32_t>(state_.position) + delta;
    if (new_position > total)
    {
        new_position = (2 * total) - new_position;
        state_.direction = 0u;
    }
    else if (new_position < 0)
    {
        new_position = -new_position;
        state_.direction = 1u;
    }
    // Guard against pathological speed/length combinations still falling outside range
    if (new_position < 0)
        new_position = 0;
    else if (new_position > total)
        new_position = total;
    state_.position = static_cast<std::uint32_t>(new_position);

    const LedSize head = static_cast<LedSize>(state_.position >> FRACTION_BITS);
    (*strip)[head] = head_color;

    for (LedSize i = 1u; i <= TAIL_LENGTH; ++i)
    {
        LedState color = head_color;
        blendColors(&color, BLACK, i, static_cast<std::uint16_t>(TAIL_LENGTH + 1u));

        if (head >= i)
            (*strip)[static_cast<LedSize>(head - i)] = color;

        const LedSize forward = static_cast<LedSize>(head + i);
        if (forward < led_count)
            (*strip)[forward] = color;
    }

    (void)flags;
}

bool BounceAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    case ParamId::HUE:
        config_.hue = setCyclicParameter<decltype(config_.hue), MAX_HUE, 0>(
            config_.hue, value, type);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> BounceAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    case ParamId::HUE:
        return static_cast<int>(config_.hue);

    default:
        return {};
    }
}

std::size_t BounceAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t BounceAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
