#include "time_service.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

volatile bool TimeService::should_run_;


void TimeService::initialize()
{
    should_run_ = false;

    // Enable interrupts
    sei();

    // Disable power reduction
    PRR &= ~((1 << PRTIM0));

    // Firstly, stop the timer
    TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00));

    // Disable all interrupts, except overflow
    TIMSK &= ~((1 << TOIE0) | (1 << OCIE0B));
    TIMSK |= (1 << OCIE0A);
    // Overflow every 8 milliseconds: 15.625 kHz / 125 = 125 Hz
    OCR0A = 124;
    // Clear timer on Compare match (CTC) mode, disable compare outputs, frequency: 16 MHz / 1024 = 15.625 kHz
    TCCR0A = (0 << COM0A1) | (0 << COM0A0) |
            (0 << COM0B1) | (0 << COM0B0) |
            (1 << WGM01) | (0 << WGM00);
    TCCR0B = (0 << FOC0A) | (0 << FOC0B) |
            (0 << WGM02) |
            (1 << CS02) | (0 << CS01) | (1 << CS00);
}

void TimeService::restartCounter()
{
    TCNT0 = 0;
}

ISR(TIM0_COMPA_vect)
{
    TimeService::should_run_ = true;
}

