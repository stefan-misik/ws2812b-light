/**
 * @file
 */

#ifndef DRIVER_LED_CONTROLLER_HPP_
#define DRIVER_LED_CONTROLLER_HPP_

#include "driver/common.hpp"
#include "led_strip.h"


namespace driver
{

class LedController
{
public:
    static const inline std::size_t BUFFER_LENGTH = 128;

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

    /**
     * @brief Initiate LED strip update process
     *
     * @param[in] led_strip LED Data to use
     *
     * @return Success
     */
    bool update(const AbstractLedStrip * led_strip);

private:
    struct Private;
    struct PrivateStorage
    {
        char data[12 + 4 + (2 * BUFFER_LENGTH) + 12] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    void forcePwm(std::uint8_t value);

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_LED_CONTROLLER_HPP_
