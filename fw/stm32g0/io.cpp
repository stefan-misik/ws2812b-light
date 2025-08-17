#include "io.hpp"



bool Io::initialize()
{
    status_leds_.initialize();
    led_controller_.initialize(driver::TimerId::TIM_1, 2, driver::DmaChannelId::DMA_1);
    ir_receiver_.initialize(driver::TimerId::TIM_3, driver::DmaChannelId::DMA_2);
    keypad_.initialize();
    buzzer_.initialize(driver::TimerId::TIM_16, 1);
    cpu_usage_.initialize(driver::TimerId::TIM_6);

    return true;
}
