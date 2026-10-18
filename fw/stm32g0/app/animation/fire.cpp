#include "app/animation/fire.hpp"

#include <cstdlib>

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"


namespace
{

/**
 * @brief Map a heat value onto a black -> red -> yellow -> white flame color
 *
 * @param heat Heat value of the LED
 * @param[out] color Resulting flame color
 */
void heatToColor(std::uint8_t heat, LedState * color)
{
    // Split the heat range into thirds and blend towards white as it gets hotter
    const std::uint8_t ramp = static_cast<std::uint8_t>((heat / 3) * 3);

    if (heat > 170)
    {
        // Hottest third: yellow fading up to white
        color->red = 0xFF;
        color->green = 0xFF;
        color->blue = ramp;
    }
    else if (heat > 85)
    {
        // Middle third: red fading up to yellow
        color->red = 0xFF;
        color->green = ramp;
        color->blue = 0x00;
    }
    else
    {
        // Coolest third: black fading up to red
        color->red = ramp;
        color->green = 0x00;
        color->blue = 0x00;
    }
}

}  // namespace


void FireAnimation::step(LedSize led_count)
{
    // Cool down every cell a little
    for (LedSize i = 0; i != led_count; ++i)
    {
        const std::uint8_t cooldown = static_cast<std::uint8_t>(
            (static_cast<std::uint16_t>(std::rand()) % (config_.cooling + 1)) >> 1);
        heat_[i] = (heat_[i] > cooldown) ? static_cast<std::uint8_t>(heat_[i] - cooldown) : 0;
    }

    // Heat rises and diffuses from the base (LED 0) towards the tip
    for (LedSize i = led_count; i-- > 2; )
    {
        heat_[i] = static_cast<std::uint8_t>((static_cast<std::uint16_t>(heat_[i - 1]) +
            static_cast<std::uint16_t>(heat_[i - 2]) + static_cast<std::uint16_t>(heat_[i - 2])) / 3);
    }

    // Randomly ignite a new spark near the base
    const std::uint8_t roll = static_cast<std::uint8_t>(static_cast<std::uint16_t>(std::rand()) % 255);
    if (roll < config_.sparking)
    {
        const LedSize spark_range = (led_count < 3) ? led_count : static_cast<LedSize>(3);
        const LedSize pos = static_cast<LedSize>(static_cast<std::uint16_t>(std::rand()) % spark_range);
        const std::uint16_t new_heat = static_cast<std::uint16_t>(
            heat_[pos] + 160 + (std::rand() % 95));
        heat_[pos] = (new_heat > 255) ? static_cast<std::uint8_t>(255) : static_cast<std::uint8_t>(new_heat);
    }
}

void FireAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    const LedSize led_count = strip->led_count;
    if ((0 == led_count) || (led_count > MAX_STRIP_SIZE))
        return;  // Nothing to render, or the strip does not fit into the simulation buffer

    accumulator_ = static_cast<std::uint16_t>(accumulator_ + config_.speed);
    if (accumulator_ >= UPDATE_THRESHOLD)
    {
        accumulator_ = static_cast<std::uint16_t>(accumulator_ - UPDATE_THRESHOLD);
        step(led_count);
    }

    LedState color;
    for (LedSize i = 0; i != led_count; ++i)
    {
        heatToColor(heat_[i], &color);
        (*strip)[i] = color;
    }

    (void)flags;
}

bool FireAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    case ParamId::COOLING:
        config_.cooling = setLimitParameter<decltype(config_.cooling)>(config_.cooling, value, type);
        return true;

    case ParamId::SPARKING:
        config_.sparking = setLimitParameter<decltype(config_.sparking)>(config_.sparking, value, type);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> FireAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    case ParamId::COOLING:
        return static_cast<int>(config_.cooling);

    case ParamId::SPARKING:
        return static_cast<int>(config_.sparking);

    default:
        return {};
    }
}

std::size_t FireAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t FireAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
