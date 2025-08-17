/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_HPP_
#define DRIVER_I2C_BUS_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>

#include "tools/list.hpp"
#include "tools/hidden.hpp"
#include "driver/common.hpp"


namespace driver
{

class I2cBus
{
public:
    class Address
    {
    private:
        static const inline std::uint16_t INVALID = 0xFFFFu;
        static const inline std::uint16_t VALID_MASK = 0x007Fu;

    public:
        static Address Invalid() { return Address(); }

        Address() = default;
        Address(const Address &) = default;
        Address & operator =(const Address &) = default;

        explicit Address(std::uint16_t value):
            value_(value & VALID_MASK)
        { }

        operator std::uint16_t() const { return value_ & VALID_MASK; }
        bool isValid() const { return INVALID != value_; }

        friend bool operator ==(const Address & lhs, const Address & rhs) { return lhs.value_ == rhs.value_; }
        friend bool operator !=(const Address & lhs, const Address & rhs) { return lhs.value_ != rhs.value_; }

    private:
        std::uint16_t value_ = INVALID;
    };


    class TransactionProcessor;

    /**
     * @brief Object representing single transaction on I2C bus
     */
    class Transaction
    {
    public:
        static const inline std::size_t STORAGE_SIZE = 8;

        enum class Status: std::uint8_t
        {
            DONE,

            FIRST_ERROR,
            ERR_NO_ANSWER = FIRST_ERROR,
            ERR_ARBITRATION_LOST,
            ERR_BUS_ERROR,
        };

        enum class Operation: std::uint8_t
        {
            READ, WRITE,
        };

        /** @brief Check if transaction is empty */
        bool isEmpty() const { return !address_.isValid(); }
        /** @brief Mark transaction as empty */
        void reset() { address_ = Address::Invalid(); }

        void * localBuffer() { return local_; }

        void read(Address address, std::size_t local_size, void * remote = nullptr, std::size_t remote_size = 0)
        {
            address_ = address;
            operation_ = Operation::READ;
            remote_size_ = remote_size;
            local_size_ = local_size;
            remote_ = reinterpret_cast<std::uintptr_t>(remote);
        }

        void write(Address address, std::size_t local_size, void * remote = nullptr, std::size_t remote_size = 0)
        {
            address_ = address;
            operation_ = Operation::WRITE;
            remote_size_ = remote_size;
            local_size_ = local_size;
            remote_ = reinterpret_cast<std::uintptr_t>(remote);
        }

        std::pair<const void *, std::size_t> local() const
        {
            return {local_, local_size_};
        }

        std::pair<const void *, std::size_t> remote() const
        {
            return {reinterpret_cast<const void *>(remote_), remote_size_};
        }

        void setAddress(Address address) { address_ = address; }
        const Address & address() const { return address_; }
        Operation operation() const { return operation_; }
        Status status() const { return status_; }

    private:
        Address address_;
        Operation operation_;
        Status status_;

        std::uint16_t remote_size_;
        std::uint8_t local_size_;

        std::uintptr_t remote_;
        alignas (void *)
        char local_[STORAGE_SIZE];

        friend TransactionProcessor;
    };


    /** @brief Object used to represent transaction within linked list */
    class TransactionNode: public Transaction, public List::Node
    {
    };


    I2cBus();
    ~I2cBus();

    /**
     * @brief Initialize I2C Bus driver
     *
     * @param i2c_id I2C hardware to use
     * @param dma_channel_id DMA Channel used for I2C Bus transmission
     * @param transaction_cnt Number of transaction buffers to allocate
     *
     * @return Success
     */
    bool initialize(I2cId i2c_id, DmaChannelId dma_channel_id, std::size_t transaction_cnt);

    /**
     * @brief Get a new transaction
     *
     * @return New transaction
     * @retval nullptr no transactions available in the pool
     */
    [[nodiscard]]
    Transaction * allocate()
    {
        return static_cast<TransactionNode *>(free_.pop());
    }

    /**
     * @brief Release transaction object
     *
     * @param transaction Transaction to be released
     */
    void release(Transaction * transaction)
    {
        transaction->reset();
        releaseRaw(transaction);
    }

    /**
     * @brief Get processed transaction
     *
     * @return Finished transaction
     * @retval nullptr no processed transaction found
     */
    [[nodiscard]]
    Transaction * get();

    /**
     * @brief Enqueue a transaction to be processed
     *
     * @param transaction Transaction to pre processed
     */
    void enqueue(Transaction * transaction);

    /**
     * @brief Call from I2C interrupt
     */
    void maybeHandleI2cInterrupt();

private:
    struct Private;
    Hidden<Private, 20 + (sizeof(void *) * (1 + (2 * 2))) + 4> p_;

    void releaseRaw(Transaction * transaction)
    {
        free_.push(static_cast<TransactionNode *>(transaction));
    }

    List free_;
};


}  // namespace driver

#endif  // DRIVER_I2C_BUS_HPP_
