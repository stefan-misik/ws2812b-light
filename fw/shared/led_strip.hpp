/**
 * @file
 */

#ifndef SHARED_LED_STRIP_HPP_
#define SHARED_LED_STRIP_HPP_

#ifndef LED_ORDER
#define LED_ORDER red,green,blue
#endif  // LED_ORDER

#include "defs.hpp"

using LedSize = def::SmallSize;

template <LedSize LED_C>
struct LedStrip;

using AbstractLedStrip = LedStrip<1>;

struct LedState
{
    LedState() = default;  // @suppress("Class members should be properly initialized") leave uninitialized by default
    constexpr LedState(const LedState &) = default;
    constexpr LedState(LedState &&) = default;

    explicit constexpr LedState(uint32_t color):
            red(color >> 16), green(color >> 8), blue(color >> 0)
    { }

    constexpr LedState(uint8_t red, uint8_t green, uint8_t blue):
            red(red), green(green), blue(blue)
    { }

    constexpr uint32_t color() const
    {
        return static_cast<uint32_t>(red) << 16 |
            static_cast<uint32_t>(green) << 8 |
            static_cast<uint32_t>(blue) << 0;
    }

    constexpr LedState & operator =(const LedState &) = default;
    constexpr LedState & operator =(LedState &&) = default;

    constexpr bool operator ==(const LedState & o) const
    {
        return o.red == red && o.green == green && o.blue == blue;
    }

    constexpr bool operator !=(const LedState & o) const
    {
        return o.red != red || o.green != green || o.blue != blue;
    }

    uint8_t LED_ORDER;
};

/**
 * @brief State of a LED strip
 */
template <LedSize LED_C>
struct LedStrip
{
    LedStrip():
        led_count(LED_C)
    { }

    const LedSize led_count;
    LedState leds[LED_C];

    /**
     * @brief Get the ID of the next LED
     *
     * Wrap to the begin at the end.
     *
     * @param id LED ID
     * @return Next LED ID
     */
    LedSize nextId(LedSize id) const
    {
        ++id;
        if (id == led_count)
            id = 0;
        return id;
    }

    /**
     * @brief Get the id of previous LED
     *
     * Wrap to the end at the begin.
     *
     * @param id LED ID
     * @return Previous LED ID
     */
    LedSize prevId(LedSize id) const
    {
        return 0 == id ? (led_count - 1) : (id - 1);
    }

    /**
     * @brief Operator for easy access to configured LED states
     *
     * @param led Led number
     * @return Reference to the LED state
     */
    LedState & operator[] (LedSize led)
    {
        return leds[led];
    }

    /**
     * @brief Begin iteration
     *
     * @return Pointer to first LED state
     */
    LedState * begin()
    {
        return &(leds[0]);
    }

    /**
     * @brief End of iteration
     *
     * @return Pointer to past-end LED state
     */
    LedState * end()
    {
        return &(leds[led_count]);
    }

    /**
     * @brief Make abstract LED strip pointer
     *
     * @return Pointer to an abstract LED strip type
     */
    AbstractLedStrip * abstractPtr()
    {
        return reinterpret_cast<AbstractLedStrip *>(this);
    }

    /** @copydoc abstractPtr() */
    const AbstractLedStrip * abstractPtr() const
    {
        return reinterpret_cast<const AbstractLedStrip *>(this);
    }
};


#endif  // SHARED_LED_STRIP_HPP_
