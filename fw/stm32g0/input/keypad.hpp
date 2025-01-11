/**
 * @file
 */

#ifndef INPUT_KEYPAD_HPP_
#define INPUT_KEYPAD_HPP_

#include "input.hpp"
#include "driver/keypad.hpp"


class KeypadSource:
        public Input::Source
{
public:
    KeypadSource(driver::Keypad * keypad):
        keypad_(keypad)
    { }

    void getPressedKeys(std::uint32_t time, Input::ButtonStateList * buttons) final;

private:
    driver::Keypad * const keypad_;
};


#endif  // INPUT_KEYPAD_HPP_
