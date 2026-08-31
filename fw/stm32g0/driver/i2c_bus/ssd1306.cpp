#include "driver/i2c_bus/ssd1306.hpp"

#include <cstring>


namespace driver
{
namespace i2c
{

namespace
{

/**
 * @brief SSD1306 command byte prefix: Co=0, D/C#=0 (command mode)
 */
static constexpr std::uint8_t CMD_PREFIX = 0x00;

/**
 * @brief SSD1306 data byte prefix: Co=0, D/C#=1 (data mode)
 */
static constexpr std::uint8_t DATA_PREFIX = 0x40;


/**
 * @brief Initialization command sequences
 *
 * Each sequence is: {length, cmd_byte_0, cmd_byte_1, ...}
 * Sent as I2C write: [address] [0x00 (command prefix)] [cmd bytes...]
 * The command prefix is placed in the transaction's local buffer, and the
 * command payload is in the remote buffer.
 */
struct InitCmd
{
    const std::uint8_t * data;
    std::uint8_t size;
};

static constexpr std::uint8_t INIT_DISPLAY_OFF[] = { 0xAE };
static constexpr std::uint8_t INIT_SET_MUX_RATIO[] = { 0xA8, 63 };
static constexpr std::uint8_t INIT_SET_DISPLAY_OFFSET[] = { 0xD3, 0x00 };
static constexpr std::uint8_t INIT_SET_START_LINE[] = { 0x40 };
static constexpr std::uint8_t INIT_SET_SEG_REMAP[] = { 0xA1 };
static constexpr std::uint8_t INIT_SET_COM_SCAN_DIR[] = { 0xC8 };
static constexpr std::uint8_t INIT_SET_COM_PINS[] = { 0xDA, 0x12 };
static constexpr std::uint8_t INIT_SET_CONTRAST[] = { 0x81, 0x7F };
static constexpr std::uint8_t INIT_ENTIRE_DISPLAY_ON[] = { 0xA4 };
static constexpr std::uint8_t INIT_SET_NORMAL_DISPLAY[] = { 0xA6 };
static constexpr std::uint8_t INIT_SET_CLK_DIV[] = { 0xD5, 0x80 };
static constexpr std::uint8_t INIT_SET_CHARGE_PUMP[] = { 0x8D, 0x14 };
static constexpr std::uint8_t INIT_SET_MEMORY_MODE[] = { 0x20, 0x02 };  // Page addressing mode
static constexpr std::uint8_t INIT_DISPLAY_ON[] = { 0xAF };

static const InitCmd INIT_SEQUENCE[] = {
    { INIT_DISPLAY_OFF,          sizeof(INIT_DISPLAY_OFF) },
    { INIT_SET_CLK_DIV,          sizeof(INIT_SET_CLK_DIV) },
    { INIT_SET_MUX_RATIO,        sizeof(INIT_SET_MUX_RATIO) },
    { INIT_SET_DISPLAY_OFFSET,   sizeof(INIT_SET_DISPLAY_OFFSET) },
    { INIT_SET_START_LINE,       sizeof(INIT_SET_START_LINE) },
    { INIT_SET_CHARGE_PUMP,      sizeof(INIT_SET_CHARGE_PUMP) },
    { INIT_SET_MEMORY_MODE,      sizeof(INIT_SET_MEMORY_MODE) },
    { INIT_SET_SEG_REMAP,        sizeof(INIT_SET_SEG_REMAP) },
    { INIT_SET_COM_SCAN_DIR,     sizeof(INIT_SET_COM_SCAN_DIR) },
    { INIT_SET_COM_PINS,         sizeof(INIT_SET_COM_PINS) },
    { INIT_SET_CONTRAST,         sizeof(INIT_SET_CONTRAST) },
    { INIT_ENTIRE_DISPLAY_ON,    sizeof(INIT_ENTIRE_DISPLAY_ON) },
    { INIT_SET_NORMAL_DISPLAY,   sizeof(INIT_SET_NORMAL_DISPLAY) },
    { INIT_DISPLAY_ON,           sizeof(INIT_DISPLAY_ON) },
};

static constexpr std::size_t INIT_SEQUENCE_COUNT = sizeof(INIT_SEQUENCE) / sizeof(INIT_SEQUENCE[0]);


}  // namespace


void Ssd1306::clear()
{
    std::memset(framebuffer_, 0, FRAMEBUFFER_SIZE);
}

void Ssd1306::setPixel(std::size_t x, std::size_t y, bool on)
{
    if (x >= WIDTH || y >= HEIGHT)
        return;

    const std::size_t page = y / 8;
    const std::uint8_t bit = static_cast<std::uint8_t>(1u << (y % 8));
    const std::size_t index = page * WIDTH + x;

    if (on)
        framebuffer_[index] |= bit;
    else
        framebuffer_[index] &= ~bit;
}


void Ssd1306::handleResponse(const I2cBus::Transaction * transaction)
{
    (void)transaction;
    pending_ = false;
}


void Ssd1306::createRequest(I2cBus * bus)
{
    // Don't create a new request while one is still pending
    if (pending_)
        return;

    I2cBus::Transaction * tr = nullptr;

    switch (state_)
    {
    case State::INIT_CMDS:
    {
        if (init_step_ >= INIT_SEQUENCE_COUNT)
        {
            state_ = State::IDLE;
            dirty_ = true;  // Send initial framebuffer content
            return;
        }

        tr = bus->allocate();
        if (nullptr == tr)
            return;

        const auto & cmd = INIT_SEQUENCE[init_step_];

        // Place command prefix in local buffer, command payload in remote
        auto * local = static_cast<std::uint8_t *>(tr->localBuffer());
        local[0] = CMD_PREFIX;
        tr->write(address(), 1,
                  const_cast<std::uint8_t *>(cmd.data), cmd.size);

        ++init_step_;
        break;
    }

    case State::IDLE:
    {
        if (!dirty_)
            return;

        dirty_ = false;
        current_page_ = 0;
        state_ = State::SET_PAGE_ADDR;
        // Fall through to SET_PAGE_ADDR
    }
    [[fallthrough]];

    case State::SET_PAGE_ADDR:
    {
        tr = bus->allocate();
        if (nullptr == tr)
            return;

        // Set page address and column address for current page:
        //   0xB0 | page  - set page start address
        //   0x00          - set lower column start address to 0
        //   0x10          - set upper column start address to 0
        auto * local = static_cast<std::uint8_t *>(tr->localBuffer());
        local[0] = CMD_PREFIX;
        local[1] = static_cast<std::uint8_t>(0xB0u | current_page_);
        local[2] = 0x00;
        local[3] = 0x10;
        tr->write(address(), 4);

        state_ = State::SEND_DATA;
        break;
    }

    case State::SEND_DATA:
    {
        tr = bus->allocate();
        if (nullptr == tr)
            return;

        // Send one page (128 bytes) of framebuffer data
        auto * local = static_cast<std::uint8_t *>(tr->localBuffer());
        local[0] = DATA_PREFIX;
        tr->write(address(), 1,
                  &framebuffer_[current_page_ * WIDTH], WIDTH);

        ++current_page_;
        if (current_page_ >= PAGES)
            state_ = State::IDLE;
        else
            state_ = State::SET_PAGE_ADDR;
        break;
    }
    }

    if (nullptr != tr)
    {
        pending_ = true;
        bus->enqueue(tr);
    }
}


}  // namespace i2c
}  // namespace driver
