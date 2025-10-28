#include "driver/base.hpp"
#include "driver/systick.hpp"
#include "tools/periodic_timer.hpp"
#include "app/lights.hpp"


driver::Systick system_time;
PeriodicTimer led_update_timer;

Lights lights;


int main()
{
    driver::Base::init();
    driver::Systick::initialize();
    lights.initialize();

    while (true)
    {
        const std::uint32_t current_time = system_time.currentTime();

        if (led_update_timer.shouldRun(current_time, 8))
        {
            lights.io().cpuUsage().startPeriod();
            lights.step(current_time);
            lights.io().cpuUsage().endPeriod();
        }
        lights.io().run();
    }

    return 0;
}


extern "C" void SysTick_Handler()
{
    system_time.handleInterrupt();
}

extern "C" void DMA1_Channel1_IRQHandler()
{
    lights.io().ledController().maybeHandleDmaInterrupt();
}

extern "C" void I2C1_IRQHandler()
{
    lights.io().i2cBus().maybeHandleI2cInterrupt();
}
