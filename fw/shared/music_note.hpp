/**
 * @file
 */

#ifndef SHARED_MUSIC_NOTE_HPP_
#define SHARED_MUSIC_NOTE_HPP_

#include "defs.hpp"


/**
 * @brief Object representing single note by its octave and note
 */
class MusicNote
{
public:
    static const def::Uint8 INVALID_CODE = 0xFF;

    enum Tone: def::Uint8
    {
        TONE_C = 0, TONE_CS,
        TONE_D, TONE_DS,
        TONE_E,
        TONE_F, TONE_FS,
        TONE_G, TONE_GS,
        TONE_A, TONE_AS,
        TONE_B
    };

    static constexpr MusicNote InvalidNote() { return MusicNote(INVALID_CODE); }

    constexpr MusicNote() = default;
    constexpr MusicNote(const MusicNote &) = default;
    constexpr MusicNote & operator =(const MusicNote &) = default;
    constexpr MusicNote(def::Uint8 octave, Tone tone):
            code_((octave << 4) | static_cast<def::Uint8>(tone))
    { }

    constexpr def::Uint8 octave() const { return code_ >> 4; }
    constexpr Tone tone() const { return static_cast<Tone>(code_ & 0x0F); }

    bool isValid() const { return code_ != INVALID_CODE; }

    MusicNote & operator +=(int8_t diff)
    {
        int8_t new_tone = static_cast<int8_t>(tone()) + diff;
        def::Uint8 new_octave = octave();

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
    def::Uint8 code_ = 0;

    constexpr explicit MusicNote(def::Uint8 code):
        code_(code)
    { }
};


#endif  // SHARED_MUSIC_NOTE_HPP_
