/**
 * @file
 */

#ifndef APP_MAIN_HPP_
#define APP_MAIN_HPP_

#include "app/lights.hpp"

extern Lights lights;

/**
 * @brief Get the global logger
 *
 * @return Logger Logger
 */
inline Logger logger()
{
    return lights.logger();
}


#endif  // APP_MAIN_HPP_