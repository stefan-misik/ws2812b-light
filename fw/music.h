/**
 * @file
 */

#ifndef MUSIC_H_
#define MUSIC_H_

#include <stdint.h>

#include "buttons.h"


/**
 * @brief Object representing single music element
 */
class SongElement
{
public:
    enum class Control: uint8_t
    {
        CONTROL,
        PLAY_NOTE,
    };

    enum class NoteLength
    {
        WHOLE, HALF, QUARTER, EIGHTH, SIXTEENTH
    };

    static const uint8_t TERMINATE_CONTROL = 0;
    static const uint8_t END_REPEAT_CONTROL = 1;

    static const uint8_t SILENT_NOTE_BYTE = 0;
    static constexpr uint8_t makeNoteByte(uint8_t octave, uint8_t note) { return (octave << 4) | note; }
    static uint8_t parseOctave(uint8_t note_byte) { return note_byte >> 4; }
    static uint8_t parseNote(uint8_t note_byte) { return note_byte & 0x0F; }

    static constexpr SongElement Terminate() { return SongElement{Control::CONTROL, TERMINATE_CONTROL}; }
    static constexpr SongElement EndRepeat() { return SongElement{Control::CONTROL, END_REPEAT_CONTROL}; }
    static constexpr SongElement Repeat(uint8_t times) { return SongElement{Control::CONTROL, times}; }
    static constexpr SongElement PlayNote(NoteLength length)
    {
        return SongElement{Control::PLAY_NOTE, static_cast<uint8_t>(length)};
    }

    constexpr SongElement(Control control, uint8_t param):
            code_((static_cast<uint8_t>(control) << 4) | param)
    { }

    explicit SongElement(uint8_t code): code_(code) { }
    constexpr SongElement(const SongElement & ) = default;
    constexpr SongElement & operator =(const SongElement &) = default;
    constexpr operator uint8_t() const { return code_; }

    Control control() const { return static_cast<Control>(code_ >> 4); }
    uint8_t param() const { return code_ & 0x0F; }

    NoteLength noteLength() const { return static_cast<NoteLength>(param());}

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
