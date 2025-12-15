/**
 * @file
 */

#ifndef APP_ANIMATION_SHIFTING_COLOR_HPP_
#define APP_ANIMATION_SHIFTING_COLOR_HPP_

#include "app/animation.hpp"


class ShiftingColorAnimation final:
        public Animation
{
public:
    static const inline std::uint8_t VARIANT_CNT = 6;
    static const inline std::size_t FRACTION_BITS = 8;

    enum ParamId: std::uint32_t
    {
        VARIANT = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
    };

    struct Segment
    {
        LedState color;
        LedSize length;
        LedSize transition_length;
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    struct Configuration
    {
        std::uint8_t variant = 0;
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
