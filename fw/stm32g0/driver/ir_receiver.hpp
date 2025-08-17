/**
 * @file
 */

#ifndef DRIVER_IR_RECEIVER_HPP_
#define DRIVER_IR_RECEIVER_HPP_

#include <utility>

#include "tools/hidden.hpp"
#include "driver/common.hpp"


namespace driver
{

class IrReceiver
{
public:
    class Code
    {
    public:
        static Code Invalid() { return Code{}; }

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

        friend bool operator ==(const Code & lhs, const Code & rhs)
        {
            if (lhs.is_valid_ != rhs.is_valid_)
                return false;
            if (!lhs.is_valid_)
                return true;
            if (lhs.address_ != rhs.address_)
                return false;
            if (lhs.command_ != rhs.command_)
                return false;
            return true;
        }

        friend bool operator !=(const Code & lhs, const Code & rhs)
        {
            return ! operator ==(lhs, rhs);
        }

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
    bool initialize(TimerId tim_id, DmaChannelId dma_channel_id);

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
    Hidden<Private, 8 + (BUFFER_LENGTH + 4) + 8 + 4> p_;
    Code current_code_ = Code::Invalid();
};

}  // driver


#endif  // DRIVER_IR_RECEIVER_HPP_
