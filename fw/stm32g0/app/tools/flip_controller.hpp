/**
 * @file
 */

#ifndef APP_TOOLS_FLIP_CONTROLLER_HPP_
#define APP_TOOLS_FLIP_CONTROLLER_HPP_

#include <cstddef>
#include <cstdint>


/**
 * @brief Class managing double-buffering of LED data
 */
class FlipController
{
public:
    static constexpr std::size_t BUFFER_COUNT = 2u;

    std::size_t renderBuffer() const { return flip_state_; }
    std::size_t updateBuffer() const { return otherBuffer(flip_state_); }

    void flip()
    {
        flip_state_ = otherBuffer(flip_state_);
    }

private:
    std::uint8_t flip_state_ = 0u;

    static constexpr std::uint8_t otherBuffer(std::uint8_t buffer)
    {
        return 1u - buffer;
    }
};

#endif  // APP_TOOLS_FLIP_CONTROLLER_HPP_