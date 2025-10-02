/**
 * @file
 */

#ifndef ANIMATION_H_
#define ANIMATION_H_

#include "tools/flags.hpp"
#include "led_strip.h"

#include <cstddef>
#include <cstdint>
#include <optional>


/**
 * @brief Class defining a single type of animation
 */
class Animation
{
public:
    /** @brief Parameter change type */
    enum class ChangeType
    {
        ABSOLUTE,
        RELATIVE,
    };

    /** @brief Render modifiers */
    enum class RenderFlag
    {
        MUSIC_STOPPED, /**< Music has stopped */
        NOTE_CHANGED,  /**< Music has changed a note */
    };

    /** @brief Store/restore data type */
    enum class DataType
    {
        BOTH,
        ONLY_CONFIG,
    };

    enum ParamId: std::uint32_t
    {
        RESERVED_,
        PRIMARY,
        SECONDARY,

        FIRST_CUSTOM_ID_ = 0x10000ul,
    };

    /**
     * @brief Render the animation frame onto the LED strip
     *
     * @param[in,out] strip Object representing the led strip
     * @param flags Flags modifying the render
     */
    virtual void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) = 0;

    /**
     * @brief Set an animation parameter
     *
     * @param param_id Parameter ID
     * @param value Value of the parameter
     * @param type Parameter change type
     *
     * @return Success
     */
    virtual bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) = 0;

    /**
     * @brief Get value of a parameter
     *
     * @param param_id Parameter ID
     * @param[out] value Value of the parameter
     *
     * @return Parameter value
     * @retval {} Failed to get parameter value
     */
    virtual std::optional<int> getParameter(std::uint32_t param_id) = 0;

    /**
     * @brief Store state of the animation
     *
     * @param[out] buffer Buffer to receive the state of the animation
     * @param capacity Capacity of the buffer
     * @param type Type of the state to store
     *
     * @return Number of bytes written in the buffer
     */
    virtual std::size_t store(void * buffer, std::size_t capacity, DataType type) const = 0;

    /**
     * @brief Restore state of the animation
     *
     * @param[in] buffer Buffer containing the state of the animation
     * @param max_size Maximum number of bytes to read from the buffer
     * @param type Type of the state to restore
     *
     * @return Number of bytes read from the buffer
     */
    virtual std::size_t restore(const void * buffer, std::size_t max_size, DataType type) = 0;

    virtual ~Animation() = default;
};

#endif  // ANIMATION_H_
