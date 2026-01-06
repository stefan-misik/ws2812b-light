#include "app/animation/twinkle.hpp"

#include <cstdlib>

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"


namespace
{

using KeyFrame = TwinkleAnimation::KeyFrame;


bool getColor(LedState * color, std::span<const KeyFrame> key_frames, std::uint32_t pos)
{
    std::uint32_t remaining = pos;
    const auto * prev_color = &(key_frames.back().color);
    for (const auto & key_frame : key_frames)
    {
        if (remaining <= key_frame.length)
        {
            // Blend the previous color with current color
            *color = *prev_color;
            blendColors(color, key_frame.color, remaining, key_frame.length);
            return true;
        }
        else
        {
            remaining -= key_frame.length;
            prev_color = &(key_frame.color);
        }
    }
    return false;
}


}  // namespace


void TwinkleAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    ++(step_);
    if (5 == step_)
        step_ = 0;
    // This is to limit the number of new blinks
    const bool allow_new = (0 == step_);

    // Paint background
    {
        const LedState & bg_color = config_.key_frames[0].color;
        for (auto & led: *strip)
            led = bg_color;
    }

    const std::span<const KeyFrame> key_frames{config_.key_frames.begin(), config_.key_frame_count};
    const std::uint16_t mask = flags.isFlagSet(RenderFlag::NOTE_CHANGED) ? 0x7F : (0x7FFF >> config_.frequency);

    for (uint8_t pos = 0; pos != State::BLINK_CNT; ++pos)
    {
        auto & blink = state_.blinks[pos];
        if (blink.isDone())
        {
            if (!allow_new)
                continue;
            const uint16_t num = (static_cast<uint16_t>(std::rand()) & mask);
            if (num < strip->led_count)
                blink.start(num);
            else
                continue;
        }
        else
            blink.next();

        if (!getColor(strip->begin() + blink.led(), key_frames, blink.position() + 1u))
            blink.done();
    }
}

bool TwinkleAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::FREQUENCY:
        config_.frequency = setLimitParameter<decltype(config_.frequency), 7>(
            config_.frequency, value, type);
        return true;

    case ParamId::KEY_FRAME_COUNT:
        if (ChangeType::ABSOLUTE != type)
            return false;
        if (static_cast<std::size_t>(value) > MAX_KEY_FRAME_COUNT)
            return false;
        config_.key_frame_count = static_cast<std::uint8_t>(value);
        return true;

    case ParamId::KEY_FRAME_FIRST...ParamId::KEY_FRAME_LAST:
        if (ChangeType::ABSOLUTE != type)
            return false;
        config_.key_frames[param_id - ParamId::KEY_FRAME_FIRST] = KeyFrameParam::parse(value);
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> TwinkleAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::FREQUENCY:
        return static_cast<int>(config_.frequency);

    case ParamId::KEY_FRAME_COUNT:
        return static_cast<int>(config_.key_frame_count);

    case ParamId::KEY_FRAME_FIRST...ParamId::KEY_FRAME_LAST:
        return KeyFrameParam::make(config_.key_frames[param_id - ParamId::KEY_FRAME_FIRST]);

    default:
        return {};
    }
}

std::size_t TwinkleAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t TwinkleAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
