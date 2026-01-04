/**
 * @file
 */

#ifndef APP_ANIMATION_SHIFTING_COLOR_HPP_
#define APP_ANIMATION_SHIFTING_COLOR_HPP_

#include <array>

#include "app/animation.hpp"


class ShiftingColorAnimation final:
        public Animation
{
private:
    static const inline std::size_t COLOR_THEME_MAX_LENGTH = 8;

public:
    static const inline std::size_t FRACTION_BITS = 8;

    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
        LENGTHS_FIRST,
        LENGTHS_LAST = LENGTHS_FIRST + (COLOR_THEME_MAX_LENGTH - 1),
    };

    struct Lengths
    {
        std::uint8_t length;
        std::uint8_t transition;
    };

    struct Segment
    {
        LedState color;
        Lengths length;
    };

    /** @brief Helper functions for changing lengths parameters */
    struct LengthsParam
    {
        static constexpr int make(const Lengths & Lengths)
        {
            return static_cast<int>((static_cast<std::uint32_t>(Lengths.transition) << 8) |
                (static_cast<std::uint32_t>(Lengths.length) << 0));
        }

        static constexpr Lengths parse(int value)
        {
            return Lengths(static_cast<std::uint32_t>(value) >> 0, static_cast<std::uint32_t>(value) >> 8);
        }
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    struct Configuration
    {
        std::uint8_t theme_length;
        std::array<Segment, COLOR_THEME_MAX_LENGTH> theme;
        std::uint8_t speed = 3;
    };

    struct State
    {
        std::uint32_t offset = 0;
    };

    Configuration config_;
    State state_;
};



#endif  // APP_ANIMATION_SHIFTING_COLOR_HPP_
