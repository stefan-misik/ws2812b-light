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
     * @return Time in milliseconds
     */
    static uint32_t getTime();

    /**
     * @brief Current time in milliseconds
     */
    static volatile uint32_t current_time_;
};

#endif  // TIME_SERVICE_H_
