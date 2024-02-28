#include "music.h"

#include <avr/io.h>
#include <avr/pgmspace.h>


namespace
{

const uint8_t NOTES[] PROGMEM = {
        239 - 1,  //  0: C
        226 - 1,  //  1: Cs
        213 - 1,  //  2: D
        201 - 1,  //  3: Ds
        190 - 1,  //  4: E
        179 - 1,  //  5: F
        169 - 1,  //  6: Fs
        160 - 1,  //  7: G
        151 - 1,  //  8: Gs
        142 - 1,  //  9: A
        134 - 1,  // 10: As
        127 - 1   // 11: B
};

inline void playNote(uint8_t octave, uint8_t note)
{
    const uint8_t prescaler = 12 - octave;
    TCCR1 = ((1 << CTC1) | (0 << PWM1A) | (0 << COM1A1) | (1 << COM1A0)) | prescaler;
    OCR1C = pgm_read_byte(&(NOTES[note]));
}

inline void stopNote()
{
    TCCR1 = ((1 << CTC1) | (0 << PWM1A) | (0 << COM1A1) | (0 << COM1A0));
}

}  // namespace


void Music::initialize()
{
    DDRB |= (1 << DDB1);  // Set the Buzzer GPIO to output mode

    // Disable power reduction
    PRR &= ~((1 << PRTIM1));

    stopNote();

    // Disable PWM B
    GTCCR &= ~((1 << PWM1B) | (1 << COM1B1) | (1 << COM1B0));
    // Disable all interrupts
    TIMSK &= ~((1 << TOIE1) | (1 << OCIE1B) | (1 << OCIE1A));
    // Use system clock
    PLLCSR &= ~((1 << PCKE));
}

void Music::play(Buttons::ButtonId btn)
{
    switch (btn)
    {
    case Buttons::X_BTN:
        if (11 == note_)
        {
            if (12 != octave_)
            {
                note_ = 0;
                ++octave_;
            }
        }
        else
            ++note_;
        break;

    case Buttons::O_BTN:
        if (0 == note_)
        {
            if (0 != octave_)
            {
                note_ = 11;
                --octave_;
            }
        }
        else
            -- note_;
        break;

    default:
        break;
    }

    if (0 == octave_ && 0 == note_)
        stopNote();
    else
        playNote(octave_, note_);
}
