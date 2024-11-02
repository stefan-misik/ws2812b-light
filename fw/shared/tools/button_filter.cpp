#include "tools/button_filter.h"

namespace
{

inline def::Uint8 countUp(def::Uint8 counter)
{
    if (counter < ButtonFilter::NEXT_REPEAT_THRESHOLD)
        return counter + 1;
    else
        return ButtonFilter::REPEAT_THRESHOLD;
}

inline def::Uint8 countDown(def::Uint8 counter)
{
    if (counter > ButtonFilter::PRESS_THRESHOLD)
        return ButtonFilter::PRESS_THRESHOLD;
    else if (counter > 0)
        return counter - 1;
    else
        return counter;
}

inline def::Uint8 countHardUp(def::Uint8 counter)
{
    if (counter < ButtonFilter::PRESS_THRESHOLD)
        return ButtonFilter::PRESS_THRESHOLD;
    else if (counter < ButtonFilter::NEXT_REPEAT_THRESHOLD)
        return counter + 1;
    else
        return ButtonFilter::REPEAT_THRESHOLD;
}

inline def::Uint8 countHardDown(def::Uint8 counter)
{
    (void)counter;
    return 0;
}

}  // namespace


void ButtonFilter::updateButton(bool button_state)
{
    if (button_state)
        counter_ = countUp(counter_);
    else
        counter_ = countDown(counter_);

    updateState();
}

void ButtonFilter::updateHardButton(bool button_state)
{
    if (button_state)
        counter_ = countHardUp(counter_);
    else
        counter_ = countHardDown(counter_);

    updateState();
}

void ButtonFilter::updateState()
{
    bool old_pressed = (state_ & PRESSED);
    def::Uint8 new_state = 0;

    if (0 == counter_)
    {
        if (old_pressed)
        {
            new_state |= UP;
        }
    }
    else if (counter_ >= PRESS_THRESHOLD)
    {
        new_state |= PRESSED;

        // This condition could use counter comparison to PRESS_THRESHOLD,
        // however following has more stable behavior, as it won't cause erratic
        // presses (and button down events) in case the button is momentarily
        // released (e.g. just for one update period).
        if (!old_pressed)
        {
            new_state |= (DOWN | PRESS);
        }
        else if (REPEAT_THRESHOLD == counter_)
        {
            // This is branch is also taken in update cycle when
            // NEXT_REPEAT_THRESHOLD is reached, see countUp()
            new_state |= PRESS;
        }
    }
    else if (old_pressed)
    {
        new_state |= PRESSED;
    }

    state_ = new_state;
}
