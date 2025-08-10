/**
 * @file
 */

#ifndef ANIMATION_RAINBOW_HPP_
#define ANIMATION_RAINBOW_HPP_

#include "animation.hpp"


class RainbowAnimation final:
        public Animation
{
public:
    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    bool getParameter(std::uint32_t param_id, int * value) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    struct Configuration
    {
        std::uint8_t space_increment = 8;
        std::uint8_t time_increment = 4;
    };

    struct State
    {
        std::uint16_t hue = 0;
    };

    Configuration config_;
    State state_;
    std::uint8_t step_ = 0;

    static const inline std::uint16_t MAX_HUE = 0x02FF;

    static void toSaturatedHue(std::uint16_t hue, LedState * color)
    {
        const std::uint8_t secondary_value = static_cast<std::uint8_t>(hue & 0xFF);
        const std::uint8_t primary_value = 0xFF - secondary_value;

        switch (static_cast<std::uint8_t>(hue >> 8))
        {
        case 0:
            color->red = primary_value;
            color->green = secondary_value;
            color->blue = 0;
            break;

        case 1:
            color->red = 0;
            color->green = primary_value;
            color->blue = secondary_value;
            break;

        case 2:
            color->red = secondary_value;
            color->green = 0;
            color->blue = primary_value;
            break;

        default:
            color->red = 0;
            color->green = 0;
            color->blue = 0;
            break;
        }
    }

    static std::uint16_t incrementHue(std::uint16_t hue, std::int16_t value = 1)
    {
        std::int16_t new_value = value + static_cast<std::int16_t>(hue);
        if (new_value < 0)
            return (MAX_HUE + 1) + new_value;
        else if (new_value > static_cast<std::int16_t>(MAX_HUE))
            return new_value - (MAX_HUE + 1);
        else
            return new_value;
    }
};



#endif  // ANIMATION_RAINBOW_HPP_
