#include "driver/base.hpp"
#include "driver/systick.hpp"
#include "driver/led_controller.hpp"
#include "driver/ir_receiver.hpp"
#include "driver/keypad.hpp"
#include "driver/buzzer.hpp"
#include "driver/cpu_usage.hpp"
#include "tools/periodic_timer.hpp"
#include "input/keypad.hpp"
#include "input/ir_remote.hpp"
#include "lights.hpp"


driver::Systick system_time;
driver::LedController led_controller;
driver::IrReceiver ir_receiver;
driver::Keypad keypad;
driver::Buzzer buzzer;
driver::CpuUsage cpu_usage;
PeriodicTimer led_update_timer;

Lights lights;


int main()
{
    driver::Base::init();
    driver::Systick::initialize();
    led_controller.initialize(driver::TimerId::TIM_1, 2, 0);
    ir_receiver.initialize(driver::TimerId::TIM_3, 1);
    keypad.initialize();
    buzzer.initialize(driver::TimerId::TIM_16, 1);
    cpu_usage.initialize(driver::TimerId::TIM_6);
    lights.statusLeds().initialize();
    lights.initialize();

    // Keypad and IR Receiver are now managed by Input and their respective
    // Input Sources, no need to manage them further here
    lights.input().createSource<KeypadSource>(0, &keypad);
    lights.input().createSource<IrRemoteSource>(1, &ir_receiver);

    while (true)
    {
        const std::uint32_t current_time = system_time.currentTime();

        if (led_update_timer.shouldRun(current_time, 8))
        {
            cpu_usage.startPeriod();
            lights.step(current_time);
            led_controller.update(lights.leds());
            cpu_usage.endPeriod();
        }
    }

    return 0;
}


extern "C" void SysTick_Handler()
{
    system_time.handleInterrupt();
}

extern "C" void DMA1_Channel1_IRQHandler()
{
    led_controller.maybeHandleDmaInterrupt();
}
