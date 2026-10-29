#ifndef TESTS_DRIVER_I2C_BUS_FAKE_HPP_
#define TESTS_DRIVER_I2C_BUS_FAKE_HPP_

#include <vector>

#include "driver/i2c_bus.hpp"

namespace test_i2c
{

struct EnqueueRecord
{
    driver::I2cBus::Transaction * transaction;
    bool interrupts_disabled;
};

void reset();
const std::vector<EnqueueRecord> & enqueued();
void completeNext(driver::I2cBus & bus, driver::I2cBus::Transaction::Status status);

}  // namespace test_i2c

#endif  // TESTS_DRIVER_I2C_BUS_FAKE_HPP_