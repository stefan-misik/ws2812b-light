/**
 * @file
 */

#ifndef APP_ANIMATION_TWINKLE_HPP_
#define APP_ANIMATION_TWINKLE_HPP_

#include "app/animation.hpp"


class TwinkleAnimation final:
        public Animation
{
public:
    static const inline std::uint8_t VARIANT_CNT = 2;

    enum ParamId: std::uint32_t
    {
        VARIANT = Animation::ParamId::FIRST_CUSTOM_ID_,
        FREQUENCY,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    class Blink
    {
    public:
        bool isDone() const { return 0 == state_; }
        std::uint8_t position() const { return state_ - 1; }
        void next() { ++state_; }
        void done() { state_ = 0; }

        void start(LedSize led) { led_ = led; state_ = 1; }

        LedSize led() const { return led_; }

    private:
        LedSize led_;
        std::uint8_t state_;
    };

    struct Configuration
    {
        std::uint8_t variant = 0;
        std::uint8_t frequency = 3;
    };

    struct State
    {
        static const inline std::uint8_t BLINK_CNT = 7;

        Blink blinks[BLINK_CNT];
    };

    Configuration config_;
    State state_;
    std::uint8_t step_ = 0;
};



#endif  // APP_ANIMATION_TWINKLE_HPP_
