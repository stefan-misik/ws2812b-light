/**
 * @file
 */

#ifndef NVM_STORAGE_H_
#define NVM_STORAGE_H_

#include <stdint.h>


struct AnimationConfigurationData
{
    static const uint8_t LENGTH = 4;
    char data[LENGTH];
};


/**
 * @brief Tool used to access data located in EEPROM
 */
class EepromBuffer
{
public:
    static const uint8_t BUFFER_CAPACITY = 8;

    static const uintptr_t EEPROM_SIZE = 512;

    EepromBuffer():
        length_(0),
        position_(0)
    { }

    bool isDone() const { return length_ == position_; }

    /**
     * @brief Begin writing/writing data in the buffer
     *
     * @param length Length of the new operation
     * @param address EEPROM address to write into / read from
     *
     * @return Operation successfully started
     */
    bool begin(uint8_t length, uintptr_t address);

    /**
     * @brief Continue writing
     *
     * @return Is finished
     */
    bool continueWrite();

    /**
     * @brief Continue writing
     *
     * @return Is finished
     */
    bool continueRead();

    uint8_t capacity() const { return BUFFER_CAPACITY; }

    void * buffer() { return reinterpret_cast<void *>(buffer_); }

    const void * buffer() const { return reinterpret_cast<const void *>(buffer_); }
    uint8_t lebgth() const { return length_; }

private:
    uintptr_t address_ = 0;
    char buffer_[BUFFER_CAPACITY];
    uint8_t length_;
    uint8_t position_;
};



#endif  // NVM_STORAGE_H_
