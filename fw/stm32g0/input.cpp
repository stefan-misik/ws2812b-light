#include "input.hpp"


namespace
{

}  // namespace

void Input::update(std::uint32_t time)
{
    {
        PressedButtonList list;
        for (std::size_t src_id = 0; src_id != SOURCE_COUNT; ++src_id)
        {
            list.resetAll();
            sources_[src_id]->getPressedKeys(time, &list);

            {
                const auto * pos = list.list();
                const auto * const end = pos + list.count();
                for (; pos != end; ++pos)
                    keys_[static_cast<std::size_t>(pos->key_id)].markPressedBy(src_id, pos->is_new);
            }
        }
    }

    for (auto & state: keys_)
        state.process();
}


void Input::KeyState::process()
{
    if (PressState::NONE == state_)
        source_id_ = INVALID_SOURCE_ID;

    // Mark press state as processed
    state_ = PressState::NONE;
}
