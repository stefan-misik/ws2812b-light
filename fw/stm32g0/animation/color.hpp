/**
 * @file
 */

#ifndef ANIMATION_COLOR_HPP_
#define ANIMATION_COLOR_HPP_

#include "animation.hpp"


class ColorAnimation final:
        public Animation
{
public:
    static const inline std::size_t FIRST_COLOR = 1;

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    struct Configuration
    {
        std::uint8_t color = 0;
    };

    struct State
    {
    };

    Configuration config_;
    State state_;
};



#endif  // ANIMATION_COLOR_HPP_
