#include "time_service.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

volatile uint32_t TimeService::current_time_;


void TimeService::initialize()
{
    current_time_ = 0;

    // Clean flag register and enable overflow interrupt
    TIFR0 = (1 << OCF0B) | (1 << OCF0A) | (1 << TOV0);
    TIMSK0 = (0 << OCIE0B) | (0 << OCIE0A) | (1 << TOIE0);

    // CTC Mode, no output signals
    // F = 16 MHz / 256 = 62.5 kHz
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) |
            (0 << COM0B1) | (0 << COM0B0) |
            (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << FOC0A) | (0 << FOC0B) |
            (0 << WGM02) |
            (0 << CS02) | (0 << CS01) | (0 << CS00);
    // Overflow every 250 timer clock, which @62.5 kHz equals overflow every
    // 4 ms
    OCR0A = 249;
    OCR0B = 0;
}

uint32_t TimeService::getTime()
{
    uint32_t time;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        time = current_time_;
    }
    return time;
}

ISR(TIMER0_OVF_vect)
{
    TimeService::current_time_ += 4;
}

