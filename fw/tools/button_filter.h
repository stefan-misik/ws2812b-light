/**
 * @file
 */

#ifndef TOOLS_BUTTON_FILTER_H_
#define TOOLS_BUTTON_FILTER_H_

#include <stdint.h>

class ButtonFilter
{
public:
    enum State
    {
        /** @brief Indicates current state of the button */
        PRESSED = 0x01,
        /** @brief Set in an update cycle when the button is first pressed */
        DOWN = 0x02,
        /** @brief Set in an update cycle when the button is released */
        UP = 0x04,
        /** @brief Set every update cycle a press event is generated */
        PRESS = 0x08
    };

    ButtonFilter():
        counter_(0),
        state_(0)
    { }

    void reset()
    {
        counter_ = 0;
        state_ = 0;
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

    /**
     * @brief Get the current state of the button
     *
     * @return Bitwise or of values from @ref State
     */
    uint8_t state() const
    {
        return state_;
    }

private:
    static constexpr uint8_t PRESS_THRESHOLD = 5;
    static constexpr uint8_t REPEAT_THRESHOLD = PRESS_THRESHOLD + 50;
    static constexpr uint8_t NEXT_REPEAT_THRESHOLD = REPEAT_THRESHOLD + 8;

    uint8_t counter_;
    uint8_t state_;

    void countUp();
    void countDown();
    void updateState();
};


#endif  // TOOLS_BUTTON_FILTER_H_
