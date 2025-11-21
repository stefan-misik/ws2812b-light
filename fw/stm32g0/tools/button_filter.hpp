/**
 * @file
 */

#ifndef TOOLS_BUTTON_FILTER_HPP_
#define TOOLS_BUTTON_FILTER_HPP_

#include <cstdint>

class ButtonFilter
{
public:
    static const std::uint8_t PRESS_THRESHOLD = 5;

    ButtonFilter():
        counter_(0),
        is_pressed_(0)
    { }

    void reset()
    {
        counter_ = 0;
        is_pressed_ = 0;
    }

    /**
     * @brief Update the state of the button filter with current state of the
     *        button
     *
     * This method shall be called periodically each couple of milliseconds to
     * filter the button states and generate button press events.
     *
     * @param button_state Current button state
     */
    void updateButton(bool button_state);

    bool isPressed() const
    {
        return is_pressed_;
    }

private:
    std::uint8_t counter_;
    bool is_pressed_;
};


#endif  // TOOLS_BUTTON_FILTER_HPP_
