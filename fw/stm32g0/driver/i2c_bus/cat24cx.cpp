#include "driver/i2c_bus/cat24cx.hpp"


namespace driver
{
namespace i2c
{

void Cat24cx::handleResponse(const I2cBus::Transaction * transaction)
{
    (void)transaction;
}

void Cat24cx::createRequest(I2cBus * bus)
{
    (void)bus;
}

}  // namespace i2c
}  // namespace driver
