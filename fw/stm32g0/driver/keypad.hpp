/**
 * @file
 */

#ifndef DRIVER_KEYPAD_HPP_
#define DRIVER_KEYPAD_HPP_

#include "driver/common.hpp"
#include "tools/button_filter.h"


namespace driver
{


class Keypad
{
public:
    enum class KeyId
    {
        KEY_O,
        KEY_LEFT,
        KEY_UP,
        KEY_DOWN,
        KEY_RIGHT,
        KEY_X,

        KEY_COUNT_,
    };

    static const inline std::size_t KEY_COUNT = static_cast<std::size_t>(KeyId::KEY_COUNT_);

    bool initialize();

    bool isPressed(KeyId id) const
    {
        return keys_[static_cast<std::size_t>(id)].isPressed();
    }

    /**
     * @brief Update keypad
     *
     * This needs to be called in regular intervals.
     */
    void update();

private:
    ButtonFilter keys_[KEY_COUNT];
};


}  // driver


#endif  // DRIVER_KEYPAD_HPP_
