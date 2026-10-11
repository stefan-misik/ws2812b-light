/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_SSD1306_HPP_
#define DRIVER_I2C_BUS_SSD1306_HPP_

#include <cstdint>
#include <cstddef>

#include <u8g2.h>

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
     * @brief Configure u8g2 to render directly into this driver's framebuffer.
     */
    void initialize();

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
    * @return Pointer to the u8g2-managed framebuffer, or nullptr before
    *         initialize() has completed
     */
    std::uint8_t * framebuffer() { return framebuffer_; }
    const std::uint8_t * framebuffer() const { return framebuffer_; }

    /**
     * @brief Get the u8g2 drawing context.
     *
     * Draw with the regular u8g2 APIs, then call sendBuffer() to schedule the
     * resulting framebuffer for asynchronous transfer.
     */
    u8g2_t * u8g2() { return &u8g2_; }

    /** @brief Schedule the u8g2 framebuffer for asynchronous transfer. */
    void sendBuffer() { markDirty(); }

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

    u8g2_t u8g2_ = {};
    std::uint8_t * framebuffer_ = nullptr;

    static std::uint8_t u8g2ByteCallback(u8x8_t * u8x8, std::uint8_t message,
                                          std::uint8_t argument, void * data);
    static std::uint8_t u8g2GpioAndDelayCallback(u8x8_t * u8x8,
                                                  std::uint8_t message,
                                                  std::uint8_t argument,
                                                  void * data);
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_SSD1306_HPP_
