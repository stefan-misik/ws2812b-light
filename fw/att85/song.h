/**
 * @file
 */

#ifndef SONG_H_
#define SONG_H_

#include <avr/pgmspace.h>


enum class NoteLength: uint8_t
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
    enum class ControlType: uint8_t
    {
        FLOW = 0,
        SET_OCTAVE,
        SILENCE,

        CONTROL_COUNT_,
    };

    enum ControlFlowParam: uint8_t
    {
        CONTROL_FLOW_TERMINATE = 0,
        CONTROL_FLOW_LOOP_END = 0x0F,
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

    static constexpr MusicElement Terminate() { return MusicElement{ControlType::FLOW, 0}; }
    static constexpr MusicElement BeginLoop(uint8_t count) { return MusicElement{ControlType::FLOW, count}; }
    static constexpr MusicElement EndLoop() { return MusicElement{ControlType::FLOW, CONTROL_FLOW_LOOP_END}; }
    static constexpr MusicElement SetOctave(uint8_t octave) { return MusicElement{ControlType::SET_OCTAVE, octave}; }
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


#endif  // SONG_H_
