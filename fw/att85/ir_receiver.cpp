#include "ir_receiver.h"

#include <avr/io.h>
#include <avr/interrupt.h>

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


struct IrCmd
{
    static IrCmd Invalid() { return {0xFF, 0x00}; }
    static IrCmd Repeat() { return {0xFE, 0x00}; }

    uint8_t address;
    uint8_t command;

    bool isInvalid() const { return 0xFF == address; }
    bool isRepeat() const { return 0xFE == address; }
};


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

inline IrCmd receiveCommand()
{
    // Initial burst
    if (0xFF == waitForIRLevel(true, 53))
        return IrCmd::Invalid();

    // Initial pause
    {
        const uint8_t duration = waitForIRLevel(false, 27);
        if (0xFF == duration || duration < 10)
            return IrCmd::Invalid();
        if (duration < 18)
        {
            // Repeated button
            waitForIRLevel(true, 4);
            return IrCmd::Repeat();
        }
    }

    IrData data;
    while (!data.isDone())
    {
        if (0xFF == waitForIRLevel(true, 4))
            return IrCmd::Invalid();

        const uint8_t duration = waitForIRLevel(false, 11);
        if (0xFF == duration)
            return IrCmd::Invalid();

        data.append(duration > 7);
    }
    waitForIRLevel(true, 4);

    if (!data.isValid())
        return IrCmd::Invalid();

    return {data.address(), data.command()};
}

inline IrReceiver::ButtonId decodeButton(IrCmd ir_cmd)
{
    using ButtonId = IrReceiver::ButtonId;

    switch (ir_cmd.address)
    {
    case 0x00:
        switch (ir_cmd.command)
        {
        case 0b00011000: return ButtonId::UP;
        case 0b01011010: return ButtonId::RIGHT;
        case 0b01010010: return ButtonId::DOWN;
        case 0b00001000: return ButtonId::LEFT;
        case 0b00010110: return ButtonId::O_BTN;
        case 0b00001101: return ButtonId::X_BTN;
        }
        break;

    case 0x01:
        switch (ir_cmd.command)
        {
        case 0x16: return ButtonId::UP;
        case 0x50: return ButtonId::RIGHT;
        case 0x1A: return ButtonId::DOWN;
        case 0x51: return ButtonId::LEFT;
        case 0x13: return ButtonId::O_BTN;
        case 0x19: return ButtonId::X_BTN;
        }
        break;
    }
    return ButtonId::NONE;
}

}  // namespace

void IrReceiver::initialize()
{
    // Use PB0 exclusively as IR input
    DDRB &= ~((1 << DDB0));
    PORTB |= ((1 << DDB0));  // Enable pull-up
}

uint8_t IrReceiver::run()
{
    if (true == readIr())
        return 0;

    IrCmd command;
    {
        volatile uint8_t old_sreg = SREG;
        cli();
        command = receiveCommand();
        SREG = old_sreg;
    }

    if (command.isInvalid())
        return Status::RECEIVED;
    else if (command.isRepeat())
    {
        if (repeat_count_ == REPEAT_SKIP)
        {
            repeat_count_ = 0;
            return Status::RECEIVED | Status::PRESS;
        }
        ++repeat_count_;
        return Status::RECEIVED;
    }

    current_button_ = decodeButton(command);
    repeat_count_ = 0;

    return Status::RECEIVED | Status::PRESS;
}
