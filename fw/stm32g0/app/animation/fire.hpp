/**
 * @file
 */

#ifndef APP_ANIMATION_FIRE_HPP_
#define APP_ANIMATION_FIRE_HPP_

#include <array>

#include "app/animation.hpp"


class FireAnimation final:
        public Animation
{
public:
    static const inline LedSize MAX_STRIP_SIZE = 128;

    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
        COOLING,
        SPARKING,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    /** @brief Number of steps needed to accumulate before a simulation update happens */
    static const inline std::uint16_t UPDATE_THRESHOLD = 16;

    struct Configuration
    {
        std::uint8_t speed = 4;
        std::uint8_t cooling = 55;
        std::uint8_t sparking = 120;
    };

    struct State
    {
    };

    Configuration config_;
    State state_;

    // Heat simulation buffer, intentionally not part of Configuration/State: it is
    // re-seeded from scratch (via default member initialization) whenever the animation
    // instance is (re-)created, and does not need to survive a slot switch.
    std::array<std::uint8_t, MAX_STRIP_SIZE> heat_{};
    std::uint16_t accumulator_ = 0;

    void step(LedSize led_count);
};


#endif  // APP_ANIMATION_FIRE_HPP_
