/**
 * @file
 */

#ifndef SHARED_TOOLS_BUTTON_FILTER_H_
#define SHARED_TOOLS_BUTTON_FILTER_H_

#include "defs.h"

class ButtonFilter
{
public:
    static const def::Uint8 PRESS_THRESHOLD = 5;
    static const def::Uint8 REPEAT_THRESHOLD = PRESS_THRESHOLD + 50;
    static const def::Uint8 NEXT_REPEAT_THRESHOLD = REPEAT_THRESHOLD + 8;

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
     * @brief Similar to @ref updateButton(), but no de-bouncing is performed
     *
     * @param button_state Current button state
     */
    void updateHardButton(bool button_state);

    /**
     * @brief Get the current state of the button
     *
     * @return Bitwise or of values from @ref State
     */
    def::Uint8 state() const
    {
        return state_;
    }

private:
    def::Uint8 counter_;
    def::Uint8 state_;

    void updateState();
};


#endif  // SHARED_TOOLS_BUTTON_FILTER_H_
