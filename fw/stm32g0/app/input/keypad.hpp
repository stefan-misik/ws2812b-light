/**
 * @file
 */

#ifndef APP_INPUT_KEYPAD_HPP_
#define APP_INPUT_KEYPAD_HPP_

#include "app/input.hpp"
#include "driver/keypad.hpp"


class KeypadSource final:
        public Input::Source
{
public:
    KeypadSource(driver::Keypad * keypad):
        keypad_(keypad)
    { }

    void getPressedKeys(std::uint32_t time, Input::PressedButtonList * buttons) final;

private:
    driver::Keypad * const keypad_;
};


#endif  // APP_INPUT_KEYPAD_HPP_
