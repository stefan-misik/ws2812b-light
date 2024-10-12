/**
 * @file
 */

#ifndef MUSIC_H_
#define MUSIC_H_

#include <stdint.h>

#include "music_note.hpp"


/**
 * @brief Music player object
 */
class Music
{
public:
    enum class Result: uint8_t
    {
        NONE,
        STOPPED,
        PLAYING,
        CHANGE
    };

    static const uint8_t MAX_NESTED_LOOPS = 3;

    static void initialize();

    /**
     * @brief (Continue to) play a song
     *
     * @return Result of the music playing
     */
    Result play();

    bool isPlaying() const
    {
        return 0 != current_song_id_;
    }

    void change(int8_t diff)
    {
        current_song_id_ += diff;
        position_ = nullptr;
    }

private:
    struct LoopState
    {
        uint8_t start_offset;
        uint8_t remaining;
    };

    static const uint8_t INVALID_LOOP_ID = 0xFF;

    uint8_t bpm_counter_ = 0;

    int8_t current_song_id_ = 0;
    const uint8_t * position_ = nullptr;

    uint8_t loop_id_ = INVALID_LOOP_ID;
    LoopState loops_[MAX_NESTED_LOOPS];

    MusicNote current_note_;
    uint8_t remaining_duration_ = 0;

    static const uint8_t DIVIDE_FACTOR = 8;

    void reset()
    {
        remaining_duration_ = 0;
        loop_id_ = INVALID_LOOP_ID;
    }
};


#endif  // MUSIC_H_
