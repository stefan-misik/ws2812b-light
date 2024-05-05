/**
 * @file
 */

#ifndef MUSIC_H_
#define MUSIC_H_

#include <stdint.h>

#include "buttons.h"


/**
 * @brief Object representing single music element
 *
 * Each song element is a 8-bit unsigned integer organized as follows:
 *
 * # Element
 *     | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *     | C | C | C | C | . | . | . | . |
 *
 *    C - control code (see enum)
 *
 * ## Note
 *     | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *     | L | L | L | L | N | N | N | N |
 *
 *    L - Note length control code (see enum + NOTE_START)
 *    N - Note difference form previous note (0 ~ -8, 15 ~ +7)
 *
 * ## Control
 *     | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
 *     | T | T | T | T | P | P | P | P |
 *
 *    T - Control Type (see enum)
 *    P - Parameter
 */
class MusicElement
{
public:
    enum class NoteLength: uint8_t
    {
        WHOLE = 0,
        HALF_DOT, HALF,
        QUARTER_DOT, QUARTER,
        EIGHT_DOT, EIGHTH,
        SIXTEENTH
    };

    enum class ControlType: uint8_t
    {
        TERMINATE = 0,
        SET_OCTAVE,
        SILENCE,

        CONTROL_COUNT_,
    };

    static const uint8_t NOTE_START = static_cast<uint8_t>(ControlType::CONTROL_COUNT_);

    static constexpr NoteLength toNoteLength(ControlType control)
    {
        return static_cast<NoteLength>(static_cast<uint8_t>(control) - NOTE_START);
    }

    constexpr MusicElement(const MusicElement &) = default;
    constexpr MusicElement & operator =(const MusicElement &) = default;
    constexpr MusicElement(uint8_t code): code_(code) { }
    constexpr MusicElement & operator =(uint8_t code)
    {
        code_ = code;
        return *this;
    }

    constexpr operator uint8_t() const { return code_; }

    constexpr MusicElement(NoteLength length, int8_t note_diff):
            code_(((NOTE_START + static_cast<uint8_t>(length)) << 4) | static_cast<uint8_t>(note_diff + 8))
    { }

    constexpr MusicElement(ControlType control, uint8_t param):
            code_((static_cast<uint8_t>(control) << 4) | param)
    { }

    static constexpr MusicElement Silence(NoteLength length)
    {
        return MusicElement{ControlType::SILENCE, static_cast<uint8_t>(length)};
    }

    ControlType controlType() const { return static_cast<ControlType>(code_ >> 4); }
    uint8_t param() const { return code_ & 0x0F; }
    int8_t noteDiff() const { return static_cast<int8_t>(param()) - 8; }

private:
    uint8_t code_;
};


/**
 * @brief Object representing single note by its octave and note
 */
class MusicNote
{
public:
    enum Tone: uint8_t
    {
        TONE_C = 0, TONE_CS,
        TONE_D, TONE_DS,
        TONE_E,
        TONE_F, TONE_FS,
        TONE_G, TONE_GS,
        TONE_A, TONE_AS,
        TONE_B
    };

    constexpr MusicNote(const MusicNote &) = default;
    constexpr MusicNote & operator =(const MusicNote &) = default;
    constexpr MusicNote(uint8_t octave, Tone tone):
            code_((octave << 4) | static_cast<uint8_t>(tone))
    { }

    constexpr uint8_t octave() const { return code_ >> 4; }
    constexpr Tone tone() const { return static_cast<Tone>(code_ & 0x0F); }

    MusicNote & operator +(int8_t diff)
    {
        int8_t new_tone = static_cast<int8_t>(tone()) + diff;
        uint8_t new_octave = octave();

        if (new_tone < 0)
        {
            do
            {
                new_tone += 12;
                --new_octave;
            } while (new_tone < 0);
        }
        else if (new_tone > 11)
        {
            do
            {
                new_tone -= 12;
                ++new_octave;
            } while (new_tone > 11);
        }

        code_ = (new_octave << 4) | new_tone;
        return *this;
    }

private:
    uint8_t code_;
};


class Music
{
public:
    static void initialize();

    void play(Buttons::ButtonId btn);

private:
    uint8_t octave_ = 0;
    uint8_t note_ = 0;
};


#endif  // MUSIC_H_
