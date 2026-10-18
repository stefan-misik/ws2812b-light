/**
 * @file
 */

#ifndef APP_ANIMATION_BOUNCE_HPP_
#define APP_ANIMATION_BOUNCE_HPP_

#include "app/animation.hpp"


/**
 * @brief Larson-scanner ("Cylon eye") style bouncing animation
 *
 * A bright head sweeps back and forth across the strip, reflecting off each end instead of
 * wrapping around, with a symmetric fading trail following it on both sides.
 */
class BounceAnimation final:
        public Animation
{
public:
    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
        HUE,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    /** @brief Number of fractional bits used for the sub-LED position accumulator */
    static const inline std::uint8_t FRACTION_BITS = 8;
    /** @brief Number of LEDs making up the fading trail on each side of the bouncing head */
    static const inline std::uint8_t TAIL_LENGTH = 4;

    struct Configuration
    {
        std::uint8_t speed = 4;
        std::uint16_t hue = 0;
    };

    struct State
    {
        std::uint32_t position = 0;
        /** @brief Current direction of travel: 1 - increasing position, 0 - decreasing position */
        std::uint8_t direction = 1;
    };

    Configuration config_;
    State state_;
};


#endif  // APP_ANIMATION_BOUNCE_HPP_
