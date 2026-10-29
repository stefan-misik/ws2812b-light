#include "driver/i2c_bus_fake.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>


extern "C"
{
std::uint32_t test_i2c_primask = 0;
}

namespace
{

using Transaction = driver::I2cBus::Transaction;

std::vector<test_i2c::EnqueueRecord> records;
Transaction::Status next_status = Transaction::Status::DONE;

}  // namespace


namespace driver
{

class I2cBus::TransactionProcessor
{
public:
    static void setStatus(Transaction * transaction, Transaction::Status status)
    {
        transaction->status_ = status;
    }
};

struct I2cBus::Private
{
    static const inline std::size_t STORAGE_SIZE = 20 + (sizeof(void *) * (1 + (2 * 2))) + 4;

    List pending;
    List done;
    std::unique_ptr<TransactionNode[]> transactions;
    std::array<std::byte, STORAGE_SIZE - (2 * sizeof(List)) - sizeof(transactions)> padding{};
};

I2cBus::I2cBus() = default;
I2cBus::~I2cBus() = default;

bool I2cBus::initialize(I2cId, DmaChannelId, std::size_t transaction_cnt)
{
    auto & priv = *p_;
    priv.transactions.reset(new (std::nothrow) TransactionNode[transaction_cnt]);
    if (!priv.transactions)
        return false;

    for (std::size_t i = 0; i < transaction_cnt; ++i)
        free_.push(&priv.transactions[i]);
    return true;
}

void I2cBus::enqueue(Transaction * transaction)
{
    if (transaction->isEmpty())
    {
        release(transaction);
        return;
    }

    records.push_back({transaction, 0 != test_i2c_primask});
    p_->pending.push(static_cast<TransactionNode *>(transaction));
}

Transaction * I2cBus::get()
{
    return static_cast<TransactionNode *>(p_->done.pop());
}

void I2cBus::maybeHandleI2cInterrupt()
{
    auto * const transaction = static_cast<TransactionNode *>(p_->pending.pop());
    if (nullptr == transaction)
        return;

    TransactionProcessor::setStatus(transaction, next_status);
    p_->done.push(transaction);
}

}  // namespace driver


namespace test_i2c
{

void reset()
{
    records.clear();
    test_i2c_primask = 0;
    next_status = Transaction::Status::DONE;
}

const std::vector<EnqueueRecord> & enqueued()
{
    return records;
}

void completeNext(driver::I2cBus & bus, Transaction::Status status)
{
    next_status = status;
    bus.maybeHandleI2cInterrupt();
}

}  // namespace test_i2c