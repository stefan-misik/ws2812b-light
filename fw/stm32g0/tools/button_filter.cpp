#include "tools/button_filter.hpp"

void ButtonFilter::updateButton(bool button_state)
{
    if (button_state)
    {
        if (PRESS_THRESHOLD != counter_)
            ++counter_;
        if (PRESS_THRESHOLD == counter_)
            is_pressed_ = true;
    }
    else
    {
        if (0 != counter_)
            --counter_;
        if (0 == counter_)
            is_pressed_ = false;
    }
}
