#include "app/settings_store.hpp"


void SettingsStore::initialize()
{
    // A write may have been interrupted by an explicit retry.
    if (State::WRITING == state_)
        setDirty(current_record_, true);
    records_.reinitialize();
    error_ = Records::SUCCESS;
    current_record_ = SELECTION_RECORD;
    state_ = State::SCANNING;
}

bool SettingsStore::setAnimationParameter(std::uint32_t param_id, int value, Animation::ChangeType type,
    std::uint32_t current_time)
{
    const bool accepted = animations_->setParamater(param_id, value, type);
    if (accepted)
    {
        dirty_slots_[animations_.slotId()] = true;
        last_change_time_ = current_time;
    }
    return accepted;
}

bool SettingsStore::changeAnimation(AnimationSlotId slot_id, std::uint32_t current_time)
{
    const auto previous_slot = animations_.slotId();
    const bool changed = animations_.change(slot_id);
    if (changed && slot_id != previous_slot)
    {
        dirty_selection_ = true;
        last_change_time_ = current_time;
    }
    return changed;
}

bool SettingsStore::isReady() const
{
    return State::READY == state_ || State::WRITING == state_;
}

bool SettingsStore::isDirty(Records::RecordId record_id) const
{
    return SELECTION_RECORD == record_id ? dirty_selection_ : dirty_slots_[record_id - 1u];
}

void SettingsStore::setDirty(Records::RecordId record_id, bool dirty)
{
    if (SELECTION_RECORD == record_id)
        dirty_selection_ = dirty;
    else
        dirty_slots_[record_id - 1u] = dirty;
}

void SettingsStore::loadCurrentRecord()
{
    if (isDirty(current_record_))
        return;
    const auto data = records_.getRecordData(&eeprom_);
    if (data.empty() || FORMAT_VERSION != data[0])
        return;
    if (SELECTION_RECORD == current_record_)
    {
        if (data.size() != 3u)
            return;
        const auto slot_id = static_cast<AnimationSlotId>(
            (static_cast<AnimationSlotId>(data[1]) << 8u) | data[2]);
        if (slot_id < AnimationStorage::SLOT_COUNT)
            animations_.change(slot_id);
    }
    else
    {
        const auto slot_id = static_cast<AnimationSlotId>(current_record_ - 1u);
        animations_.restoreConfig(slot_id, data.data() + 1u, data.size() - 1u);
    }
}

void SettingsStore::beginSave(Records::RecordId record_id)
{
    auto buffer = records_.writeRecordBuffer(&eeprom_);
    if (buffer.empty())
        return;

    buffer[0] = FORMAT_VERSION;
    std::size_t length = 0;
    if (SELECTION_RECORD == record_id)
    {
        if (buffer.size() >= 3u)
        {
            const auto slot_id = animations_.slotId();
            buffer[1] = static_cast<std::uint8_t>(slot_id >> 8u);
            buffer[2] = static_cast<std::uint8_t>(slot_id);
            length = 3u;
        }
    }
    else
    {
        const auto slot_id = static_cast<AnimationSlotId>(record_id - 1u);
        const auto config_size = animations_.storeConfig(slot_id, buffer.data() + 1u, buffer.size() - 1u);
        if (0u != config_size)
            length = config_size + 1u;
    }
    if (0u == length)
    {
        error_ = Records::BAD_LENGTH;
        state_ = State::ERROR;
        return;
    }

    const bool started = records_.beginWrite(record_id, length);
    if (started)
    {
        // A later change sets the bit again while this snapshot is in flight.
        setDirty(record_id, false);
        current_record_ = record_id;
        state_ = State::WRITING;
    }
}

void SettingsStore::run(std::uint32_t current_time)
{
    if (State::UNINITIALIZED == state_ || State::ERROR == state_)
        return;

    const auto result = records_.run(&eeprom_, current_time);
    if (result >= Records::FIRST_ERROR_)
    {
        if (State::WRITING == state_)
            setDirty(current_record_, true);
        error_ = result;
        state_ = State::ERROR;
        return;
    }
    if (Records::PENDING == result)
        return;

    switch (state_)
    {
    case State::SCANNING:
        current_record_ = SELECTION_RECORD;
        state_ = State::LOADING;
        break;

    case State::READING:
        loadCurrentRecord();
        ++current_record_;
        state_ = State::LOADING;
        break;

    case State::WRITING:
        state_ = State::READY;
        break;

    default:
        break;
    }

    if (State::LOADING == state_)
    {
        while (current_record_ <= AnimationStorage::SLOT_COUNT)
        {
            if (!isDirty(current_record_) && records_.getRecordPage(current_record_) != Records::INVALID_PAGE_ID)
            {
                const bool started = records_.beginRead(current_record_);
                if (started)
                    state_ = State::READING;
                return;
            }
            ++current_record_;
        }
        state_ = State::READY;
    }

    if (State::READY != state_ || current_time - last_change_time_ < SAVE_DELAY_MS)
        return;

    // Save configurations before the selected slot, without implying atomicity.
    for (std::size_t slot = 0; slot < AnimationStorage::SLOT_COUNT; ++slot)
    {
        if (dirty_slots_[slot])
        {
            beginSave(static_cast<Records::RecordId>(slot + 1u));
            return;
        }
    }
    if (dirty_selection_)
        beginSave(SELECTION_RECORD);
}