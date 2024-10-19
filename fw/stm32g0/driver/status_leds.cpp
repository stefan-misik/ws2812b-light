#include "driver/status_leds.hpp"

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


const GpioPin STATUS_LED_PINS[StatusLeds::LED_COUNT] = {
        {STATUS_LED1_GPIO_Port, STATUS_LED1_Pin},
        {STATUS_LED2_GPIO_Port, STATUS_LED2_Pin},
        {STATUS_LED3_GPIO_Port, STATUS_LED3_Pin},
};


void writePin(const GpioPin & pin, bool value)
{
    if (value)
        ::LL_GPIO_SetOutputPin(pin.gpio, pin.pin_mask);
    else
        ::LL_GPIO_ResetOutputPin(pin.gpio, pin.pin_mask);
}


}  // namespace


bool StatusLeds::initialize()
{
    for (auto & led : led_state_)
        led = false;
    return true;
}

void StatusLeds::update()
{
    for (std::size_t n = 0; n != LED_COUNT; ++n)
        writePin(STATUS_LED_PINS[n], led_state_[n]);
}


}  // namespace driver
