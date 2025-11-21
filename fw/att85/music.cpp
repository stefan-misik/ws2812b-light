#include "music.h"

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "tools/id_selector.h"
#include "song.hpp"


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
#include "songs/jingle_bells.inc"

}  // namespace songs

static const uint8_t SONG_COUNT = 3;
const uint8_t * getSongById(uint8_t id)
{
    switch (id)
    {
    default:
    case 0: return nullptr;
    case 1: return songs::SILENT_NIGHT;
    case 2: return songs::JINGLE_BELLS;
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

auto Music::play() -> Result
{
    if (nullptr == position_)
    {
        switch (current_song_id_)
        {
        case -1: current_song_id_ = SONG_COUNT - 1; break;
        case SONG_COUNT: current_song_id_ = 0; break;
        }

        if (0 == current_song_id_)
        {
            stopNote();
            position_ = reinterpret_cast<const uint8_t *>(0xFFFF);  // Non-zero value
            return Result::STOPPED;
        }
        else
        {
            position_ = getSongById(current_song_id_);
            reset();
        }
    }

    if (0 == current_song_id_)
        return Result::NONE;

    // Check BPM counter
    switch (bpm_counter_)
    {
    case DIVIDE_FACTOR - 1:
        bpm_counter_ = 0;
        break;

    case DIVIDE_FACTOR - 2:
        // Stop the note just before end of last section
        if (1 == remaining_duration_)
            stopNote();
        // no break

    default:
        ++bpm_counter_;
        return Result::PLAYING;
    }

    if (remaining_duration_ > 1)
    {
        --remaining_duration_;
        // Wait for the current note to end
        return Result::PLAYING;
    }

    while (true)
    {
        uint8_t duration = 0;
        Result result = Result::PLAYING;

        const MusicElement el(pgm_read_byte(position_));

        const MusicElement::ControlType control = el.controlType();
        switch (control)
        {
        case MusicElement::ControlType::FLOW:
        {
            const uint8_t * const song_start = getSongById(current_song_id_);
            switch (el.param())
            {
            case MusicElement::CONTROL_FLOW_TERMINATE:
                duration = 64;
                position_ = song_start - 1;
                loop_id_ = INVALID_LOOP_ID;
                break;

            case MusicElement::CONTROL_FLOW_LOOP_END:
                if (loop_id_ != INVALID_LOOP_ID)
                {
                    auto & current_loop = loops_[loop_id_];
                    if (0 == current_loop.remaining)
                    {
                        loop_id_ -= 1;
                    }
                    else
                    {
                        current_loop.remaining -= 1;
                        // Recall the position to the loop start element, it will be skipped by `++position_;` later
                        position_ = song_start + current_loop.start_offset;
                    }
                }
                break;

            default:  // Loop Start
                if (loop_id_ != (MAX_NESTED_LOOPS - 1))
                {
                    loop_id_ += 1;
                    auto & current_loop = loops_[loop_id_];
                    current_loop.remaining = el.param();
                    current_loop.start_offset = position_ - song_start;
                }
                break;
            }
        }
            break;

        case MusicElement::ControlType::SET_OCTAVE:
            current_note_ = MusicNote(el.param(), MusicNote::TONE_C);
            break;

        case MusicElement::ControlType::SILENCE:
            duration = getNoteDuration(static_cast<NoteLength>(el.param()));
            break;

        default:
            current_note_ += el.noteDiff();
            duration = getNoteDuration(MusicElement::toNoteLength(control));
            playNote(current_note_.octave(), current_note_.tone());
            result = Result::CHANGE;
            break;
        }

        ++position_;
        if (0 != duration)
        {
            remaining_duration_ = duration;
            return result;
        }
    }
}
