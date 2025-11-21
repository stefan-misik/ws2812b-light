/**
 * @file
 */

#ifndef APP_LED_CORRECTION_HPP_
#define APP_LED_CORRECTION_HPP_

#include <cstdint>

#include <utility>
#include <algorithm>
#include <type_traits>

#include "led_strip.hpp"


/**
 * @brief Interface of the object used to transform LED colors
 */
class LedCorrection
{
public:
    /**
     * @brief Correct the colors of the LEDs
     *
     * @param[in] leds LEDs whose color to transform
     * @param count Number of LEDs
     * @param[out] buffer Buffer to store the raw data into
     * @param capacity Capacity of the output buffer
     *
     * @return Number of bytes written into the buffer
     */
    virtual std::size_t correct(
            const LedState * leds, std::size_t count,
            std::uint8_t * buffer, std::size_t capacity) const = 0;

    virtual ~LedCorrection() = default;
};


template <typename T>
concept ComponentWriter = requires(std::uint8_t * buffer, std::uint8_t r, std::uint8_t g, std::uint8_t b){
    { T::setComponents(buffer, r, g, b) } -> std::same_as<void>;
};


/**
 * @brief Set of function used to write colors components into the data buffer
 * @tparam R Offset of Red color
 * @tparam G Offset of Green color
 * @tparam B Offset of Blue
 */
template <std::size_t R = 0, std::size_t G = 1, std::size_t B = 2>
struct StandardComponentWriter
{
    static void setComponents(std::uint8_t * buffer, std::uint8_t r, std::uint8_t g, std::uint8_t b)
    {
        buffer[R] = r;
        buffer[G] = g;
        buffer[B] = b;
    }
};

using ComponentWriterRGB = StandardComponentWriter<0, 1, 2>;
using ComponentWriterGRB = StandardComponentWriter<1, 0, 2>;


template <typename T>
concept LedWriter = requires
{
    { T::LED_LENGTH } -> std::convertible_to<std::size_t>;
} && requires(const T * writer, const LedState & led, std::uint8_t * buffer)
{
    { writer->writeLed(led, buffer) } -> std::same_as<void>;
};


/**
 * @brief Object used to write LED
 *
 * @tparam CWT Color writer type
 */
template <ComponentWriter CWT = ComponentWriterRGB>
class StandardLedWriter
{
public:
    static const inline std::size_t LED_LENGTH = 3;

    void writeLed(const LedState & led, std::uint8_t * buffer) const
    {
        CWT::setComponents(buffer, led.red, led.green, led.blue);
    }
};


/**
 * @brief Object used to write LEDs with dimmed intensity
 *
 * @tparam CWT Color writer type
 */
template <ComponentWriter CWT = ComponentWriterRGB>
class DimmingLedWriter
{
public:
    static const inline std::size_t LED_LENGTH = 3;

    DimmingLedWriter() = delete;

    DimmingLedWriter(std::uint32_t intensity):
        intensity_(intensity)
    { }

    void writeLed(const LedState & led, std::uint8_t * buffer) const
    {
        CWT::setComponents(buffer, dim(led.red), dim(led.green), dim(led.blue));
    }

    void changeIntensity(std::uint32_t intensity) { intensity_ = intensity; }

private:
    std::uint32_t intensity_ = 256;

    std::uint8_t dim(std::uint8_t value) const { return (value * intensity_) >> 8; }
};


/**
 * @brief Common implementation of a LED correction
 *
 * @tparam WT LED writer object
 */
template <LedWriter WT = StandardLedWriter<>>
class CommonLedCorrection final:
        public LedCorrection
{
public:
    using LedWriterType = WT;

    template <typename... Ts>
    CommonLedCorrection(Ts &&... args):
        writer_(std::forward<Ts>(args)...)
    { }

    /** @copydoc LedCorrection::correct() */
    std::size_t correct(
            const LedState * leds, std::size_t count,
            std::uint8_t * buffer, std::size_t capacity) const override
    {
        const LedState * const leds_end = leds + std::min<std::size_t>(count, capacity / LedWriterType::LED_LENGTH);
        std::uint8_t * buffer_pos = buffer;
        for (const LedState * leds_pos = leds; leds_pos != leds_end; ++leds_pos)
        {
            writer_.writeLed(*leds_pos, buffer_pos);
            buffer_pos += LedWriterType::LED_LENGTH;
        }
        return buffer_pos - buffer;
    }

private:
    LedWriterType writer_;
};


#endif  // APP_LED_CORRECTION_HPP_
