/**
 * @file
 */

#ifndef TIME_SERVICE_H_
#define TIME_SERVICE_H_

#include <stdint.h>

class TimeService
{
public:
    /**
     * @brief Initialize the time service
     */
    static void initialize();

    /**
     * @brief Get the current time
     *
     * @return Time in 8 milliseconds
     */
    static uint8_t getTime()
    {
        return current_time_;
    }

    /**
     * @brief Get the long form of current time
     *
     * @return Time in 8 milliseconds
     */
    static uint16_t getLongTime()
    {
        return current_time_;
    }

    /**
     * @brief Current time in 8 milliseconds
     */
    static volatile uint16_t current_time_;
};


/**
 * @brief Periodic routine
 *
 */
class PeriodicRoutine
{
public:
    PeriodicRoutine(uint8_t period_length = 1):
        period_length_(period_length),
        last_routine_run_(0)
    { }

    /**
     * @brief Change the period of the periodic routine
     *
     * @param period_length New length of the periodic routine run
     */
    void setPeriod(uint8_t period_length)
    {
        period_length_ = period_length;
    }

    /**
     * @brief Reset the timer used to time the routine period
     *
     * @param time Current time
     */
    void resetTimer(uint8_t time = TimeService::getTime())
    {
        last_routine_run_ = time;
    }

    /**
     * @brief Check whether the periodic routine should be ran at given time
     *
     * @param time Current time
     *
     * @retval false The routine was not called
     * @retval true The routine was called
     */
    bool shouldRun(uint8_t time = TimeService::getTime());

    /**
     * @brief Has at least one period elapsed since last run
     *
     * @param time Current time
     *
     * @return At least one period has elapsed
     */
    bool hasElapsed(uint8_t time = TimeService::getTime()) const
    {
        uint8_t time_since_last_run = time - last_routine_run_;
        return time_since_last_run >= period_length_;
    }

private:
    uint8_t period_length_;
    uint8_t last_routine_run_;
};
#endif  // TIME_SERVICE_H_
