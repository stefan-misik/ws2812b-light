/**
 * @file
 */

#ifndef SONG_HPP_
#define SONG_HPP_

#include "defs.hpp"


enum class NoteLength: def::Uint8
{
    WHOLE = 0,
    HALF_DOT, HALF,
    QUARTER_DOT, QUARTER,
    EIGHTH_DOT, EIGHTH,
    SIXTEENTH
};


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
    enum class ControlType: def::Uint8
    {
        FLOW = 0,
        SET_OCTAVE,
        SILENCE,

        CONTROL_COUNT_,
    };

    enum ControlFlowParam: def::Uint8
    {
        CONTROL_FLOW_TERMINATE = 0,
        CONTROL_FLOW_LOOP_END = 0x0F,
    };

    static const def::Uint8 NOTE_START = static_cast<def::Uint8>(ControlType::CONTROL_COUNT_);

    static constexpr NoteLength toNoteLength(ControlType control)
    {
        return static_cast<NoteLength>(static_cast<def::Uint8>(control) - NOTE_START);
    }

    constexpr MusicElement(const MusicElement &) = default;
    constexpr MusicElement & operator =(const MusicElement &) = default;
    constexpr MusicElement(def::Uint8 code): code_(code) { }
    constexpr MusicElement & operator =(def::Uint8 code)
    {
        code_ = code;
        return *this;
    }

    constexpr operator def::Uint8() const { return code_; }

    constexpr MusicElement(NoteLength length, def::Int8 note_diff):
            code_(((NOTE_START + static_cast<def::Uint8>(length)) << 4) | static_cast<def::Uint8>(note_diff + 8))
    { }

    constexpr MusicElement(ControlType control, def::Uint8 param):
            code_((static_cast<def::Uint8>(control) << 4) | param)
    { }

    static constexpr MusicElement Terminate() { return MusicElement{ControlType::FLOW, 0}; }
    static constexpr MusicElement BeginLoop(def::Uint8 count) { return MusicElement{ControlType::FLOW, count}; }
    static constexpr MusicElement EndLoop() { return MusicElement{ControlType::FLOW, CONTROL_FLOW_LOOP_END}; }
    static constexpr MusicElement SetOctave(def::Uint8 octave) { return MusicElement{ControlType::SET_OCTAVE, octave}; }
    static constexpr MusicElement Silence(NoteLength length)
    {
        return MusicElement{ControlType::SILENCE, static_cast<def::Uint8>(length)};
    }

    ControlType controlType() const { return static_cast<ControlType>(code_ >> 4); }
    def::Uint8 param() const { return code_ & 0x0F; }
    def::Int8 noteDiff() const { return static_cast<def::Int8>(param()) - 8; }

private:
    def::Uint8 code_;
};


#endif  // SONG_HPP_
