/**
 * @file
 */

#ifndef APP_ANIMATION_COMET_HPP_
#define APP_ANIMATION_COMET_HPP_

#include "app/animation.hpp"


class CometAnimation final:
        public Animation
{
public:
    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    /** @brief Number of fractional bits used for the sub-LED position accumulator */
    static const inline std::uint8_t FRACTION_BITS = 8;
    /** @brief Number of LEDs making up the fading tail behind the comet's head */
    static const inline std::uint8_t TAIL_LENGTH = 8;

    struct Configuration
    {
        std::uint8_t speed = 4;
    };

    struct State
    {
        std::uint32_t position = 0;
    };

    Configuration config_;
    State state_;
};


#endif  // APP_ANIMATION_COMET_HPP_
