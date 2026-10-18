#include "driver/base.hpp"
#include "driver/systick.hpp"
#include "tools/periodic_timer.hpp"
#include "app/lights.hpp"


driver::Systick system_time;
PeriodicTimer led_update_timer;
PeriodicTimer dump_stats_timer;

Lights lights;


void dumpCpuUsage()
{
    using LogToken = Logger::LevelToken<Logger::Level::TRACE>;
    if constexpr (!LogToken::ENABLED)
        return;
    auto & cpu_usage = lights.io().cpuUsage();

    const auto & stats = cpu_usage.stats();

    const bool success = lights.logger().log<LogToken, "⏱️  min: %u ‰, max: %u ‰">(
        stats.min / 8, stats.max / 8);

    if (success)
        cpu_usage.clearStats();
}

int main()
{
    driver::Base::init();
    driver::Systick::initialize();
    lights.initialize();

    lights.logger().log<Logger::Level::INFO, "Application started 🎄">();

    while (true)
    {
        const std::uint32_t current_time = system_time.currentTime();

        if (led_update_timer.shouldRun(current_time, 8))
        {
            lights.io().cpuUsage().startPeriod();
            lights.step(current_time);
            lights.io().cpuUsage().endPeriod();
        }

        if (dump_stats_timer.shouldRun(current_time, 1000))
        {
            dumpCpuUsage();
        }

        lights.runBackgroundTasks(current_time);
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

extern "C" void DMA1_Channel4_5_6_7_DMAMUX_DMA2_Channel1_2_3_4_5_IRQHandler()
{
    lights.io().debugUart().maybeHandleDmaInterrupt();
}

extern "C" void I2C1_IRQHandler()
{
    lights.io().i2cBus().maybeHandleI2cInterrupt();
}
