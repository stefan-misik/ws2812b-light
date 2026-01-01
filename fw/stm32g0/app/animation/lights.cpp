#include "app/animation/lights.hpp"

#include <cstdlib>
#include <utility>

#include "tools/serdes.hpp"
#include "tools/bit_array.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"

namespace
{

using LedFlags = LightsAnimation::LedFlags;

constexpr const inline LedState SENTINEL = {0u, 0u, 0u};

// 'constexpr' in following array definitions ensures that array is placed in
// FLASH, i.e. constexpr LedState constructors are used
constexpr const LedState variant_0[] = {
    LedState{0x55, 0x55, 0x55},
    LedState{0xFF, 0x00, 0x00},
    SENTINEL
};
constexpr const LedState variant_1[] = {
    LedState{0xFF, 0x00, 0x00},
    LedState{0x00, 0xFF, 0x00},
    LedState{0xC0, 0x5F, 0x00},
    LedState{0x00, 0x00, 0xFF},
    SENTINEL
};
constexpr const LedState variant_2[] = {
    LedState{0x3F, 0x1C, 0x00},
    LedState{0xFF, 0x8F, 0x00},
    SENTINEL
};
constexpr const LedState variant_3[] = {
    LedState{0x00, 0xFF, 0x00},
    LedState{0xFF, 0x00, 0x00},
    SENTINEL
};
constexpr const LedState variant_4[] = {
    LedState{0x00, 0x00, 0xFF},
    LedState{0xC0, 0x5F, 0x00},
    SENTINEL
};
constexpr const LedState variant_5[] = {
    LedState{0x02, 0xF5, 0xC4},
    LedState{0xE9, 0x45, 0xCB},
    SENTINEL
};

constexpr const LedState * const variants[LightsAnimation::VARIANT_CNT] = {
    variant_0, variant_1, variant_2, variant_3, variant_4, variant_5
};

const std::uint16_t FULL_STATE = 0xFFFFul;

inline std::size_t variantLength(const LedState * sequence)
{
    const auto * pos = sequence;
    while (SENTINEL != *pos)
        ++pos;
    return pos - sequence;
}

template <typename... Ts>
inline std::size_t findResetBit(const LedFlags * begin, const LedFlags * const end, std::size_t reset_bit_count,
    Ts &&... args)
{
    const auto * it = begin;
    for (; it != end; ++it)
    {
        if (it->isDone(std::forward<Ts>(args)...))
        {
            if (0u == reset_bit_count)
                break;
            --reset_bit_count;
        }
    }
    return it - begin;
}

template <typename... Ts>
inline std::size_t countAvailableLeds(const LedFlags * begin, const LedFlags * const end, Ts &&... args)
{
    std::size_t count = 0;
    const auto * it = begin;
    for (; it != end; ++it)
    {
        if (it->isDone(std::forward<Ts>(args)...))
            count += 1;
    }
    return count;
}

}  // namespace


void LightsAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    if (strip->led_count > MAX_STRIP_SIZE)
        return;  // Can not render this

    const LedState * const variant = variants[config_.variant];
    const std::size_t variant_length = variantLength(variant);
    // Render individual lights
    {
        auto flags = flags_.begin();
        for (auto & led: *strip)
        {
            const auto pos = flags->position();
            led = variant[pos];
            ++flags;
        }
    }

    // Handle transitions and locate unused transition state
    Transition * free_transition = nullptr;
    bool is_transitioning = false;
    for (auto & transition : transitions_)
    {
        if (!transition.isValid())
        {
            free_transition = &transition;
            continue;
        }
        LedState & led = strip->leds[transition.ledId()];
        LedFlags & flags = flags_[transition.ledId()];

        // Blend the colors
        const auto next_pos_value = flags.nextValue(variant_length);
        const LedState & next_color = variant[next_pos_value];
        transition.step(static_cast<std::uint16_t>(config_.speed) << 8, FULL_STATE);
        blendColors(&led, next_color, transition.state(), FULL_STATE);

        // Check whether we are done
        if (transition.state() == FULL_STATE)
        {
            flags.reset(next_pos_value);
            transition.reset();
            free_transition = &transition;
        }
        else
        {
            is_transitioning = true;
        }
    }

    // Add new LED in case there are available LEDs and transitions
    const auto * const flags_end = flags_.cbegin() + strip->led_count;
    const std::size_t available_led_cnt = config_.synchronized ?
        countAvailableLeds(flags_.cbegin(), flags_end, state_.synchronized_pos) :
        countAvailableLeds(flags_.cbegin(), flags_end);
    if (0 != available_led_cnt && nullptr != free_transition)
    {
        const std::size_t led_offset = static_cast<std::size_t>(std::rand()) % (available_led_cnt << 4);
        // Only add new led with some probability
        if (led_offset < available_led_cnt)
        {
            const LedSize new_led_pos = config_.synchronized ?
                findResetBit(flags_.cbegin(), flags_end, led_offset, state_.synchronized_pos) :
                findResetBit(flags_.cbegin(), flags_end, led_offset);
            if (new_led_pos < strip->led_count)
            {
                flags_[new_led_pos].startTransitioning();
                free_transition->start(new_led_pos);
            }
        }
    }

    // Advance synchronized position if we are in synchronized mode
    if (config_.synchronized && 0 == available_led_cnt && !is_transitioning)
    {
        // This code might be called multiple times when switching from not synchronized to synchronized, when LEDs are
        // in random states. But it will stabilize and once it has, this works fine, as by switching to next position
        // here, all LEDs suddenly become available, and this code is not executed until next transition.
        state_.synchronized_pos = LedFlags::next(state_.synchronized_pos, variant_length);
    }
    (void)flags;
}

bool LightsAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    case ParamId::VARIANT:
        config_.variant = setCyclicParameter<decltype(config_.variant), VARIANT_CNT - 1>(
            config_.variant, value, type);
        reset();
        return true;

    case ParamId::SYNCHRONIZED:
        config_.synchronized = setCyclicParameter<unsigned, 1>(
            config_.synchronized, value, type);
        state_.synchronized_pos = 0ul;
        return true;

    default:
        return false;
    }
    return false;
    (void)value; (void)type;
}

std::optional<int> LightsAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    case ParamId::VARIANT:
        return static_cast<int>(config_.variant);

    case ParamId::SYNCHRONIZED:
        return static_cast<int>(config_.synchronized);

    default:
        return {};
    }
}

std::size_t LightsAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t LightsAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
