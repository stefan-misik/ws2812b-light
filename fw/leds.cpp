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

static constexpr uint8_t ZERO_BIT_HIGH_LENGTH = 3;
static constexpr uint8_t ONE_BIT_HIGH_LENGTH = 10;

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
    OCR0A = 50;
}

void Leds::update() const
{
    const uint8_t * data = reinterpret_cast<const uint8_t *>(leds_);
    const uint8_t * data_end = reinterpret_cast<const uint8_t *>(leds_) +
            (count_ * sizeof(LedState));

    OCR0B = 0x00;
    StartTimer();

    for (; data != data_end; ++data)
    {
        for (uint8_t bit = 0x80; bit != 0; bit >>= 1)
        {
            OCR0B = (*data) & bit ? ONE_BIT_HIGH_LENGTH : ZERO_BIT_HIGH_LENGTH;
            while (0 == (TIFR0 & (1 << TOV0)))
            { }
            TIFR0 = (1 << OCF0B) | (1 << OCF0A) | (1 << TOV0);
        }
    }
    while (0 == (TIFR0 & (1 << OCF0B)))
    { }
    StopTimer();
    TIFR0 = (1 << OCF0B) | (1 << OCF0A) | (1 << TOV0);
}
