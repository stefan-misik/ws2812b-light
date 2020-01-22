#include "tools/button_filter.h"

void ButtonFilter::updateButton(bool button_state)
{
    if (button_state)
    {
        countUp();
    }
    else
    {
        countDown();
    }

    updateState();
}

void ButtonFilter::countUp()
{
    if (counter_ < NEXT_REPEAT_THRESHOLD)
    {
        ++counter_;
    }
    else
    {
        counter_ = REPEAT_THRESHOLD;
    }
}

void ButtonFilter::countDown()
{
    if (counter_ > PRESS_THRESHOLD)
    {
        counter_ = PRESS_THRESHOLD;
    }
    else if (counter_ > 0)
    {
        --counter_;
    }
}

void ButtonFilter::updateState()
{
    bool old_pressed = (state_ & PRESSED);
    uint8_t new_state = 0;

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
