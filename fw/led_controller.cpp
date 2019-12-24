#include "led_controller.h"

#include <avr/io.h>


static constexpr uint8_t ZERO_BIT_PULSE_LENGTH = 2;
static constexpr uint8_t ONE_BIT_PULSE_LENGTH = 4;


void LedController::initialize(AbstractLedStrip * led_strip)
{
    strip_ = led_strip;

    // Set output direction to OC0B - PD5
    PORTD &= ~((1 << DDD5));
    DDRD |= ((1 << DDD5));
}

void LedController::update() const
{
    const uint8_t * data = reinterpret_cast<const uint8_t *>(&strip_->leds);
    const uint8_t * data_end =
            reinterpret_cast<const uint8_t *>(&strip_->leds) +
            (strip_->led_count * sizeof(AbstractLedStrip::LedState));

    {
        uint8_t current_byte;
        uint8_t bit_position;
        uint8_t pulse_length;
        uint8_t old_sreg;

        asm volatile (
                "    rjmp  lu_check_all_sent           \n"
                "lu_send_byte:                         \n"
                "    ld    %[current_byte], %a[data]+  \n"
                "    ldi   %[bit_position], 0x80       \n"

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
                "    lsr   %[bit_position]             \n"
                "    brne  lu_send_bit                 \n"

                "lu_check_all_sent:                    \n"
                "    cp    %A[data], %A[data_end]      \n"
                "    cpc   %B[data], %B[data_end]      \n"
                "    breq  lu_end                      \n"
                "    rjmp  lu_send_byte                \n"

                "lu_end:                               \n"
                : [current_byte] "=&r" (current_byte),
                  [old_sreg] "=&r" (old_sreg),
                  [bit_position] "=&d" (bit_position),
                  [pulse_length] "=&d" (pulse_length)
                : [data] "z" (data),
                  [data_end] "e" (data_end),
                  [zero_pl] "M" (ZERO_BIT_PULSE_LENGTH),
                  [one_pl] "M" (ONE_BIT_PULSE_LENGTH),
                  [pinr] "I" (_SFR_IO_ADDR(PIND)),
                  [pinb] "I" (PIND5)
        );
    }
}
