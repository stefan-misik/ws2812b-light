/**
 * @file
 */

#ifndef BUTTONS_H_
#define BUTTONS_H_


#include "tools/button_filter.h"

#include <stdint.h>

class Buttons
{
public:
    enum ButtonId: uint8_t
    {
        NONE = 0,
        O_BTN,
        LEFT,
        UP,
        DOWN,
        RIGHT,
        X_BTN,

        BUTTON_COUNT
    };

    ButtonId button() const { return current_button_; }

    void initialize();
    uint8_t run(uint16_t keypad_value);

private:
    ButtonId current_button_ = ButtonId::NONE;
    ButtonFilter filter_;
};

#endif  // BUTTONS_H_
