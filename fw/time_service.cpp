#include "time_service.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

volatile uint8_t TimeService::current_time_;


void TimeService::initialize()
{
    // Enable interrupts
    sei();

    // Disable power reduction
    PRR &= ~((1 << PRTIM1));

    // Firstly, stop the timer
    TCCR1 &= ~((1 << CS13) | (1 << CS12) | (1 << CS11) | (1 << CS10));

    // Use synchronous mode of Timer 1
    PLLCSR &= ~((1 << PCKE));
    // Disable all interrupts, except overflow
    TIMSK &= ~((1 << TOIE1) | (1 << OCIE1B));
    TIMSK |= (1 << OCIE1A);
    // Overflow every 8 milliseconds: 31.25 kHz / 250 = 125 Hz
    OCR1A = 0;
    OCR1C = 249;
    // Disable PWM B
    GTCCR &= ~((1 << PWM1B) | (1 << COM1B1) | (1 << COM1B0) |
            (1 << FOC1B) | (1 << FOC1A) | (1 << PSR1));
    // Reload on OCR1C match, disable PWM A, frequency: 16 MHz / 512 = 31.25 kHz
    TCCR1 = (1 << CTC1) | (0 << PWM1A) | (0 << COM1A1) | (0 << COM1A0) |
            (1 << CS13) | (0 << CS12) | (1 << CS11) | (0 << CS10);

    current_time_ = 0;
}

bool PeriodicRoutine::shouldRunAt(uint8_t time)
{
    uint8_t time_since_last_run = time - last_routine_run_;
    if (time_since_last_run >= period_length_)
    {
        last_routine_run_ += period_length_;
        return true;
    }
    else
    {
        return false;
    }
}

ISR(TIM1_COMPA_vect)
{
    ++TimeService::current_time_;
}

