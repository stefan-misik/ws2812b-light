#include "app/io.hpp"



bool Io::initialize()
{
    status_leds_.initialize();
    led_controller_.initialize(driver::TimerId::TIM_1, 2, driver::DmaChannelId::DMA_1);
    ir_receiver_.initialize(driver::TimerId::TIM_3, driver::DmaChannelId::DMA_2);
    keypad_.initialize();
    buzzer_.initialize(driver::TimerId::TIM_16, 1);
    i2c_bus_.initialize(driver::I2cId::I2C_1, driver::DmaChannelId::DMA_3, 16);
    cpu_usage_.initialize(driver::TimerId::TIM_6);

    return true;
}

void Io::run()
{
    // Handle I2C transactions
    {
        const auto eeprom_address = eeprom_.address();
        while (auto * transaction = i2c_bus_.get())
        {
            if (transaction->address() == eeprom_address)
                eeprom_.handleResponse(transaction);
            i2c_bus_.release(transaction);
        }
        eeprom_.createRequest(&i2c_bus_);
    }
}
