#include "buttons.h"

#include <avr/io.h>

namespace
{

class IrData
{
public:
    void append(bool bit)
    {
        if (4 == byte_)
            return;

        {
            uint8_t datuum = data_[byte_];
            datuum >>= 1;
            if (bit)
                datuum |= 0x80;
            data_[byte_] = datuum;
        }

        if (7 == bit_)
        {
            bit_ = 0;
            ++byte_;
        }
        else
            ++bit_;
    }

    bool isValid() const
    {
        if (data_[0] != static_cast<uint8_t>(~(data_[1])))
            return false;
        if (data_[2] != static_cast<uint8_t>(~(data_[3])))
            return false;
        return true;
    }

    bool isDone() const { return 4 == byte_; }

    uint8_t address() const { return data_[0]; }
    uint8_t command() const { return data_[2]; }

private:
    uint8_t data_[4];

    uint8_t byte_ = 0;
    uint8_t bit_ = 0;
};


using ButtonId = Buttons::ButtonId;

inline ButtonId classifyButton(uint16_t keypad_value)
{
    if (keypad_value < 73u)
        return ButtonId::NONE;
    else if (keypad_value <= 219u)
        return ButtonId::O_BTN;
    else if (keypad_value <= 364u)
        return ButtonId::LEFT;
    else if (keypad_value <= 501u)
        return ButtonId::UP;
    else if (keypad_value <= 637u)
        return ButtonId::DOWN;
    else if (keypad_value <= 793u)
        return ButtonId::RIGHT;
    else if (keypad_value <= 951u)
        return ButtonId::X_BTN;

    return ButtonId::NONE;
}


/**
 * @brief Time-precise function waiting for certain logic level to be detected
 *
 * @param level Level to obtain
 * @param timeout Timeout to wait
 *
 * @return Time it took in ~176 us increments
 * @retval 0xFF Timeout occurred
 */
__attribute((noinline))
uint8_t waitForIRLevel(bool level, uint8_t timeout)
{
    uint16_t duration = 0;

    uint8_t pin_value;
    asm volatile (
        "rjmp .ir_wait_L2\n"
        ".ir_wait_L4:\n"
        "\t"    "subi %A[duration],-1\n"  // 1 CLK
        "\t"    "sbci %B[duration],-1\n"  // 1 CLK
        "\t"    "cp %[timeout],%B[duration]\n"  // 1 CLK
        "\t"    "breq .ir_wait_L5\n"  // 1 CLK (not taken branch)
        ".ir_wait_L2:\n"
        "\t"    "in %[pin_value],%[pinr]\n"  // 1 CLK
        "\t"    "bst %[pin_value],%[pinb]\n"  // 1 CLK
        "\t"    "clr %[pin_value]\n"  // 1 CLK
        "\t"    "bld %[pin_value],0\n"  // 1 CLK
        "\t"    "cpse %[level],%[pin_value]\n"  // 1 CLK (not skipping)
        "\t"    "rjmp .ir_wait_L4\n"  // 2 CLK
        "\t"    "rjmp .ir_wait_exit\n"
        ".ir_wait_L5:\n"
        "\t"    "ldi %B[duration],lo8(-1)\n"
        ".ir_wait_exit:\n"
        : [duration] "+r" (duration), [pin_value] "=&r" (pin_value)
        : [level] "r" (level), [timeout] "r" (timeout),
          [pinr] "I" (_SFR_IO_ADDR(PINB)), [pinb] "I" (PINB0)
    );

    return duration >> 8;
}

inline bool readIr()
{
    return 0 != (PINB & (1 << PINB0));
}

inline ButtonId receiveIrCmd()
{
    if (true == readIr())
        return ButtonId::NONE;

    // Initial burst
    if (0xFF == waitForIRLevel(true, 53))
        return ButtonId::NONE;

    // Initial pause
    {
        const uint8_t duration = waitForIRLevel(false, 27);
        if (0xFF == duration || duration < 10)
            return ButtonId::NONE;
        if (duration < 18)
        {
            // Repeated button
            waitForIRLevel(true, 4);
            return ButtonId::NONE;
        }
    }

    IrData data;
    while (!data.isDone())
    {
        if (0xFF == waitForIRLevel(true, 4))
            return ButtonId::NONE;

        const uint8_t duration = waitForIRLevel(false, 11);
        if (0xFF == duration)
            return ButtonId::NONE;

        data.append(duration > 7);
    }
    waitForIRLevel(true, 4);

    if (!data.isValid())
        return ButtonId::NONE;

    switch (data.command())
    {
    case 0b00011000: return ButtonId::UP;
    case 0b01011010: return ButtonId::RIGHT;
    case 0b01010010: return ButtonId::DOWN;
    case 0b00001000: return ButtonId::LEFT;
    case 0b00010110: return ButtonId::O_BTN;
    case 0b00001101: return ButtonId::X_BTN;
    }

    return ButtonId::NONE;
}

}  // namespace

void Buttons::initialize()
{
    current_button_ = ButtonId::NONE;

    // Use PB0 exclusively as IR input
    DDRB &= ~((1 << DDB0));
    PORTB |= ((1 << DDB0));  // Enable pull-up
}

uint8_t Buttons::run(uint16_t keypad_value)
{
    const auto button = classifyButton(keypad_value);

    {
        const auto ir_btn = receiveIrCmd();
        if (ButtonId::NONE != ir_btn)
        {
            filter_.reset();
            current_button_ = ir_btn;
            return ButtonFilter::PRESS;
        }
    }

    bool button_state = false;
    if (button == current_button_)
    {
        button_state = true;
    }
    else
    {
        if (0 == (filter_.state() & ButtonFilter::PRESSED))
        {
            filter_.reset();
            current_button_ = button;
            button_state = true;
        }
    }

    if (ButtonId::NONE == current_button_)
        return 0;
    filter_.updateButton(button_state);

    return filter_.state();
}
