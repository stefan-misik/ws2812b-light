/**
 * @file
 */

#ifndef APP_LOGGER_HPP_
#define APP_LOGGER_HPP_

#include <cstddef>
#include <cstdint>
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
    static const inline std::size_t MAX_MESSAGE_SIZE = 96;

    enum class Level: std::uint8_t
    {
        ERROR = 0,
        WARNING,
        INFO,
        TRACE,
    };

    static const inline Level MAX_LEVEL =
    #ifdef DEBUG
        Level::TRACE;
    #else
        Level::INFO;
    #endif

    /**
     * @brief Check whether a log level is enabled
     *
     * @param lvl Log level to check
     *
     * @return Whether the log level is enabled
    */
    static constexpr bool isEnabled(Level lvl)
    {
        return lvl <= MAX_LEVEL;
    }

    template <Level LVL>
    static constexpr bool isEnabled()
    {
        return isEnabled(LVL);
    }

    explicit Logger(driver::UsartTx * uart):
        uart_(*uart)
    {}

    Logger(const Logger &) = default;
    Logger(Logger &&) = default;

    template <Level L>
    struct LevelToken
    {
        static constexpr Level LEVEL = L;
        static constexpr bool ENABLED = isEnabled<L>();
    };

    /**
     * @brief Log a message with a specific log level
     *
     * @tparam LVL Log level
     * @tparam FMT Format string
     * @tparam Ts Format arguments
     * @param args Format arguments
     *
     * @return true If the message was successfully queued, or ignored due to log level
     * @return false If the message could not be queued
     */
    template <Level LVL, FixedString FMT, typename... Ts>
    bool log(Ts &&... args);

    template <typename LVL_T, FixedString FMT, typename... Ts>
    bool log(Ts &&... args)
    {
        return log<LVL_T::LEVEL, FMT>(std::forward<Ts>(args)...);
    }

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
    [[gnu::format(printf, 2, 3)]]
    bool log(const char * format, ...);
};

template <Logger::Level LVL>
consteval FixedString<2> loggerLevelPrefix()
{
    if constexpr (LVL == Logger::Level::ERROR)
        return "!";
    else if constexpr (LVL == Logger::Level::WARNING)
        return "W";
    else if constexpr (LVL == Logger::Level::INFO)
        return "I";
    else if constexpr (LVL == Logger::Level::TRACE)
        return "T";
    else
        return "?";
}

template <Logger::Level LVL, FixedString FMT, typename... Ts>
inline bool Logger::log(Ts &&... args)
{
    if constexpr (!isEnabled<LVL>())
        return true;

    return log(ConcatenatedFixedString<loggerLevelPrefix<LVL>(), " % 10lu: ", FMT>::VALUE.value,
        system_time.currentTime(), std::forward<Ts>(args)...);
}

#endif  // APP_LOGGER_HPP_