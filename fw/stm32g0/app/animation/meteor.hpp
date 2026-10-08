/**
 * @file
 */

#ifndef APP_ANIMATION_METEOR_HPP_
#define APP_ANIMATION_METEOR_HPP_

#include <array>
#include <cstdint>

#include "app/animation.hpp"


/**
 * @brief Meteor shower animation
 *
 * Simulates several meteors chasing each other around the strip, each with its own hue,
 * speed and randomized re-spawn delay, leaving a smoothly fading tail behind its bright head.
 */
class MeteorAnimation final:
        public Animation
{
public:
    /** @brief Maximum number of meteors that can be active at once */
    static const inline std::uint8_t MAX_METEORS = 4;

    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
        COUNT,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    /** @brief Number of fractional bits used for the sub-LED position accumulator */
    static const inline std::uint8_t FRACTION_BITS = 8;
    /** @brief Number of LEDs making up the fading tail behind a meteor's head */
    static const inline std::uint8_t TAIL_LENGTH = 6;

    /** @brief Single meteor's runtime state */
    struct Meteor
    {
        /** @brief Sub-LED position accumulator, wraps around at the strip length */
        std::uint32_t position = 0;
        /** @brief Hue of the meteor's head/tail color */
        std::uint16_t hue = 0;
        /** @brief Relative speed multiplier, applied to the common speed setting (8 == 1.0x) */
        std::uint8_t speed_factor = 8;
        /** @brief Number of frames left to wait before the meteor (re-)appears */
        std::uint8_t delay = 0;
    };

    struct Configuration
    {
        std::uint8_t speed = 4;
        std::uint8_t count = 3;
    };

    struct State
    {
        // Distinct initial values so the meteors do not all overlap right from the start
        std::array<Meteor, MAX_METEORS> meteors = {{
            {0, 0, 8, 0},
            {0, 200, 10, 3},
            {0, 400, 6, 7},
            {0, 600, 12, 11},
        }};
    };

    Configuration config_;
    State state_;

    void respawn(Meteor * meteor, LedSize led_count);
};


#endif  // APP_ANIMATION_METEOR_HPP_
