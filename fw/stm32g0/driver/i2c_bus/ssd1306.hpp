/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_SSD1306_HPP_
#define DRIVER_I2C_BUS_SSD1306_HPP_

#include <cstdint>
#include <cstddef>
#include "driver/i2c_bus.hpp"


namespace driver
{
namespace i2c
{

/**
 * @brief Non-blocking driver for SSD1306 128x64 I2C OLED display
 *
 * Uses a state machine to send initialization commands and framebuffer data
 * over the I2C bus without blocking.
 */
class Ssd1306
{
public:
    static constexpr std::size_t WIDTH = 128;
    static constexpr std::size_t HEIGHT = 64;
    static constexpr std::size_t PAGES = HEIGHT / 8;
    static constexpr std::size_t FRAMEBUFFER_SIZE = WIDTH * PAGES;

    I2cBus::Address address() const { return I2cBus::Address(0x3C); }

    /**
     * @brief Handle completed I2C transaction response
     *
     * @param transaction Completed transaction
     */
    void handleResponse(const I2cBus::Transaction * transaction);

    /**
     * @brief Create next I2C request if state machine needs to proceed
     *
     * @param bus I2C bus to enqueue transactions on
     */
    void createRequest(I2cBus * bus);

    /**
     * @brief Get pointer to the framebuffer
     *
     * The framebuffer is organized in pages (8 rows per page), each page
     * containing 128 bytes (one per column). Each byte represents 8 vertical
     * pixels, with the LSB being the topmost pixel.
     *
     * @return Pointer to the framebuffer
     */
    std::uint8_t * framebuffer() { return framebuffer_; }
    const std::uint8_t * framebuffer() const { return framebuffer_; }

    /**
     * @brief Mark framebuffer as dirty, triggering a transfer on next
     *        createRequest cycle
     */
    void markDirty() { dirty_ = true; }

    /**
     * @brief Clear the entire framebuffer
     */
    void clear();

    /**
     * @brief Set a single pixel
     *
     * @param x X coordinate (0..127)
     * @param y Y coordinate (0..63)
     * @param on true to set, false to clear
     */
    void setPixel(std::size_t x, std::size_t y, bool on = true);

private:
    enum class State : std::uint8_t
    {
        INIT_CMDS,       ///< Sending initialization command sequences
        IDLE,            ///< Initialization done, waiting for dirty flag
        SET_PAGE_ADDR,   ///< Setting page addressing range
        SEND_DATA,       ///< Sending framebuffer data page by page
    };

    State state_ = State::INIT_CMDS;
    std::uint8_t init_step_ = 0;
    std::uint8_t current_page_ = 0;
    bool dirty_ = false;
    bool pending_ = false;

    std::uint8_t framebuffer_[FRAMEBUFFER_SIZE] = {};
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_SSD1306_HPP_
