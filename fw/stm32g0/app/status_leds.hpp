/**
 * @file
 */

#ifndef APP_STATUS_LEDS_HPP_
#define APP_STATUS_LEDS_HPP_

#include <cstddef>
#include <cstdint>
#include <array>

#include "driver/status_leds.hpp"


/**
 * @brief Writes diagnostic messages through a DMA-driven UART
 */
class StatusLeds
{
public:
    using LedPatternUint = std::uint32_t;

    /** @brief LED animation interval in animation step intervals */
    static constexpr std::uint8_t UPDATE_INTERVAL_STEPS = 5;
    /** @brief Number of bits in LedPatternUint */
    static constexpr std::uint8_t BIT_COUNT = 8u * sizeof(StatusLeds::LedPatternUint);

    static const inline std::size_t LED_COUNT = driver::StatusLeds::LED_COUNT;
    using LedId = driver::StatusLeds::LedId;

    /**
     * @brief Set the Repeat Pattern
     *
     * @param led LedId of the LED to set the pattern for
     * @param pattern Bitfield of the pattern to set for the LED
     */
    void setRepeatPattern(LedId led, LedPatternUint pattern)
    {
        led_state_[static_cast<std::size_t>(led)].repeat = pattern;
    }

    /**
     * @brief Set the One-Shot Pattern
     *
     * @param led LedId of the LED to set the pattern for
     * @param pattern Bitfield of the pattern to set for the LED
     * @param duration Duration of the one-shot pattern
     */
    void setOneShotPattern(LedId led, LedPatternUint pattern, std::uint8_t duration = BIT_COUNT)
    {
        auto & led_state = led_state_[static_cast<std::size_t>(led)];
        led_state.one_shot = pattern;
        led_state.one_shot_pos = duration;
    }

    /**
     * @brief Updates the LED states based on the current animation step
     *
     * @return The LED states have changed
     */
    bool run();

    /**
     * @brief Returns the current LED states
     *
     * @return std::uint8_t Bitfield of the current LED states.
     */
    std::uint8_t ledStates() const { return led_states_; }

private:
    struct LedState
    {
        LedPatternUint repeat = 0u;
        LedPatternUint one_shot = 0u;
        std::uint8_t one_shot_pos = 0u;
    };

    LedState led_state_[LED_COUNT];
    std::uint8_t step_ = 0;
    std::uint8_t pos_ = 0;
    std::uint8_t led_states_;
};


#endif  // APP_STATUS_LEDS_HPP_