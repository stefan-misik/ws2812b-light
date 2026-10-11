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
 * Each command owns its payload in a fixed-size buffer. The longest command
 * sequence currently used sets the page and column addresses in three bytes.
 * Sent as I2C write: [address] [0x00 (command prefix)] [cmd bytes...]
 * The command prefix is placed in the transaction's local buffer, and the
 * command payload is in the remote buffer.
 */
struct Command
{
    std::uint8_t data[3];
    std::uint8_t size;
};

enum class ChargePump: std::uint8_t
{
    DISABLED = 0x10,
    ENABLED = 0x14,
};

/** Set Display Clock Divide Ratio / Oscillator Frequency (0xD5) parameter. */
enum class ClockDivideRatio: std::uint8_t
{
    DIVIDE_BY_1 = 0x00,
    DIVIDE_BY_2 = 0x01,
    DIVIDE_BY_3 = 0x02,
    DIVIDE_BY_4 = 0x03,
    DIVIDE_BY_5 = 0x04,
    DIVIDE_BY_6 = 0x05,
    DIVIDE_BY_7 = 0x06,
    DIVIDE_BY_8 = 0x07,
    DIVIDE_BY_9 = 0x08,
    DIVIDE_BY_10 = 0x09,
    DIVIDE_BY_11 = 0x0A,
    DIVIDE_BY_12 = 0x0B,
    DIVIDE_BY_13 = 0x0C,
    DIVIDE_BY_14 = 0x0D,
    DIVIDE_BY_15 = 0x0E,
    DIVIDE_BY_16 = 0x0F,
};

enum class OscillatorFrequency: std::uint8_t
{
    FREQ_0 = 0x00,
    FREQ_1 = 0x01,
    FREQ_2 = 0x02,
    FREQ_3 = 0x03,
    FREQ_4 = 0x04,
    FREQ_5 = 0x05,
    FREQ_6 = 0x06,
    FREQ_7 = 0x07,
    DEFAULT = 0x08,
    FREQ_9 = 0x09,
    FREQ_10 = 0x0A,
    FREQ_11 = 0x0B,
    FREQ_12 = 0x0C,
    FREQ_13 = 0x0D,
    FREQ_14 = 0x0E,
    FREQ_15 = 0x0F,
};

enum class MemoryMode: std::uint8_t
{
    HORIZONTAL = 0x00,
    VERTICAL = 0x01,
    PAGE = 0x02,
};

enum class SegmentRemap: std::uint8_t
{
    COLUMN_0 = 0xA0,
    COLUMN_127 = 0xA1,
};

enum class ComScanDirection: std::uint8_t
{
    NORMAL = 0xC0,
    REMAPPED = 0xC8,
};

enum class ComPins: std::uint8_t
{
    SEQUENTIAL = 0x02,
    ALTERNATIVE = 0x12,
    SEQUENTIAL_REMAP = 0x22,
    ALTERNATIVE_REMAP = 0x32,
};

enum class EntireDisplay: std::uint8_t
{
    USE_RAM = 0xA4,
    IGNORE_RAM = 0xA5,
};

enum class DisplayMode: std::uint8_t
{
    NORMAL = 0xA6,
    INVERTED = 0xA7,
};

enum class DisplayPower: std::uint8_t
{
    OFF = 0xAE,
    ON = 0xAF,
};

constexpr inline Command setClockDivider(ClockDivideRatio divide_ratio,
                                         OscillatorFrequency frequency)
{
    const auto value = static_cast<std::uint8_t>(
        (static_cast<std::uint8_t>(frequency) << 4)
        | static_cast<std::uint8_t>(divide_ratio));
    return { { 0xD5, value }, 2 };
}
constexpr inline Command setMuxRatio(std::uint8_t ratio) { return { { 0xA8, ratio }, 2 }; }
constexpr inline Command setDisplayOffset(std::uint8_t offset) { return { { 0xD3, offset }, 2 }; }
constexpr inline Command setStartLine(std::uint8_t line) { return { { static_cast<std::uint8_t>(0x40u | (line & 0x3Fu)) }, 1 }; }
constexpr inline Command setChargePump(ChargePump setting) { return { { 0x8D, static_cast<std::uint8_t>(setting) }, 2 }; }
constexpr inline Command setMemoryMode(MemoryMode mode) { return { { 0x20, static_cast<std::uint8_t>(mode) }, 2 }; }
constexpr inline Command setSegmentRemap(SegmentRemap remap) { return { { static_cast<std::uint8_t>(remap) }, 1 }; }
constexpr inline Command setComScanDirection(ComScanDirection direction) { return { { static_cast<std::uint8_t>(direction) }, 1 }; }
constexpr inline Command setComPins(ComPins configuration) { return { { 0xDA, static_cast<std::uint8_t>(configuration) }, 2 }; }
constexpr inline Command setContrast(std::uint8_t contrast) { return { { 0x81, contrast }, 2 }; }
constexpr inline Command setEntireDisplay(EntireDisplay setting) { return { { static_cast<std::uint8_t>(setting) }, 1 }; }
constexpr inline Command setDisplayMode(DisplayMode mode) { return { { static_cast<std::uint8_t>(mode) }, 1 }; }
constexpr inline Command setDisplayPower(DisplayPower power) { return { { static_cast<std::uint8_t>(power) }, 1 }; }

/**
 * @brief Set the page and column start address in page-addressing mode.
 */
constexpr inline Command setPageAndColumnAddress(std::uint8_t page,
                                                  std::uint8_t column)
{
    return { { static_cast<std::uint8_t>(0xB0u | (page & 0x07u)),
               static_cast<std::uint8_t>(column & 0x0Fu),
               static_cast<std::uint8_t>(0x10u | ((column >> 4) & 0x0Fu)) }, 3 };
}

static constexpr Command INIT_SEQUENCE[] = {
    setDisplayPower(DisplayPower::OFF),
    setClockDivider(ClockDivideRatio::DIVIDE_BY_1,
                    OscillatorFrequency::DEFAULT),
    setMuxRatio(63),
    setDisplayOffset(0x00),
    setStartLine(0),
    setChargePump(ChargePump::ENABLED),
    setMemoryMode(MemoryMode::PAGE),
    setSegmentRemap(SegmentRemap::COLUMN_127),
    setComScanDirection(ComScanDirection::REMAPPED),
    setComPins(ComPins::ALTERNATIVE),
    setContrast(0x7F),
    setEntireDisplay(EntireDisplay::USE_RAM),
    setDisplayMode(DisplayMode::NORMAL),
    setDisplayPower(DisplayPower::ON),
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

        const auto cmd = setPageAndColumnAddress(current_page_, 0);
        auto * local = static_cast<std::uint8_t *>(tr->localBuffer());
        local[0] = CMD_PREFIX;
        std::memcpy(&local[1], cmd.data, cmd.size);
        tr->write(address(), cmd.size + 1);

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
