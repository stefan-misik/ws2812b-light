/**
 * @file
 */

#ifndef APP_SETTINGS_STORE_HPP_
#define APP_SETTINGS_STORE_HPP_

#include <array>
#include <cstdint>

#include "app/animation_storage.hpp"
#include "app/records.hpp"


/**
 * @brief Coordinates user settings and their asynchronous EEPROM persistence.
 *
 * Animation configurations remain owned by AnimationStorage. Record 0 stores
 * the selected slot; record 1 + slot ID stores that slot's configuration.
 * Changes made while loading take precedence over the stored settings.
 * Call run() after servicing the EEPROM driver. On error, error() reports the
 * failure and initialize() can be used to retry without losing dirty changes.
 */
class SettingsStore
{
public:
    using AnimationSlotId = AnimationStorage::AnimationSlotId;

    static constexpr std::uint32_t SAVE_DELAY_MS = 2000;

    SettingsStore(AnimationStorage & animations, Records & records, driver::i2c::Cat24cx & eeprom):
        animations_(animations), records_(records), eeprom_(eeprom)
    { }

    void initialize();
    void run(std::uint32_t current_time);

    bool setAnimationParameter(std::uint32_t param_id, int value, Animation::ChangeType type,
        std::uint32_t current_time);
    bool changeAnimation(AnimationSlotId slot_id, std::uint32_t current_time);

    /** True after the initial scan and load have completed successfully. */
    bool isReady() const;
    Records::Result error() const { return error_; }

private:
    enum class State: std::uint8_t
    {
        UNINITIALIZED,
        SCANNING,
        LOADING,
        READING,
        READY,
        WRITING,
        ERROR,
    };

    static constexpr Records::RecordId SELECTION_RECORD = 0;
    // Increment when the slot mapping or animation configuration layout changes.
    static constexpr std::uint8_t FORMAT_VERSION = 1;
    static_assert(AnimationStorage::SLOT_COUNT + 1 <= Records::MAX_RECORDS);

    AnimationStorage & animations_;
    Records & records_;
    driver::i2c::Cat24cx & eeprom_;
    std::array<bool, AnimationStorage::SLOT_COUNT> dirty_slots_{};
    bool dirty_selection_ = false;
    std::uint32_t last_change_time_ = 0;
    Records::RecordId current_record_ = SELECTION_RECORD;
    State state_ = State::UNINITIALIZED;
    Records::Result error_ = Records::SUCCESS;

    bool isDirty(Records::RecordId record_id) const;
    void setDirty(Records::RecordId record_id, bool dirty);
    void loadCurrentRecord();
    void beginSave(Records::RecordId record_id);
};


#endif  // APP_SETTINGS_STORE_HPP_