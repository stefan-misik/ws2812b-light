#include "music.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "tools/id_selector.h"
#include "song.h"


namespace
{

const uint8_t LENGTHS[] PROGMEM = {
        32,  // Whole
        16 + 8,  // Half dot
        16,  // Half
        8 + 4,  // Quarter dot
        8,  // Quarter
        4 + 2,  // Eighth dot
        4,  // Eighth
        2,  // Sixteenth
};

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

uint8_t getNoteDuration(NoteLength length)
{
    return pgm_read_byte(LENGTHS + static_cast<uint8_t>(length));
}


namespace songs
{

#include "songs/silent_night.inc"

}  // namespace songs

static const uint8_t SONG_COUNT = 2;
const uint8_t * getSongById(uint8_t id)
{
    switch (id)
    {
    default:
    case 0: return nullptr;
    case 1: return songs::SILENT_NIGHT;
    }
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
    {
        bool change_song = false;
        switch (btn)
        {
        case Buttons::X_BTN:
            current_song_id_ = nextId(current_song_id_, SONG_COUNT);
            change_song = true;
            break;

        case Buttons::O_BTN:
            current_song_id_ = previousId(current_song_id_, SONG_COUNT);
            change_song = true;
            break;

        default:
            break;
        }

        if (change_song)
        {
            remaining_ = 0;
            position_ = 0;
            current_song_ = getSongById(current_song_id_);
        }
    }

    // Check BPM counter
    switch (DIVIDE_FACTOR - bpm_counter_)
    {
    case 1:
        bpm_counter_ = 0;
        break;

    case 2:
        // Stop the note just before end of last section
        if (1 == remaining_)
            stopNote();
        // no break

    default:
        ++bpm_counter_;
        return;
    }

    if (remaining_ > 1)
    {
        --remaining_;
        // Wait for the current note to end
        return;
    }

    if (0 == current_song_id_)
    {
        stopNote();
        return;
    }

    while (true)
    {
        bool done = true;

        const MusicElement el(pgm_read_byte(current_song_ + position_));

        const MusicElement::ControlType control = el.controlType();
        switch (control)
        {
        case MusicElement::ControlType::TERMINATE:
            remaining_ = 64;
            position_ = -1;
            break;

        case MusicElement::ControlType::SET_OCTAVE:
            current_note_ = MusicNote(el.param(), MusicNote::TONE_C);
            done = false;
            break;

        case MusicElement::ControlType::SILENCE:
            remaining_ = getNoteDuration(static_cast<NoteLength>(el.param()));
            break;

        default:
            current_note_ += el.noteDiff();
            remaining_ = getNoteDuration(MusicElement::toNoteLength(control));
            playNote(current_note_.octave(), current_note_.tone());
            break;
        }

        ++position_;
        if (done)
            break;
    }
}
