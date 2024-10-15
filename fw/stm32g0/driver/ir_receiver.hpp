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
    class Packet
    {
    public:
        enum class Type
        {
            INVALID, REPEAT, NORMAL,
        };

        static Packet Invalid() { return Packet(Type::INVALID); }
        static Packet Repeat() { return Packet(Type::REPEAT); }

        Packet(): Packet(Type::INVALID)
        { }

        Packet(std::uint8_t address, std::uint8_t command):
            type_(static_cast<std::uint8_t>(Type::NORMAL)), address_(address), command_(command)
        { }

        Packet(const Packet &) = default;
        Packet & operator =(const Packet &) = default;

        Type type() const { return static_cast<Type>(type_); }
        std::uint8_t address() const { return address_; }
        std::uint8_t command() const { return command_; }

    private:
        std::uint32_t type_: 8;
        std::uint32_t address_: 8;
        std::uint32_t command_: 8;

        Packet(Type type): type_(static_cast<std::uint8_t>(type)), address_(), command_()
        { }
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
     * @return Received IR packet
     * @retval {} No more packets received
     */
    Packet read();

private:
    struct Private;
    struct PrivateStorage
    {
        char data[8 + (BUFFER_LENGTH + 4) + 8] alignas(std::uintptr_t);
    };

    Private & p() { return *reinterpret_cast<Private *>(&p_); }
    const Private & p() const { return *reinterpret_cast<const Private *>(&p_); }

    PrivateStorage p_;
};

}  // driver


#endif  // DRIVER_IR_RECEIVER_HPP_
