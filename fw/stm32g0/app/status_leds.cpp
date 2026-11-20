#include "app/status_leds.hpp"


namespace
{

}  // namespace

bool StatusLeds::run()
{
    if ((UPDATE_INTERVAL_STEPS - 1u) != step_)
    {
        ++step_;
        return false;
    }
    step_ = 0;

    // Move pos
    pos_ = (BIT_COUNT - 1u) == pos_ ? 0u : (pos_ + 1u);

    std::uint8_t led_states = 0u;

    std::uint8_t led_mask = 1u;
    const std::uint32_t pos_mask = 1ul << pos_;
    for (auto & led_state: led_state_)
    {
        if (0u != led_state.one_shot_pos)
        {
            if (0x1u & (led_state.one_shot))
                led_states |= led_mask;
            led_state.one_shot >>= 1u;
            --led_state.one_shot_pos;
        }
        else
        {
            if (0u != (led_state.repeat & pos_mask))
                led_states |= led_mask;
        }
        led_mask <<= 1u;
    }

    led_states_ = led_states;
    return true;
}