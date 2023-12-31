#include "nvm_storage.h"

#include <util/crc16.h>
#include <avr/eeprom.h>


namespace {

}  // namespace


bool EepromBuffer::begin(uint8_t length, uintptr_t address)
{
    {
        if (length > BUFFER_CAPACITY)
            return false;

        if (address >= EEPROM_SIZE)
            return false;

        const uintptr_t end = address + length;
        if (end > EEPROM_SIZE)
            return false;
    }

    address_ = address;
    length_ = length;
    position_ = 0;
    return true;
}

bool EepromBuffer::continueWrite()
{
    if (length_ == position_)
        return true;

    // Wait for previous write to complete
    if (!eeprom_is_ready())
        return false;

    // Initiate single byte write
    EEAR = (address_ + position_);
    EEDR = buffer_[position_];
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
    ++position_;

    return false;
}

bool EepromBuffer::continueRead()
{
    if (length_ == position_)
        return true;

    // EEPROM can not be read from, before previous write completes
    if (!eeprom_is_ready())
        return false;

    // Perform single byte read
    EEAR = (address_ + position_);
    EECR |= (1 << EERE);
    buffer_[position_] = EEDR;
    ++position_;

    return false;
}


auto NvmStorage::run() -> Operation
{
    return Operation::NONE;
}

