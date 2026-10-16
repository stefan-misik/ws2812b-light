/**
 * @file
 */

#ifndef APP_LOGGER_HPP_
#define APP_LOGGER_HPP_

#include <cstddef>
#include <utility>

#include "driver/usart_tx.hpp"
#include "driver/systick.hpp"
#include "tools/fixed_string.hpp"


extern driver::Systick system_time;

/**
 * @brief Writes diagnostic messages through a DMA-driven UART
 */
class Logger
{
public:
    static const inline std::size_t MAX_MESSAGE_SIZE = 64;

    enum class Level
    {
        ERROR = 0,
        WARNING,
        INFO
    };

    static const inline Level MAX_LEVEL =
    #ifdef DEBUG
        Level::INFO;
    #else
        Level::WARNING;
    #endif

    explicit Logger(driver::UsartTx * uart):
        uart_(*uart)
    {}

    Logger(const Logger &) = default;
    Logger(Logger &&) = default;

private:
    template <Level L>
    struct LevelPrefix
    {
        static constexpr char VALUE[] = "";
    };

public:
    template <Level LVL, FixedString FMT, typename... Ts>
    bool log(Ts &&... args);

private:
    driver::UsartTx & uart_;

    /**
     * @brief Format and queue a complete message for transmission
     *
     * The message is dropped if the UART cannot provide one contiguous buffer
     * large enough to contain it.
     *
     * @param format Printf-style format string
     * @param ... Format arguments
     *
     * @return Whether the complete message was queued
     */
    bool log(const char * format, ...);

};

template <>
struct Logger::LevelPrefix<Logger::Level::ERROR>
{ static constexpr char VALUE[] = "!"; };
template <>
struct Logger::LevelPrefix<Logger::Level::WARNING>
{ static constexpr char VALUE[] = "W"; };
template <>
struct Logger::LevelPrefix<Logger::Level::INFO>
{ static constexpr char VALUE[] = "I"; };

template <Logger::Level LVL, FixedString FMT, typename... Ts>
inline bool Logger::log(Ts &&... args)
{
    if constexpr (LVL > MAX_LEVEL)
        return true;

    return log(
        ConcatenatedFixedString<LevelPrefix<LVL>::VALUE, " %010lu: ", FMT>::VALUE.value,
        system_time.currentTime(), std::forward<Ts>(args)...);
}

#endif  // APP_LOGGER_HPP_