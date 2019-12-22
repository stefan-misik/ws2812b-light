#include "leds.h"

#include <avr/io.h>

static inline void StopTimer()
{
    // Stop the timer
    TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));
}

static inline void StartTimer()
{
    // Start the timer at clk_io frequency
    TCCR0B |=  ((0 << CS02) | (0 << CS01) | (1 << CS00));
}

static constexpr uint8_t ZERO_BIT_PULSE_LENGTH = 3;
static constexpr uint8_t ONE_BIT_PULSE_LENGTH = 10;

void Leds::initialize(LedState * leds, size_t count)
{
    leds_ = leds;
    count_ = count;

    StopTimer();

    // Set output direction to OC0B - PD5
    DDRD |= ((1 << DDD5));

    // Disable power reduction for Timer 0
    PRR &= ~((1 << PRTIM0));

    // Disable all interrupts
    TIMSK0 &= ~((1 << TOIE0) | (1 << OCIE0A) | (1 << OCIE0B));
    // Disable PWM output A
    TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
    // Set the PWM output B to clear on match and set at BOTTOM
    TCCR0A &= ~((1 << COM0B1) | (1 << COM0B0));
    TCCR0A |=  ((1 << COM0B1) | (0 << COM0B0));
    // Set the Fast PWM mode with TOP set to OCR0A value - mode 7
    TCCR0A |=  ((1 << WGM01) | (1 << WGM00));
    TCCR0B |=  ((1 << WGM02));
    // Timer clears after 22 * (1/16 MHz) = 1.375 us
    OCR0A = 30;
}

void Leds::update() const
{
    const uint8_t * data = reinterpret_cast<const uint8_t *>(leds_);
    const uint8_t * data_end = reinterpret_cast<const uint8_t *>(leds_) +
            (count_ * sizeof(LedState));

    OCR0B = 0x00;
    StartTimer();

    {
        uint8_t current_byte;
        uint8_t bit_position;
        uint8_t pulse_length;

        asm volatile (
                "    rjmp  lu_check_end                \n"
                "lu_get_byte:                          \n"
                "    ld    %[current_byte], %a[data]+  \n"


                "    ldi   %[bit_position], 0x80       \n"
                "lu_send_bit:                          \n"
                "    ldi   %[pulse_length], %[zero_pl] \n"
                "    lsl   %[current_byte]             \n"
                "    brcc  lu_send_bit_continue        \n"
                "    ldi   %[pulse_length], %[one_pl]  \n"
                "lu_send_bit_continue:                 \n"
                "    out   %[ocr], %[pulse_length]     \n"

                "lu_wait_send_bit:                     \n"
                "    sbis  %[tifr], %[tovb]            \n"
                "    rjmp  lu_wait_send_bit            \n"
                "    sbi   %[tifr], %[tovb]            \n"

                "lu_check_byte_sent:                   \n"
                "    lsr   %[bit_position]             \n"
                "    brne  lu_send_bit                 \n"


                "lu_check_end:                         \n"
                "    cp    %A[data], %A[data_end]      \n"
                "    cpc   %B[data], %B[data_end]      \n"
                "    breq  lu_end                      \n"
                "    rjmp  lu_get_byte                 \n"

                "lu_end:                               \n"
                "    sbis  %[tifr], %[tovb]            \n"
                "    rjmp  lu_end                      \n"
                "    sbi   %[tifr], %[tovb]            \n"
                : [current_byte] "=&r" (current_byte),
                  [bit_position] "=&d" (bit_position),
                  [pulse_length] "=&d" (pulse_length)
                : [data] "z" (data),
                  [data_end] "e" (data_end),
                  [zero_pl] "M" (ZERO_BIT_PULSE_LENGTH),
                  [one_pl] "M" (ONE_BIT_PULSE_LENGTH),
                  [ocr] "I" (_SFR_IO_ADDR(OCR0B)),
                  [tifr] "I" (_SFR_IO_ADDR(TIFR0)),
                  [tovb] "I" (TOV0)
        );
    }

    StopTimer();
    TIFR0 = (1 << OCF0B) | (1 << OCF0A) | (1 << TOV0);
}
