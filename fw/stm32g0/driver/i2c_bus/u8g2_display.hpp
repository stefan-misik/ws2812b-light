/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_U8G2_DISPLAY_HPP_
#define DRIVER_I2C_BUS_U8G2_DISPLAY_HPP_

#include <cstdint>
#include "driver/i2c_bus.hpp"
#include "u8g2.h"


namespace driver
{
namespace i2c
{

/**
 * @brief U8g2-based OLED display driver using in-memory frame buffer
 *
 * This driver uses the u8g2 library in full frame buffer mode (F variant)
 * for an SSD1306 128x64 I2C OLED display. The full frame buffer allows
 * arbitrary drawing operations before sending the entire buffer to the
 * display over I2C.
 *
 * Usage example:
 * @code
 *     // Initialize the display
 *     display.initialize(&i2c_bus);
 *
 *     // Draw content
 *     u8g2_t * u8g2 = display.u8g2();
 *     u8g2_ClearBuffer(u8g2);
 *     u8g2_SetFont(u8g2, u8g2_font_6x10_tr);
 *     u8g2_DrawStr(u8g2, 0, 12, "Hello World!");
 *
 *     // Request display update (non-blocking, uses I2C transactions)
 *     display.requestUpdate();
 * @endcode
 */
class U8g2Display
{
public:
    static constexpr std::uint8_t DEFAULT_ADDRESS = 0x3C;
    static constexpr unsigned DISPLAY_WIDTH = 128;
    static constexpr unsigned DISPLAY_HEIGHT = 64;

    U8g2Display() = default;

    /**
     * @brief Initialize the u8g2 display
     *
     * @param bus Pointer to the I2C bus used for communication
     * @param address I2C address of the display (default 0x3C)
     */
    void initialize(I2cBus * bus, std::uint8_t address = DEFAULT_ADDRESS);

    /**
     * @brief Get the u8g2 instance for drawing operations
     *
     * @return Pointer to the u8g2 structure
     */
    u8g2_t * u8g2() { return &u8g2_; }

    /**
     * @brief Get the I2C address of the display
     */
    I2cBus::Address address() const { return address_; }

    /**
     * @brief Handle completed I2C transaction
     *
     * @param transaction Completed transaction
     */
    void handleResponse(const I2cBus::Transaction * transaction);

    /**
     * @brief Create I2C requests for pending display data
     *
     * @param bus I2C bus to use
     */
    void createRequest(I2cBus * bus);

    /**
     * @brief Request a full display update
     *
     * Call this after drawing to the frame buffer to schedule an I2C
     * transfer of the buffer contents to the display.
     */
    void requestUpdate();

    /**
     * @brief Check if a display update is in progress
     */
    bool isUpdateInProgress() const { return update_pending_ || sending_; }

private:
    /** @brief u8g2 byte-level callback for I2C communication */
    static uint8_t byteCb(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void * arg_ptr);
    /** @brief u8g2 GPIO and delay callback */
    static uint8_t gpioDelayCb(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void * arg_ptr);

    u8g2_t u8g2_{};
    I2cBus * bus_ = nullptr;
    I2cBus::Address address_;

    bool update_pending_ = false;
    bool sending_ = false;
    std::uint16_t send_offset_ = 0;
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_U8G2_DISPLAY_HPP_
