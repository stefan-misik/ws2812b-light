/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_HPP_
#define DRIVER_I2C_BUS_HPP_

#include <cstddef>
#include <cstdint>

#include "tools/hidden.hpp"
#include "tools/atomic_list.hpp"
#include "driver/common.hpp"


namespace driver
{

class I2cBus
{
public:
    class Transaction
    {
    public:
        static const inline std::size_t STORAGE_SIZE = 32;
         enum class Type: std::uint8_t
         {
             EMPTY = 0u,
             READ, WRITE,
             READ_REMOTE, WRITE_REMOTE,
         };

         enum class Result: std::uint8_t
         {
             DONE,

             FIRST_ERROR,
             ERR_NO_ANSWRT = FIRST_ERROR,
         };

    private:
        Type type_;
        Result result_;
        std::uint16_t address_;

        std::size_t size_;
        alignas (void *)
        char data_[STORAGE_SIZE];
    };


    I2cBus();
    ~I2cBus();

    /**
     * @brief Initialize I2C Bus driver
     *
     * @param i2c_id I2C hardware to use
     * @param dma_channel_id_rx DMA Channel used for I2C Bus reception
     * @param dma_channel_id_tx DMA Channel used for I2C Bus transmission
     *
     * @return Success
     */
    bool initialize(I2cId i2c_id, DmaChannelId dma_channel_id_rx, DmaChannelId dma_channel_id_tx);

    void tick();

private:
    struct Private;
    Hidden<Private, 16> p_;
};

}  // namespace driver

#endif  // DRIVER_I2C_BUS_HPP_
