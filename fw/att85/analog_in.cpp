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

inline uint8_t getChannel()
{
    return ADMUX & ((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0));
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

    // Enable the ADC, start the conversion
    ADCSRA |= (1 << ADEN) | (1 << ADSC);
}

auto AnalogIn::convert(Channel next_channel) -> Channel
{
    if (ADCSRA & (1 << ADSC))
        return Channel::NONE;

    auto channel = Channel::NONE;
    switch (getChannel())
    {
    case KEYPAD_CH: keypad_ = ADC; channel = Channel::KEYPAD; break;
    case BAT_VMON_CH: bat_vmon_ = ADC; channel = Channel::BATTERY_VMON; break;
    }

    switch (next_channel)
    {
    case Channel::NONE: return channel;
    case Channel::KEYPAD: changeChannel(KEYPAD_CH); break;
    case Channel::BATTERY_VMON: changeChannel(BAT_VMON_CH); break;
    }

    // Start the conversion
    ADCSRA |= (1 << ADSC);
    return channel;
}
