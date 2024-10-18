#include "driver/keypad.hpp"


#include "stm32g0xx_ll_gpio.h"


namespace driver
{
namespace
{

::GPIO_TypeDef * const GPIOS[3] = { GPIOA, GPIOB, GPIOC, };

inline ::GPIO_TypeDef * toGpio(GpioId id)
{
    return GPIOS[static_cast<std::size_t>(id)];
}

inline std::uint32_t toPinMask(std::uint8_t pin)
{
    return 1 << pin;
}


bool readPin(const GpioPinId & pin)
{
    return ::LL_GPIO_IsInputPinSet(toGpio(pin.gpio), toPinMask(pin.pin));
}

}  // namespace

bool Keypad::initialize(std::initializer_list<GpioPinId> pins)
{
    std::size_t n = 0;
    for (const GpioPinId * pos = pins.begin(); pos != pins.end(); ++pos, ++n)
        pins_[n] = *pos;

    return true;
}

void Keypad::update()
{
    for (std::size_t n = 0; n != KEY_COUNT; ++n)
        keys_[n].updateButton(readPin(pins_[n]));
}


}  // namespace driver
