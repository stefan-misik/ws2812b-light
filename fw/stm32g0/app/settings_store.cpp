#include "app/settings_store.hpp"


void SettingsStore::initialize()
{
    dirty_flags_.fill(false);
}

void SettingsStore::run(std::uint32_t current_time)
{
    (void)current_time;  // To avoid unused parameter warning
}