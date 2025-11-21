#ifndef APP_MUSIC_HPP_
#define APP_MUSIC_HPP_

#include <cstddef>
#include <cstdint>

#include "music_note.hpp"


/**
 * @brief Music player object
 */
class Music
{
public:
    using SongPos = std::uint16_t;

    enum class Result
    {
        NONE,
        STOPPED,
        PLAYING,
        CHANGE,
        PRE_CHANGE,
    };

    static const std::size_t MAX_NESTED_LOOPS = 3;

    /**
     * @brief (Continue to) play a song
     *
     * @return Result of the music playing
     */
    Result play();

    const MusicNote & currentNote() const
    {
        return current_note_;
    }

    bool isPlaying() const
    {
        return 0 != current_song_id_;
    }

    void change(int diff)
    {
        current_song_id_ += diff;
        position_ = nullptr;
    }

private:
    struct LoopState
    {
        SongPos start_offset;
        std::uint8_t remaining;
    };

    static const std::uint8_t INVALID_LOOP_ID = 0xFF;

    std::uint8_t bpm_counter_ = 0;

    std::int8_t current_song_id_ = 0;
    const std::uint8_t * position_ = nullptr;

    std::uint8_t loop_id_ = INVALID_LOOP_ID;
    LoopState loops_[MAX_NESTED_LOOPS];

    MusicNote current_note_;
    SongPos remaining_duration_ = 0;

    static const std::uint8_t DIVIDE_FACTOR = 8;

    void reset()
    {
        remaining_duration_ = 0;
        loop_id_ = INVALID_LOOP_ID;
    }
};

#endif  // APP_MUSIC_HPP_