#include "input/keypad.hpp"


namespace
{

using KeyId = Input::KeyId;
using RawKeyId = driver::Keypad::KeyId;

}  // namespace


void KeypadSource::getPressedKeys(std::uint32_t time, Input::PressedButtonList * buttons)
{
    (void) time;  // Unused

    keypad_->update();

    for (std::size_t key = 0; key != driver::Keypad::KEY_COUNT; ++key)
    {
        if (keypad_->isPressed(static_cast<RawKeyId>(key)))
            buttons->addKey(static_cast<KeyId>(key));
    }
}
