/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_HPP_
#define DRIVER_I2C_BUS_HPP_

#include <cstddef>
#include <cstdint>

#include "tools/list.hpp"
#include "tools/hidden.hpp"
#include "driver/common.hpp"


namespace driver
{

class I2cBus
{
public:
    /**
     * @brief Object representing single transaction on I2C bus
     */
    class Transaction
    {
    public:
        static const inline std::size_t STORAGE_SIZE = 32;

        static const inline std::uint16_t INVALID_ADDRESS = 0xFFFF;

        enum class Status: std::uint8_t
        {
            DONE,

            FIRST_ERROR,
            ERR_NO_ANSWER = FIRST_ERROR,
        };

        enum class Type: std::uint8_t
        {
            READ, WRITE,
            READ_REMOTE, WRITE_REMOTE,
        };

        /** @brief Check if transaction is empty */
        bool isEmpty() const { return INVALID_ADDRESS == address_; }
        /** @brief Mark transaction as empty */
        void reset() { address_ = INVALID_ADDRESS; }

        std::uint16_t address() const { return address_; }
        Status status() const { return status_; }

    private:
        std::uint16_t address_ = INVALID_ADDRESS;
        Status status_;
        Type type_;

        std::size_t size_;
        alignas (void *)
        char data_[STORAGE_SIZE];
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

    void tick();

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
    Hidden<Private, 20 + (sizeof(void *) * (1 + (2 * 2)))> p_;

    void releaseRaw(Transaction * transaction)
    {
        free_.push(static_cast<TransactionNode *>(transaction));
    }

    List free_;
};


}  // namespace driver

#endif  // DRIVER_I2C_BUS_HPP_
