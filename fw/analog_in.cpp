#include "analog_in.h"

#include <avr/io.h>


namespace
{

const uint8_t KEYPAD_CH = (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (1 << MUX0);
const uint8_t BAT_VMON_CH = (0 << MUX3) | (0 << MUX2) | (1 << MUX1) | (0 << MUX0);

inline void changeChannel(uint8_t channel)
{
    ADMUX = (0 << REFS2) | (0 << REFS1) | (0 << REFS2) | (0 << ADLAR) | channel;
}

}  // namespace


void AnalogIn::initialize()
{
    changeChannel(KEYPAD_CH);
    // Disable and stop the ADC, clear interrupt flags, disable interrupts
    // set the maximum prescaler (16 MHz / 128 = 125 kHz)
    ADCSRA = (0 << ADEN) | (0 << ADSC) | (1 << ADIF) | (0 << ADIE) |
            (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    // No bipolar input, multiplexer is not used by analog comparator,
    // no polarity reversal,
    // do not care about auto trigger source, as it is not used.
    ADCSRB = (0 << BIN) | (0 << ACME) | (0 << IPR);
    // Disable digital input buffer on battery voltage monitoring input, as it
    // is never used for digital input
    DIDR0 = (0 << ADC0D) | (0 << ADC1D) | (1 << ADC2D) | (0 << ADC3D) |
            (0 << AIN1D) | (0 << AIN0D);

    // Enable the ADC
    ADCSRA |= (1 << ADEN);
}

void AnalogIn::update()
{
}
