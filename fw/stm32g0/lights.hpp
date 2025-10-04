/**
 * @file
 */

#ifndef LIGHTS_HPP_
#define LIGHTS_HPP_

#include <array>

#include "io.hpp"
#include "animation_storage.hpp"
#include "event_queue.hpp"
#include "input.hpp"
#include "led_strip.h"
#include "animation_register.hpp"


/**
 * @brief Class managing main application of the LED lights
 */
class Lights
{
public:

    static const inline std::size_t ANIMATION_SLOT_CNT = 2;

    Lights();

    /**
     * @brief Initialize the application
     *
     * @return Success
     */
    bool initialize();

    /**
     * @brief Led lights application step
     *
     * Call every 8 ms.
     *
     * @param current_time Current time in milliseconds
     */
    void step(std::uint32_t current_time);

    Io & io() { return io_; }

private:
    Io io_;

    std::size_t current_register_;
    AnimationStorage animation_;

    EventQueue event_queue_;
    Input input_;
    LedStrip<100> leds_;

    std::array<AnimationRegister, ANIMATION_SLOT_CNT> animation_file_;

    void handleEvents();
    bool handleInputEvent(const Input::EventParam & e);

    void switchAnimation(int dir);
};



#endif  // LIGHTS_HPP_
