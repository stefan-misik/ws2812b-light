#include "led_controller.h"

#include <avr/io.h>
#include <avr/pgmspace.h>

static constexpr uint16_t INTENSITY = 0x60;
static const uint8_t lookup_table[256] PROGMEM =
{
#include "led_corrections/none.inc"
};

namespace
{

static constexpr uint8_t ZERO_BIT_PULSE_LENGTH = 1;
static constexpr uint8_t ONE_BIT_PULSE_LENGTH = 3;


void blastLeds(const uint8_t * data, const uint8_t * const data_end)
{
    uint16_t tmp_w;
    uint8_t current_byte;
    uint8_t bit_position;
    uint8_t pulse_length;
    uint8_t old_sreg;

    asm volatile (
            "    rjmp  lu_check_all_sent           \n"
            "lu_send_byte:                         \n"
            "    ld    %[current_byte], %a[data]+  \n"
            // Do the lookup table correction
            "    movw  %[tmp_w], %[lookup_table]   \n"
            "    add   %A[tmp_w], %[current_byte]  \n"
            "    adc   %B[tmp_w], __zero_reg__     \n"
            "    lpm   %[current_byte], %a[tmp_w]  \n"

            "    ldi   %[bit_position], 8          \n"

            "lu_send_bit:                          \n"
            "    ldi   %[pulse_length], %[zero_pl] \n"
            "    lsl   %[current_byte]             \n"
            "    brcc  lu_pulse_start              \n"
            "    ldi   %[pulse_length], %[one_pl]  \n"

            "lu_pulse_start:                       \n"
            "    in %[old_sreg], __SREG__          \n"
            "    cli                               \n"
            "    sbi   %[pinr], %[pinb]            \n"

            "lu_wait_pulse_end:                    \n"
            "    subi  %[pulse_length], 1          \n"
            "    brne  lu_wait_pulse_end           \n"
            "    sbi   %[pinr], %[pinb]            \n"
            "    out   __SREG__, %[old_sreg]       \n"

            "lu_check_byte_sent:                   \n"
            "    dec   %[bit_position]             \n"
            "    brne  lu_send_bit                 \n"

            "lu_check_all_sent:                    \n"
            "    cp    %A[data], %A[data_end]      \n"
            "    cpc   %B[data], %B[data_end]      \n"
            "    brne  lu_send_byte               \n"
            : [current_byte] "=&r" (current_byte),
              [old_sreg] "=&r" (old_sreg),
              [bit_position] "=&d" (bit_position),
              [pulse_length] "=&d" (pulse_length),
              [tmp_w] "=&z" (tmp_w),
              [data] "+e" (data)
            : [data_end] "e" (data_end),
              [lookup_table] "w" (lookup_table),
              [zero_pl] "M" (ZERO_BIT_PULSE_LENGTH),
              [one_pl] "M" (ONE_BIT_PULSE_LENGTH),
              [pinr] "I" (_SFR_IO_ADDR(PINB)),
              [pinb] "I" (PINB2)
    );
}

}  // namespace


void LedController::initialize()
{
    // Set output direction to PB2
    PORTB &= ~((1 << DDB2));
    DDRB |= ((1 << DDB2));
}

void LedController::updateStatus(ColorId status)
{
    // Volatile is necessary here, as otherwise the values inside the array are
    // optimized-away.
    volatile uint8_t led_data[3];
    {
        LedState led;
        getColor(&led, status);
        led_data[0] = led.green;
        led_data[1] = led.red;
        led_data[2] = led.blue;
    }

    const uint8_t * data = const_cast<const uint8_t *>(&(led_data[0]));
    const uint8_t * data_end = data + sizeof(led_data);
    blastLeds(data, data_end);
}
void LedController::update(const AbstractLedStrip * led_strip, ColorId status)
{
    updateStatus(status);

    const uint8_t * data = reinterpret_cast<const uint8_t *>(&led_strip->leds);
    const uint8_t * data_end =
            reinterpret_cast<const uint8_t *>(&led_strip->leds) +
            (led_strip->led_count * sizeof(LedState));
    blastLeds(data, data_end);
}
