/**
 * @file
 */

#ifndef TOOLS_TIMER_HPP_
#define TOOLS_TIMER_HPP_

#include <cstdint>


/**
 * @brief Simple object used to time tasks
 */
class Timer
{
public:
    Timer() = default;
    Timer(const Timer &) = default;
    Timer & operator =(const Timer &) = default;

    /**
     * @brief Start the timer
     *
     * @param current_time The current time
     */
    void start(std::uint32_t current_time)
    {
        start_ = current_time;
    }

    /**
     * @brief Get the elapsed time since the timer was started
     *
     * @param current_time The current time
     *
     * @return The elapsed time
     */
    std::uint32_t elapsed(std::uint32_t current_time) const
    {
        return current_time - start_;
    }

    /**
     * @brief Check if the timer has elapsed a certain duration
     *
     * @param current_time The current time
     * @param duration The duration to check against
     *
     * @return true if the timer has elapsed the duration, false otherwise
     */
    bool hasElapsed(std::uint32_t current_time, std::uint32_t duration) const
    {
        return elapsed(current_time) >= duration;
    }

    /**
     * @brief Shift the start time by a certain amount
     *
     * @param amount The amount to shift the start time by
     */
    void shift(std::uint32_t amount)
    {
        start_ += amount;
    }

private:
    std::uint32_t start_ = 0;
};


#endif  // TOOLS_TIMER_HPP_
