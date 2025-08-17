/**
 * @file
 */

#ifndef LIGHTS_HPP_
#define LIGHTS_HPP_

#include "tools/polymorphic_storage.hpp"
#include "io.hpp"
#include "animation.hpp"
#include "event_queue.hpp"
#include "input.hpp"
#include "led_strip.h"


/**
 * @brief Class managing main application of the LED lights
 */
class Lights
{
public:
    using AnimationStorage = PolymorphicStorage<Animation, 64>;

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

    AnimationStorage animation_;

    EventQueue event_queue_;
    Input input_;
    LedStrip<100> leds_;
};



#endif  // LIGHTS_HPP_
