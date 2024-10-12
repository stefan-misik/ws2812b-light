/**
 * @file
 */

#ifndef DRIVER_BUZZER_HPP_
#define DRIVER_BUZZER_HPP_

#include "driver/common.hpp"
#include "music_note.hpp"


namespace driver
{

class Buzzer
{
public:
    Buzzer();
    ~Buzzer();

    /**
     * @brief Initialize current instance for specified timer and channel ID
     *
     * @param tim_id Timer whose channel to initialize (it already needs to be initialized)
     * @param channel_id Channel to associate the Buzzer instance with
     *
     * @return Success
     */
    bool initialize(TimerId tim_id, uint8_t channel_id);

    /**
     * @brief Play given note
     *
     * @param note Note to play, invalid note to stop playing
     */
    void playNote(MusicNote note);

private:
    struct Private;
    struct PrivateStorage
    {
        char data[8] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_BUZZER_HPP_
