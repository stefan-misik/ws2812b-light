/**
 * @file
 */

#ifndef DRIVER_IR_RECEIVER_HPP_
#define DRIVER_IR_RECEIVER_HPP_

#include "driver/common.hpp"
#include "music_note.hpp"


namespace driver
{

class IrReceiver
{
public:
    enum class ChannelPair
    {
        CHANNEL_1_2,
        CHANNEL_3_4,
    };

    IrReceiver();
    ~IrReceiver();

    /**
     * @brief Initialize current instance for specified timer and channel ID
     *
     * @param tim_id Timer whose channel to initialize (it already needs to be initialized)
     * @param channel_pair_id Channel to associate the IR Receiver instance with
     *
     * @return Success
     */
    bool initialize(TimerId tim_id, ChannelPair channel_pair_id);

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
        char data[12] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_IR_RECEIVER_HPP_
