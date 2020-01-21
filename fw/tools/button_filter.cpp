#include "tools/button_filter.h"

void ButtonFilter::updateButton(bool button_state)
{
    if (button_state)
    {
        if (counter_ < NEXT_REPEAT_THRESHOLD)
        {
            ++counter_;
        }
        else
        {
            counter_ = FIRST_REPEAT_THRESHOLD;
        }
    }
    else
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
        if (!old_pressed)
        {
            new_state |= DOWN;
            new_state |= PRESS;
        }

        // This is also true in update cycle when NEXT_REPEAT_THRESHOLD is
        // reached, see above
        if (FIRST_REPEAT_THRESHOLD == counter_)
        {
            new_state |= PRESS;
        }
    }
    else if (old_pressed)
    {
        new_state |= PRESSED;
    }

    state_ = new_state;
}
