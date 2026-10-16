#include "app/logger.hpp"

#include <cstdio>
#include <cstdarg>
#include <limits>


bool Logger::log(const char * format, ...)
{
    const auto [buffer, buffer_size] = uart_.reserveBuffer(MAX_MESSAGE_SIZE);
    if (buffer_size < 2)
        return false;

    std::va_list arguments;
    va_start(arguments, format);
    const int written_size = std::vsnprintf(static_cast<char *>(buffer), buffer_size, format, arguments);
    va_end(arguments);

    if (written_size >= (static_cast<int>(buffer_size) - 2) || written_size <= 0)
        return false;

    // Append newline
    static_cast<char *>(buffer)[written_size] = '\r';
    static_cast<char *>(buffer)[written_size + 1] = '\n';

    return uart_.commitBuffer(buffer, static_cast<std::size_t>(written_size + 2));
}
