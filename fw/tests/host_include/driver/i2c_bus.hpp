#ifndef TEST_HOST_DRIVER_I2C_BUS_HPP_
#define TEST_HOST_DRIVER_I2C_BUS_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <utility>

namespace driver
{
// Minimal API used by the real Cat24cx. Queue ownership and transaction
// lifetimes match the hardware-facing API; execution is controlled by tests.
class I2cBus
{
public:
    class Address
    {
    public:
        Address() = default;
        explicit Address(std::uint16_t value): value_(value & 0x7fu) { }
        friend bool operator ==(Address lhs, Address rhs) { return lhs.value_ == rhs.value_; }
        friend bool operator !=(Address lhs, Address rhs) { return !(lhs == rhs); }
    private:
        std::uint16_t value_ = 0xffffu;
    };

    class Transaction
    {
    public:
        enum class Status: std::uint8_t { DONE, ERR_NO_ANSWER, ERR_ARBITRATION_LOST, ERR_BUS_ERROR };
        enum class Operation: std::uint8_t { READ, WRITE };

        void * localBuffer() { return local_.data(); }
        void read(Address address, std::size_t local_size, void * remote = nullptr, std::size_t remote_size = 0)
        {
            setup(address, Operation::READ, local_size, remote, remote_size);
        }
        void write(Address address, std::size_t local_size, void * remote = nullptr, std::size_t remote_size = 0)
        {
            setup(address, Operation::WRITE, local_size, remote, remote_size);
        }
        Address address() const { return address_; }
        Operation operation() const { return operation_; }
        Status status() const { return status_; }
        std::uint8_t opaque() const { return opaque_; }
        void setOpaque(std::uint8_t value) { opaque_ = value; }
        std::pair<const void *, std::size_t> local() const { return {local_.data(), local_size_}; }
        std::pair<const void *, std::size_t> remote() const { return {remote_, remote_size_}; }
        void * mutableRemote() const { return remote_; }
        void setStatus(Status value) { status_ = value; }

    private:
        void setup(Address address, Operation operation, std::size_t local_size, void * remote, std::size_t remote_size)
        {
            address_ = address;
            operation_ = operation;
            local_size_ = local_size;
            remote_ = remote;
            remote_size_ = remote_size;
        }
        std::array<std::uint8_t, 8> local_{};
        Address address_{};
        Operation operation_ = Operation::WRITE;
        Status status_ = Status::DONE;
        std::uint8_t opaque_ = 0;
        std::size_t local_size_ = 0;
        void * remote_ = nullptr;
        std::size_t remote_size_ = 0;
        bool allocated_ = false;
        friend class I2cBus;
    };

    Transaction * allocate()
    {
        for (auto & transaction : pool_)
        {
            if (!transaction.allocated_)
            {
                transaction = Transaction{};
                transaction.allocated_ = true;
                return &transaction;
            }
        }
        return nullptr;
    }
    void release(Transaction * transaction) { transaction->allocated_ = false; }
    void enqueue(Transaction * transaction) { pending_.push_back(transaction); }
    std::size_t pendingCount() const { return pending_.size(); }
    Transaction * front() const { return pending_.empty() ? nullptr : pending_.front(); }
    Transaction * pop()
    {
        auto * transaction = front();
        if (transaction != nullptr)
            pending_.pop_front();
        return transaction;
    }
    bool nextIsRead() const
    {
        return pending_.size() > 1 && pending_[1]->operation() == Transaction::Operation::READ;
    }

private:
    std::array<Transaction, 4> pool_{};
    std::deque<Transaction *> pending_;
};
}  // namespace driver

#endif