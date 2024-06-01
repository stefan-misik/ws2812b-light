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
     * @brief Restart time counter
     */
    static void restartCounter();


    static bool shouldRun()
    {
        if (!should_run_)
            return false;
        should_run_ = false;
        return true;
    }

    /**
     * @brief Variable is set to true every 8 milliseconds
     */
    static volatile bool should_run_;
};


#endif  // TIME_SERVICE_H_
