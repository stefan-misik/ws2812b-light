/**
 * @file
 */

#ifndef APP_LIGHTS_HPP_
#define APP_LIGHTS_HPP_

#include <array>

#include "app/io.hpp"
#include "app/animation_storage.hpp"
#include "app/event_queue.hpp"
#include "app/input.hpp"
#include "led_strip.hpp"
#include "app/animation_register.hpp"
#include "app/music.hpp"


/**
 * @brief Class managing main application of the LED lights
 */
class Lights
{
public:

    static const inline std::size_t ANIMATION_SLOT_CNT = 14;

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

    /**
     * @brief Perform background tasks
     *
     * @param current_time Current time in milliseconds
     */
    void runBackgroundTasks(std::uint32_t current_time);

    Io & io() { return io_; }

private:
    Io io_;

    std::size_t current_register_;
    AnimationStorage animation_;

    EventQueue event_queue_;
    Input input_;
    LedStrip<100> leds_;

    std::array<AnimationRegister, ANIMATION_SLOT_CNT> animation_file_;

    Music music_;

    void handleEvents();
    Music::Result handleMusic();
    bool handleInputEvent(const Input::EventParam & e);

    void switchAnimation(int dir);
};



#endif  // APP_LIGHTS_HPP_
