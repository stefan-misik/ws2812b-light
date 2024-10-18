#include "driver/keypad.hpp"

#include "stm32g0xx_ll_gpio.h"
#include "support/cpu_pins.h"


namespace driver
{
namespace
{

struct GpioPin
{
    ::GPIO_TypeDef * gpio;
    std::uint32_t pin_mask;
};


const GpioPin KEY_PINS[Keypad::KEY_COUNT] = {
    {BTN_O_GPIO_Port, BTN_O_Pin},
    {BTN_LEFT_GPIO_Port, BTN_LEFT_Pin},
    {BTN_UP_GPIO_Port, BTN_UP_Pin},
    {BTN_DOWN_GPIO_Port, BTN_DOWN_Pin},
    {BTN_RIGHT_GPIO_Port, BTN_RIGHT_Pin},
    {BTN_X_GPIO_Port, BTN_X_Pin},
};


bool readPin(const GpioPin & pin)
{
    return ::LL_GPIO_IsInputPinSet(pin.gpio, pin.pin_mask);
}

}  // namespace

bool Keypad::initialize()
{
    for (auto & key : keys_)
        key.reset();
    return true;
}

void Keypad::update()
{
    for (std::size_t n = 0; n != KEY_COUNT; ++n)
        keys_[n].updateButton(!readPin(KEY_PINS[n]));
}


}  // namespace driver
