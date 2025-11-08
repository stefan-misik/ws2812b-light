#include "app/input.hpp"


namespace
{

inline std::uint8_t countUp(std::uint8_t counter)
{
    if (counter < Input::KeyState::NEXT_REPEAT_THRESHOLD)
        return counter + 1;
    else
        return Input::KeyState::REPEAT_THRESHOLD;
}

inline std::uint8_t updateKeyState(uint8_t previous_flags, uint8_t counter)
{
    const bool old_pressed = (previous_flags & Input::KeyState::PRESSED) != 0;
    std::uint8_t new_flags = 0;

    if (0 == counter)
    {
        if (old_pressed)
            new_flags |= Input::KeyState::UP;
    }
    else
    {
        new_flags |= Input::KeyState::PRESSED;

        if (!old_pressed)
            new_flags |= (Input::KeyState::DOWN | Input::KeyState::PRESS);
        else if (Input::KeyState::REPEAT_THRESHOLD == counter)
        {
            // This is branch is also taken in update cycle when
            // NEXT_REPEAT_THRESHOLD is reached, see countUp()
            new_flags |= Input::KeyState::PRESS;
        }
    }

    return new_flags;
}

}  // namespace

void Input::update(std::uint32_t time, EventQueue * event_queue)
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
    {
        state.process();
        const std::uint32_t event_flags = state.flags() & (KeyState::DOWN | KeyState::UP | KeyState::PRESS);
        if (0 != event_flags)
        {
            const auto key_id = static_cast<KeyId>(&state - keys_);
            event_queue->pushEvent<EventParam>(key_id, event_flags, state.repeat(), state.sourceId());
        }
    }
}


void Input::KeyState::process()
{
    const auto press_state = readPressState();
    if (PressState::NEWLY_PRESSED == press_state)
    {
        repeat_ = 0;
        counter_ = 0;
        flags_ &= ~PRESSED;  // Clear the pressed flag to generate new press event
    }

    if (PressState::NONE != press_state)
        counter_ = countUp(counter_);
    else
    {
        source_id_ = INVALID_SOURCE_ID;
        counter_ = 0;
    }

    flags_ = updateKeyState(flags_, counter_);

    if (flags_ & PRESS)
        repeat_ += 1;
}
