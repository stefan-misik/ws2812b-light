/**
 * @file
 */

#ifndef IO_HPP_
#define IO_HPP_

#include "driver/status_leds.hpp"
#include "driver/led_controller.hpp"
#include "driver/ir_receiver.hpp"
#include "driver/keypad.hpp"
#include "driver/buzzer.hpp"
#include "driver/cpu_usage.hpp"


/**
 * @brief Container for all the driver objects
 */
class Io
{
public:
    bool initialize();

    driver::StatusLeds & statusLeds() { return status_leds_; }
    driver::LedController & ledController() { return led_controller_; }
    driver::IrReceiver & irReceiver() { return ir_receiver_; }
    driver::Keypad & keypad() { return keypad_; }
    driver::Buzzer & buzzer() { return buzzer_; }
    driver::CpuUsage & cpuUsage() { return cpu_usage_; }

private:
    driver::StatusLeds status_leds_;
    driver::LedController led_controller_;
    driver::IrReceiver ir_receiver_;
    driver::Keypad keypad_;
    driver::Buzzer buzzer_;
    driver::CpuUsage cpu_usage_;
};

#endif  // IO_HPP_
