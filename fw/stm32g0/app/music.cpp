#include "app/music.hpp"

#include "music_note.hpp"
#include "song.hpp"
#include <cstddef>


namespace songs
{

#define PROGMEM

#include "songs/silent_night.inc"
#include "songs/jingle_bells.inc"

}  // namespace songs


namespace
{

const std::size_t SONG_COUNT = 3;

const std::uint8_t * getSongById(std::uint8_t id)
{
    switch (id)
    {
    default:
    case 0: return nullptr;
    case 1: return songs::SILENT_NIGHT;
    case 2: return songs::JINGLE_BELLS;
    }
}

const std::uint8_t LENGTHS[] = {
        32,  // Whole
        16 + 8,  // Half dot
        16,  // Half
        8 + 4,  // Quarter dot
        8,  // Quarter
        4 + 2,  // Eighth dot
        4,  // Eighth
        2,  // Sixteenth
};
static_assert(sizeof(LENGTHS)/sizeof(LENGTHS[0]) == (static_cast<std::uint8_t>(NoteLength::SIXTEENTH) + 1),
    "Check note lengths");

inline std::uint8_t getNoteDuration(NoteLength length)
{
    return LENGTHS[static_cast<std::uint8_t>(length)];
}

}  // namespace


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
            position_ = reinterpret_cast<const std::uint8_t *>(0xFFFF);  // Non-zero value
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
        ++bpm_counter_;
        // Stop the note just before end of last section
        return 1 == remaining_duration_ ? Result::PRE_CHANGE : Result::PLAYING;

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
        std::uint8_t duration = 0;
        Result result = Result::PLAYING;

        const MusicElement el(*position_);

        const MusicElement::ControlType control = el.controlType();
        switch (control)
        {
        case MusicElement::ControlType::FLOW:
        {
            const std::uint8_t * const song_start = getSongById(current_song_id_);
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
