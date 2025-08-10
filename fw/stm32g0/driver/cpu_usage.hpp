/**
 * @file
 */

#ifndef DRIVER_CPU_USAGE_HPP_
#define DRIVER_CPU_USAGE_HPP_

#include <cstdint>

#include "tools/hidden.hpp"
#include "driver/common.hpp"


namespace driver
{

/**
 * @brief Simple driver to measure CPU usage
 */
class CpuUsage
{
private:
    using TimerType = std::uint16_t;

public:
    struct Stats
    {
        TimerType min;
        TimerType max;
        TimerType last;
    };

    CpuUsage();
    ~CpuUsage();

    bool initialize(TimerId tim_id);

    void startPeriod();
    void endPeriod();

    const Stats & stats() const
    {
        return stats_;
    }

    void clearStats()
    {
        clear_stats_ = true;
    }

private:
    struct Private;
    Hidden<Private, 4> p_;

    TimerType previous_start_;

    Stats stats_;
    bool clear_stats_ = false;
};

}  // driver


#endif  // DRIVER_CPU_USAGE_HPP_
