/**
 * @file
 */

#ifndef APP_ANIMATION_LIGHTS_HPP_
#define APP_ANIMATION_LIGHTS_HPP_

#include <array>

#include "app/animation.hpp"


class LightsAnimation final:
        public Animation
{
public:
    static const inline LedSize MAX_STRIP_SIZE = 128;

    enum ParamId: std::uint32_t
    {
        UNUSED_0_ = Animation::ParamId::FIRST_CUSTOM_ID_,
        SPEED,
        SYNCHRONIZED,
    };

    class LedFlags
    {
    public:
        LedFlags():
            pos_(0),
            is_transitioning_(false)
        { }

        static std::uint8_t next(std::uint8_t pos, std::uint8_t count)
        {
            const auto next_pos = pos + 1;
            return next_pos >= count ? 0 : next_pos;
        }

        std::uint8_t position() const { return pos_; }
        bool isTransitioning() const { return is_transitioning_; }
        std::uint8_t nextValue(std::uint8_t count) const { return next(pos_, count); }

        void reset(std::uint8_t value = 0)
        {
            pos_ = value;
            is_transitioning_ = false;
        }

        void next(std::uint8_t count)
        {
            pos_ = next(pos_, count);
            is_transitioning_ = false;
        }

        void startTransitioning()
        {
            is_transitioning_ = true;
        }

        bool isDone() const
        {
            return !isTransitioning();
        }

        bool isDone(std::uint8_t required_pos) const
        {
            return !isTransitioning() && (position() == required_pos);
        }

    private:
        std::uint8_t pos_: 4;
        std::uint8_t is_transitioning_: 4;
    };

    class Transition
    {
    public:
        static constexpr const LedSize INVALID_LED_ID = 0xFFFFul;

        class LimittedCounter
        {
        public:
            void reset() { value_ = 0; }
            std::uint16_t value() const { return value_; }

            void step(std::uint16_t step_size, std::uint16_t end_value)
            {
                const std::uint16_t remaining = end_value - value_;
                if (remaining < step_size)
                    value_ = end_value;
                else
                    value_ += step_size;
            }

        private:
            std::uint16_t value_ = 0;
        };


        bool isValid() const { return INVALID_LED_ID != led_id_; }
        LedSize ledId() const { return led_id_; }
        std::uint16_t state() const { return state_.value(); }

        void reset()
        {
            led_id_ = INVALID_LED_ID;
        }

        void start(LedSize led_id)
        {
            led_id_ = led_id;
            state_.reset();
        }

        void step(std::uint16_t step_size, std::uint16_t end_state)
        {
            state_.step(step_size, end_state);
        }

    private:
        LedSize led_id_ = INVALID_LED_ID;
        LimittedCounter state_;
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    static const inline std::size_t MAX_TRANSITIONS = 16;
    static const inline std::size_t COLOR_THEME_MAX_LENGTH = 8;

    struct Configuration
    {
        std::uint8_t theme_length;
        std::array<LedState, COLOR_THEME_MAX_LENGTH> theme;
        std::uint8_t speed = 3;
        bool synchronized = false;
    };

    struct State
    {
        std::uint8_t synchronized_pos = 0ul;
    };

    Configuration config_;
    State state_;

    std::array<Transition, MAX_TRANSITIONS> transitions_;
    std::array<LedFlags, MAX_STRIP_SIZE> flags_;

    Transition::LimittedCounter spawn_counter_;

    void reset()
    {
        state_.synchronized_pos = 0ul;
        for (auto & t : transitions_)
            t.reset();
        for (auto & f : flags_)
            f.reset();
    }
};



#endif  // APP_ANIMATION_LIGHTS_HPP_
