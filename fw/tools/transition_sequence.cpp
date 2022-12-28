#include "tools/transition_sequence.h"


uint8_t makeTransitionSequence(uint8_t position, uint8_t length)
{
    return (static_cast<uint16_t>(position) * static_cast<uint16_t>(255)) /
            static_cast<uint16_t>(length);
}
