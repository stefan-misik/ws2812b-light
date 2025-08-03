#include "input/ir_remote.hpp"


namespace
{

using IrCode = driver::IrReceiver::Code;
using KeyId = Input::KeyId;

inline KeyId decodeKey(const IrCode ir_code)
{
    switch (ir_code.address())
    {
    case 0x00:
        switch (ir_code.command())
        {
        case 0x18: return KeyId::KEY_UP;
        case 0x5A: return KeyId::KEY_RIGHT;
        case 0x52: return KeyId::KEY_DOWN;
        case 0x08: return KeyId::KEY_LEFT;
        case 0x16: return KeyId::KEY_O;
        case 0x0D: return KeyId::KEY_X;
        case 0x19: return KeyId::KEY_0;
        case 0x45: return KeyId::KEY_1;
        case 0x46: return KeyId::KEY_2;
        case 0x47: return KeyId::KEY_3;
        case 0x44: return KeyId::KEY_4;
        case 0x40: return KeyId::KEY_5;
        case 0x43: return KeyId::KEY_6;
        case 0x07: return KeyId::KEY_7;
        case 0x15: return KeyId::KEY_8;
        case 0x09: return KeyId::KEY_9;
        case 0x1C: return KeyId::KEY_OK;
        }
        break;

    case 0x01:
        switch (ir_code.command())
        {
        case 0x16: return KeyId::KEY_UP;
        case 0x50: return KeyId::KEY_RIGHT;
        case 0x1A: return KeyId::KEY_DOWN;
        case 0x51: return KeyId::KEY_LEFT;
        case 0x13: return KeyId::KEY_O;
        case 0x19: return KeyId::KEY_X;
        case 0x40: return KeyId::KEY_POWER;
        case 0x43: return KeyId::KEY_RED;
        case 0x0F: return KeyId::KEY_GREEN;
        case 0x10: return KeyId::KEY_YELLOW;
        case 0x18: return KeyId::KEY_BLUE;
        case 0x11: return KeyId::KEY_HOME;
        case 0x4C: return KeyId::KEY_MENU;
        case 0x01: return KeyId::KEY_0;
        case 0x4E: return KeyId::KEY_1;
        case 0x0D: return KeyId::KEY_2;
        case 0x0C: return KeyId::KEY_3;
        case 0x4A: return KeyId::KEY_4;
        case 0x09: return KeyId::KEY_5;
        case 0x08: return KeyId::KEY_6;
        case 0x46: return KeyId::KEY_7;
        case 0x05: return KeyId::KEY_8;
        case 0x04: return KeyId::KEY_9;
        case 0x41: return KeyId::KEY_MUTE;
        case 0x42: return KeyId::KEY_BACKSPACE;
        }
        break;
    }
    return KeyId::KEY_NONE;
}

inline void handleCode(Input::PressedButtonList * buttons, IrCode code)
{
    const auto key = decodeKey(code);
    if (KeyId::KEY_NONE != key)
        buttons->addKey(key);
}

}  // namespace


void IrRemoteSource::getPressedKeys(std::uint32_t time, Input::PressedButtonList * buttons)
{
    while (true)
    {
        const auto [code, is_new] = receiver_->read(time);
        if (!code.isValid())
            break;

        handleCode(buttons, code);

        // In case the button press was a new, try to fetch more key presses, as
        // there may be more key presses in the buffer.
        if (!is_new)
            break;
    }
}
