#include "tools/id_selector.h"


uint8_t nextId(uint8_t value, uint8_t count)
{
    uint8_t new_value = value + 1;
    if (count == new_value)
        return 0;
    return new_value;
}

uint8_t previousId(uint8_t value, uint8_t count)
{
    if (0 == value)
        return count - 1;
    return value - 1;
}
