/**
 * @file
 */

#ifndef DRIVER_LED_CONTROLLER_HPP_
#define DRIVER_LED_CONTROLLER_HPP_

#include "driver/common.hpp"


namespace driver
{

class LedController
{
public:
    static bool initializeTimer(TimerId tim_id);
    static bool startTimer(TimerId tim_id);

    LedController();
    ~LedController();

    /**
     * @brief Initialize  current instance for specified timer and channel ID
     *
     * @param tim_id Timer whose channel to initialize (it already needs to be initialized)
     * @param channel_id Channel to associate the Led Controller instance with
     * @param dma_channel_id ID of the DMA channel to use
     *
     * @return Success
     */
    bool initialize(TimerId tim_id, uint8_t channel_id, std::uint8_t dma_channel_id);

    void forcePwm(std::uint16_t value);

private:
    struct Private;
    struct PrivateStorage
    {
        char data[12] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_LED_CONTROLLER_HPP_
