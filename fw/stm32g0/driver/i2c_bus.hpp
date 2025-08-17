/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_HPP_
#define DRIVER_I2C_BUS_HPP_

#include <cstddef>
#include <cstdint>

#include "tools/hidden.hpp"
#include "driver/common.hpp"


namespace driver
{

class I2cBus
{
public:
    class Data
    {
    public:
        static const inline std::size_t LOCAL_SIZE = 32;


    private:
        struct Remote
        {
            void * data;
            std::size_t length;
        };

        std::size_t size;
        alignas (void *)
        char data[LOCAL_SIZE];
    };

    class II2cDevice
    {
    public:
        virtual bool exchange(Data * data) = 0;
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

private:
    struct Private;
    Hidden<Private, 16> p_;
};

}  // namespace driver

#endif  // DRIVER_I2C_BUS_HPP_
