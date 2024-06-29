#include "buttons.h"

#include <avr/io.h>

namespace
{

using ButtonId = Buttons::ButtonId;

inline ButtonId classifyButton(uint16_t keypad_value)
{
    if (keypad_value < 73u)
        return ButtonId::NONE;
    else if (keypad_value <= 219u)
        return ButtonId::O_BTN;
    else if (keypad_value <= 364u)
        return ButtonId::LEFT;
    else if (keypad_value <= 501u)
        return ButtonId::UP;
    else if (keypad_value <= 637u)
        return ButtonId::DOWN;
    else if (keypad_value <= 793u)
        return ButtonId::RIGHT;
    else if (keypad_value <= 951u)
        return ButtonId::X_BTN;

    return ButtonId::NONE;
}

}  // namespace

void Buttons::initialize()
{
    current_button_ = ButtonId::NONE;
}

uint8_t Buttons::run(uint16_t keypad_value)
{
    const auto button = classifyButton(keypad_value);

    bool button_state = false;
    if (button == current_button_)
    {
        button_state = true;
    }
    else
    {
        if (0 == (filter_.state() & ButtonFilter::PRESSED))
        {
            filter_.reset();
            current_button_ = button;
            button_state = true;
        }
    }

    if (ButtonId::NONE == current_button_)
        return 0;
    filter_.updateButton(button_state);

    return filter_.state();
}
