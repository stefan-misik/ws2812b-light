/**
 * @file
 */

#ifndef DRIVER_TIMER_HPP_
#define DRIVER_TIMER_HPP_

#include "driver/common.hpp"


namespace driver
{


class Systick
{
public:
    static bool initialize();

    std::uint32_t currentTime() const
    {
        return current_time_;
    }

    void handleInterrupt()
    {
        ++current_time_;
    }

private:
    std::uint32_t current_time_ = 0;
};


}  // driver


#endif  // DRIVER_TIMER_HPP_
