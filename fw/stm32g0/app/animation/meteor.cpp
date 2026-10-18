#include "app/animation/meteor.hpp"

#include <cstdlib>

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"


namespace
{

/**
 * @brief Draw a color onto a LED, but only if it is brighter than what is already there
 *
 * This lets several overlapping meteor trails blend together sensibly: a bright head
 * from one meteor is never dimmed by a fainter tail segment of another.
 *
 * @param strip Strip to draw onto
 * @param pos LED position to draw
 * @param color Color to draw
 */
void drawIfBrighter(AbstractLedStrip * strip, LedSize pos, const LedState & color)
{
    LedState & led = (*strip)[pos];
    const int new_sum = static_cast<int>(color.red) + color.green + color.blue;
    const int old_sum = static_cast<int>(led.red) + led.green + led.blue;
    if (new_sum > old_sum)
        led = color;
}

}  // namespace


void MeteorAnimation::respawn(Meteor * meteor, LedSize led_count)
{
    meteor->position = 0;
    meteor->hue = static_cast<std::uint16_t>(std::rand() % (MAX_HUE + 1));
    meteor->speed_factor = static_cast<std::uint8_t>(4 + (std::rand() % 12));
    meteor->delay = static_cast<std::uint8_t>(std::rand() % ((led_count / 2) + 1));
}

void MeteorAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    const LedSize led_count = strip->led_count;
    if (0 == led_count)
        return;  // Nothing to render on an empty strip

    static const LedState BLACK{0x00u, 0x00u, 0x00u};

    for (auto & led : *strip)
        led = BLACK;

    const std::uint32_t total = static_cast<std::uint32_t>(led_count) << FRACTION_BITS;
    const LedSize tail_length = (led_count <= TAIL_LENGTH) ?
        static_cast<LedSize>(led_count - 1u) : static_cast<LedSize>(TAIL_LENGTH);

    const std::uint8_t active_count = (config_.count > MAX_METEORS) ? MAX_METEORS : config_.count;

    for (std::uint8_t m = 0; m != active_count; ++m)
    {
        Meteor & meteor = state_.meteors[m];

        if (0 != meteor.delay)
        {
            --(meteor.delay);
            continue;  // Still waiting to (re-)appear
        }

        std::uint32_t step = (static_cast<std::uint32_t>(config_.speed) * meteor.speed_factor) >> 3;
        if (0 == step)
            step = 1;  // Always make forward progress
        meteor.position += step;
        if (meteor.position >= total)
        {
            // Completed a full lap: pick a new look and wait a random delay before restarting
            respawn(&meteor, led_count);
            continue;
        }

        LedState head_color;
        toSaturatedHue(meteor.hue, &head_color);

        LedSize pos = static_cast<LedSize>(meteor.position >> FRACTION_BITS);
        drawIfBrighter(strip, pos, head_color);

        for (LedSize i = 1u; i <= tail_length; ++i)
        {
            pos = strip->prevId(pos);
            LedState color = head_color;
            blendColors(&color, BLACK, i, static_cast<std::uint16_t>(tail_length + 1u));
            drawIfBrighter(strip, pos, color);
        }
    }

    (void)flags;
}

bool MeteorAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    case ParamId::COUNT:
        config_.count = setLimitParameter<decltype(config_.count), MAX_METEORS, 1>(
            config_.count, value, type);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> MeteorAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    case ParamId::COUNT:
        return static_cast<int>(config_.count);

    default:
        return {};
    }
}

std::size_t MeteorAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t MeteorAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
