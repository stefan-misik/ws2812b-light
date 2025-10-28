/**
 * @file
 */

#ifndef APP_ANIMATION_RAINBOW_HPP_
#define APP_ANIMATION_RAINBOW_HPP_

#include "app/animation.hpp"


class RainbowAnimation final:
        public Animation
{
public:
    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

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
};



#endif  // ANIMATION_RAINBOW_HPP_
