#include "driver/i2c_bus/u8g2_display.hpp"

#include <cstring>


namespace driver
{
namespace i2c
{

namespace
{

/**
 * @brief Size of a single page sent over I2C
 *
 * SSD1306 accepts data in pages. Each I2C transfer sends a control byte
 * followed by page data.
 */
constexpr std::size_t PAGE_SIZE = 128;

/** @brief I2C data/command control byte for data */
constexpr std::uint8_t CONTROL_DATA = 0x40;

/** @brief I2C data/command control byte for command */
constexpr std::uint8_t CONTROL_CMD = 0x00;

}  // namespace


void U8g2Display::initialize(I2cBus * bus, std::uint8_t address)
{
    bus_ = bus;
    address_ = I2cBus::Address(address);

    // Set up u8g2 in full frame buffer mode for SSD1306 128x64 I2C
    u8g2_SetupDisplay(
        &u8g2_,
        u8x8_d_ssd1306_128x64_noname,
        u8x8_cad_ssd13xx_fast_i2c,
        byteCb,
        gpioDelayCb
    );
    u8g2_SetupBuffer(
        &u8g2_,
        u8g2_.tile_buf_ptr,  // Will be set by SetupBuffer
        U8G2_FULL_BUFFER_TILE_HEIGHT,
        u8g2_ll_hvline_vertical_top_lsb,
        U8G2_R0
    );

    // Power-up the display via u8g2 (sends init commands)
    u8g2_InitDisplay(&u8g2_);
    u8g2_SetPowerSave(&u8g2_, 0);
    u8g2_ClearBuffer(&u8g2_);
}

void U8g2Display::handleResponse(const I2cBus::Transaction * transaction)
{
    (void)transaction;
    sending_ = false;
}

void U8g2Display::createRequest(I2cBus * bus)
{
    if (sending_)
        return;

    if (!update_pending_)
        return;

    // Get the frame buffer from u8g2
    const std::uint8_t * buffer = u8g2_GetBufferPtr(&u8g2_);
    const std::size_t buffer_size = 8 * u8g2_GetBufferTileWidth(&u8g2_) * u8g2_GetBufferTileHeight(&u8g2_);

    if (send_offset_ >= buffer_size)
    {
        // All pages sent
        update_pending_ = false;
        send_offset_ = 0;
        return;
    }

    auto * transaction = bus->allocate();
    if (nullptr == transaction)
        return;

    // Calculate how much data to send in this transaction
    const std::size_t remaining = buffer_size - send_offset_;
    const std::size_t chunk = (remaining > PAGE_SIZE) ? PAGE_SIZE : remaining;

    // Set control byte in local buffer (indicates data follows)
    auto * local = static_cast<std::uint8_t *>(transaction->localBuffer());
    local[0] = CONTROL_DATA;

    // Write: control byte (local) + pixel data (remote)
    transaction->write(
        address_,
        1,  // local size: control byte
        const_cast<std::uint8_t *>(buffer + send_offset_),
        chunk  // remote size: pixel data
    );

    send_offset_ += chunk;
    sending_ = true;
    bus->enqueue(transaction);
}

void U8g2Display::requestUpdate()
{
    if (!update_pending_)
    {
        update_pending_ = true;
        send_offset_ = 0;
    }
}

uint8_t U8g2Display::byteCb(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void * arg_ptr)
{
    // For the full frame buffer mode with async I2C, the byte-level callback
    // is mainly used during initialization (synchronous command sending).
    // During normal operation, we bypass u8g2's transfer mechanism and send
    // the frame buffer directly via I2C transactions in createRequest().

    (void)u8x8;
    (void)arg_int;
    (void)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
    case U8X8_MSG_BYTE_SET_DC:
    case U8X8_MSG_BYTE_START_TRANSFER:
    case U8X8_MSG_BYTE_END_TRANSFER:
        break;

    case U8X8_MSG_BYTE_SEND:
        // During init, commands are sent synchronously.
        // In this example, init commands are handled by the I2C bus
        // in the application's main loop.
        break;
    }

    return 1;
}

uint8_t U8g2Display::gpioDelayCb(u8x8_t * u8x8, uint8_t msg, uint8_t arg_int, void * arg_ptr)
{
    (void)u8x8;
    (void)arg_int;
    (void)arg_ptr;

    switch (msg)
    {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
        break;

    case U8X8_MSG_DELAY_MILLI:
        // Simple busy-wait delay for initialization
        for (volatile std::uint32_t i = 0; i < arg_int * 1000u; ++i)
            ;
        break;

    case U8X8_MSG_GPIO_I2C_CLOCK:
    case U8X8_MSG_GPIO_I2C_DATA:
        // I2C is handled by the hardware driver, not bit-banged
        break;
    }

    return 1;
}

}  // namespace i2c
}  // namespace driver
