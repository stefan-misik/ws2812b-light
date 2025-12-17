/**
 * @file
 */

#ifndef DRIVER_LED_CONTROLLER_HPP_
#define DRIVER_LED_CONTROLLER_HPP_

#include "tools/polymorphic_storage.hpp"
#include "tools/hidden.hpp"
#include "driver/common.hpp"
#include "led_strip.hpp"
#include "app/led_correction.hpp"


namespace driver
{

class LedController
{
public:
    /**
     * @brief Length of the intermediate DMA buffer in data bytes
     */
    static const inline std::size_t BUFFER_HALF_LENGTH = 16;

    static const inline std::size_t MAX_DATA_LENGTH = 300;

    static const inline std::uint32_t DEFAULT_INTENSITY = 0x60;

    enum class LedOrder
    {
        ORDER_RGB = 0,
        ORDER_GRB,
    };

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
    bool initialize(TimerId tim_id, uint8_t channel_id, DmaChannelId dma_channel_id);

    /**
     * @brief Initiate LED strip update process
     *
     * @param[in] led_strip LED Data to use
     *
     * @return Success
     */
    bool update(const AbstractLedStrip * led_strip);

    /**
     * @brief Handle the DMA interrupt
     */
    void maybeHandleDmaInterrupt();

    /**
     * @brief Configure LED driver
     *
     * @param order Order of LED components
     * @param intensity Intensity of the LEDs (256 ~ 100%)
     */
    void configure(LedOrder order, std::uint32_t intensity = DEFAULT_INTENSITY);

private:
    PolymorphicStorage<LedCorrection, 32> correction_;

    struct Private;
    Hidden<Private, 12 + 4 + (BUFFER_HALF_LENGTH * 16) + 4 + MAX_DATA_LENGTH> p_;
};

}  // driver


#endif  // DRIVER_LED_CONTROLLER_HPP_
