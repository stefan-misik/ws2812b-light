/**
 * @file
 */

#ifndef TOOLS_PERIODIC_TIMER_HPP_
#define TOOLS_PERIODIC_TIMER_HPP_

#include <cstdint>


/**
 * @brief Simple object used to time a periodic task
 */
class PeriodicTimer
{
public:
    /**
     * @param Check whether the periodic task should run
     *
     * @param now Current time
     * @param period Period of the task
     *
     * @return Task should be run
     */
    bool shouldRun(std::uint32_t now, std::uint32_t period)
    {
        const std::uint32_t diff = now - last_;
        if (diff >= period)
        {
            last_ += period;
            return true;
        }
        return false;
    }

private:
    std::uint32_t last_ = 0;
};


#endif  // TOOLS_PERIODIC_TIMER_HPP_
