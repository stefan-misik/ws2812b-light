/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_CAT24CX_HPP_
#define DRIVER_I2C_BUS_CAT24CX_HPP_

#include <cstdint>
#include "driver/i2c_bus.hpp"


namespace driver
{
namespace i2c
{

class Cat24cx
{
public:
    I2cBus::Address address() const { return I2cBus::Address(0b1010000); }
    void handleResponse(const I2cBus::Transaction * transaction);
    void createRequest(I2cBus * bus);

private:
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_CAT24CX_HPP_
