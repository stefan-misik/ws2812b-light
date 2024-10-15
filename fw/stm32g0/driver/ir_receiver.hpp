/**
 * @file
 */

#ifndef DRIVER_IR_RECEIVER_HPP_
#define DRIVER_IR_RECEIVER_HPP_

#include <utility>

#include "driver/common.hpp"
#include "music_note.hpp"


namespace driver
{

class IrReceiver
{
public:
    class Code
    {
    public:
        Code():
            is_valid_(false), address_(), command_()
        { }

        Code(std::uint8_t address, std::uint8_t command):
            is_valid_(true), address_(address), command_(command)
        { }

        Code(const Code &) = default;
        Code & operator =(const Code &) = default;

        bool isValid() const { return is_valid_; }
        std::uint8_t address() const { return address_; }
        std::uint8_t command() const { return command_; }

    private:
        std::uint32_t is_valid_: 8;
        std::uint32_t address_: 8;
        std::uint32_t command_: 8;
    };

    /**
     * @brief Length of the intermediate DMA buffer in data bytes
     */
    static const inline std::size_t BUFFER_LENGTH = 256;

    IrReceiver();
    ~IrReceiver();

    /**
     * @brief Initialize current instance for specified timer and channel ID
     *
     * @param tim_id Timer whose channel to initialize (it already needs to be initialized)
     * @param dma_channel_id ID of the DMA channel to use for
     *
     * @return Success
     */
    bool initialize(TimerId tim_id, std::uint8_t dma_channel_id);

    /**
     * @brief Read received IR packet
     *
     * @param time Current time in milliseconds
     *
     * @return Currently pressed button code and flag indicating that the button was pressed recently
     */
    std::pair<Code, bool> read(std::uint32_t time);

private:
    struct Private;
    struct PrivateStorage
    {
        char data[8 + (BUFFER_LENGTH + 4) + 8 + 4] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_IR_RECEIVER_HPP_
