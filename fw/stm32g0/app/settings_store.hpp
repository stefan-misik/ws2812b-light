/**
 * @file
 */

#ifndef APP_SETTINGS_STORE_HPP_
#define APP_SETTINGS_STORE_HPP_

#include <cstdint>

#include "tools/timer.hpp"
#include "tools/bit_array.hpp"
#include "app/records.hpp"
#include "app/animation_storage.hpp"


class SettingsStore
{
public:
    static constexpr std::uint32_t SETTINGS_UPDATE_DELAY_MS = 10000ul;

    explicit SettingsStore(Records & records, AnimationStorage & animation_storage):
        records_(records),
        animation_storage_(animation_storage)
    { }

    /** @brief Initialize the settings store */
    void initialize();

    /**
     * @brief Run the settings store
     *
     * @param current_time The current time in milliseconds
     */
    void run(std::uint32_t current_time);

private:
    enum class State: std::uint8_t
    {
        IDLE = 0u,
        DIRTY,
    };

    struct MainSettings
    {
        std::uint8_t animation_id = 0;
    };

    Records & records_;
    AnimationStorage & animation_storage_;

    State state_ = State::IDLE;
    BitArray<32> dirty_flags_;
    Timer last_change_time_;
};

#endif  // APP_SETTINGS_STORE_HPP_
