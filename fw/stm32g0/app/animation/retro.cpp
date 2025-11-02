#include "app/animation/retro.hpp"

#include <cstdint>
#include <cstdlib>

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/color_tools.hpp"

namespace
{

inline void copyColors(LedState (&led)[4])
{
    for (std::uint8_t color = 0; color != 4; ++color)
    {
        getColor(&(led[color]), static_cast<ColorId>(static_cast<std::uint8_t>(ColorId::RED) + color));
    }
}

}  // namespace


void RetroAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    if (flags.isFlagSet(RenderFlag::NOTE_CHANGED))
    {
        if (0 == config_.variant)
            delay_ = 0;
        is_playing_ = true;
    }
    if (flags.isFlagSet(RenderFlag::MUSIC_STOPPED))
    {
        if (0 == config_.variant)
            delay_ = 0;
        is_playing_ = false;
    }

    if (0 != delay_)
    {
        --(delay_);
        return;
    }

    LedState ram_colors[4];
    copyColors(ram_colors);

    std::uint16_t delay = 1u;
    uint8_t pos = 0;
    switch (config_.variant)
    {
    case 0:
    case 1:
        for (auto & led: *strip)
        {
            uint8_t color = ((pos & 0x01) << 1) | (state_.state & 0x01);
            if (state_.state & 0x02)
                color ^= 0x02;
            led = ram_colors[color];
            ++pos;
        }
        ++state_.state;
        if (0 == config_.variant)
            delay = is_playing_ ? 255 : (static_cast<uint8_t>(rand()) & static_cast<uint8_t>(0x03)) + 1;
        else
            delay = 4;
        break;

    case 2:
    case 3:
        pos = state_.state++;
        for (auto & led: *strip)
            led = ram_colors[(pos++) & 0x03];
        delay = (2 == config_.variant) ? 4 : 255;
        break;
    }
    delay_ = delay << 5;
}

bool RetroAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case ParamId::VARIANT:
        reset();
        setCyclicParameter<decltype(config_.variant), VARIANT_CNT - 1>(
            &(config_.variant), value, type);
        return true;

    default:
        return false;
    }
}

std::optional<int> RetroAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case ParamId::VARIANT:
        return static_cast<int>(config_.variant);

    default:
        return {};
    }
}

std::size_t RetroAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t RetroAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
