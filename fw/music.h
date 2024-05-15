/**
 * @file
 */

#ifndef MUSIC_H_
#define MUSIC_H_

#include <stdint.h>


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

    constexpr MusicNote() = default;
    constexpr MusicNote(const MusicNote &) = default;
    constexpr MusicNote & operator =(const MusicNote &) = default;
    constexpr MusicNote(uint8_t octave, Tone tone):
            code_((octave << 4) | static_cast<uint8_t>(tone))
    { }

    constexpr uint8_t octave() const { return code_ >> 4; }
    constexpr Tone tone() const { return static_cast<Tone>(code_ & 0x0F); }

    MusicNote & operator +=(int8_t diff)
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
    uint8_t code_ = 0;
};


/**
 * @brief Music player object
 */
class Music
{
public:
    static void initialize();

    void play();

    void change(int8_t diff)
    {
        current_song_id_ += diff;
        position_ = nullptr;
    }

private:
    uint8_t bpm_counter_ = 0;

    int8_t current_song_id_ = 0;
    const uint8_t * position_ = nullptr;

    MusicNote current_note_;
    uint8_t remaining_duration_ = 0;

    static const uint8_t DIVIDE_FACTOR = 8;
};


#endif  // MUSIC_H_
