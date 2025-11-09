#include "app/animation/twinkle.hpp"

#include <cstdlib>

#include "led_strip.h"
#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/color_tools.hpp"


namespace
{

constexpr const inline LedState SENTINEL = {0u, 0u, 0u};

// 'constexpr' in following array definitions ensures that array is placed in
// FLASH, i.e. constexpr LedState constructors are used
constexpr const LedState sequence_sparks[] = {
    LedState{0x47300D},
    LedState{0xFFFFFF},
    LedState{0xEDCC9C},
    LedState{0xDEA34A},
    LedState{0xA8711F},
    LedState{0x704B15},
    SENTINEL
};

constexpr const LedState sequence_shimmer[] = {
    LedState{0x47300D},

    LedState{0x2F2009},
    LedState{0x1A1105},

    LedState{0x040301},

    LedState{0x1A1105},
    LedState{0x2F2009},

    SENTINEL
};

const LedState * all_sequences[] = {
    sequence_sparks,
    sequence_shimmer
};

static_assert((sizeof(all_sequences) / sizeof(all_sequences[0])) == TwinkleAnimation::VARIANT_CNT,
    "Check variant count");

}  // namespace


void TwinkleAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    ++(step_);
    if (5 != step_)
        return;
    step_ = 0;

    const LedState * sequence = all_sequences[config_.variant];

    // Paint background
    {
        const LedState & bg_color = sequence[0];
        for (auto & led: *strip)
            led = bg_color;
    }

    const std::uint16_t mask = flags.isFlagSet(RenderFlag::NOTE_CHANGED) ? 0x7F : (0x7FFF >> config_.frequency);

    for (uint8_t pos = 0; pos != State::BLINK_CNT; ++pos)
    {
        auto & blink = state_.blinks[pos];
        if (blink.isDone())
        {
            const uint16_t num = (static_cast<uint16_t>(std::rand()) & mask);
            if (num < strip->led_count)
                blink.start(num);
            else
                continue;
        }
        else
            blink.next();

        const LedState color = sequence[blink.position() + 1];
        if (color == SENTINEL)
            blink.done();
        else
            (*strip)[blink.led()] = color;
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

    case ParamId::VARIANT:
        config_.variant = setCyclicParameter<decltype(config_.variant), VARIANT_CNT - 1>(
            config_.variant, value, type);
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

    case ParamId::VARIANT:
        return static_cast<int>(config_.variant);

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
