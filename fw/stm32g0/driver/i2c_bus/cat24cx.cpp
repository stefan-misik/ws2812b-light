#include "driver/i2c_bus/cat24cx.hpp"


namespace driver
{
namespace i2c
{

void Cat24cx::handleResponse(const I2cBus::Transaction * transaction)
{
    waiting_ -= 1;
    if (0 == waiting_)
        address_ = (address_ + sizeof(buffer_) + I2cBus::Transaction::STORAGE_SIZE) & 0x7FFFu;
    (void)transaction;
}

void Cat24cx::createRequest(I2cBus * bus)
{
    if (0 == waiting_)
    {
        auto * new_transaction = bus->allocate();
        if (nullptr != new_transaction)
        {
            char * buff = static_cast<char *>(new_transaction->localBuffer());
            buff[0] = address_ >> 8;
            data_[0] = address_;
            new_transaction->write(address(), 1, data_, 1);
            bus->enqueue(new_transaction);
            waiting_ += 1;
        }

        new_transaction = bus->allocate();
        if (nullptr != new_transaction)
        {
            new_transaction->read(address(), I2cBus::Transaction::STORAGE_SIZE, buffer_, sizeof(buffer_));
            bus->enqueue(new_transaction);
            waiting_ += 1;
        }
    }
}

}  // namespace i2c
}  // namespace driver
