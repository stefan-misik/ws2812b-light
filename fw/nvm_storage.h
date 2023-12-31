/**
 * @file
 */

#ifndef NVM_STORAGE_H_
#define NVM_STORAGE_H_

#include <stdint.h>


struct GlobalConfiguration
{
    uint8_t animatio_id;
};

struct AnimationConfigurationData
{
    static const uint8_t LENGTH = 7;
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

    void * data() { return reinterpret_cast<void *>(buffer_); }

    const void * data() const { return reinterpret_cast<const void *>(buffer_); }
    uint8_t lebgth() const { return length_; }

private:
    uintptr_t address_ = 0;
    char buffer_[BUFFER_CAPACITY];
    uint8_t length_;
    uint8_t position_;
};


/**
 * @brief Class managing NVM storage
 */
class NvmStorage
{
public:
    enum class Operation: uint8_t
    {
        NONE,
        WRITE_GLOBAL,
        WRITE_ANIMATION,
        READ_GLOBAL,
        READ_ANIMATION,
    };

    bool requestLoad()
    {
        if (State::IDLE != state_)
            return false;

        return true;
    }

    bool requestSave()
    {
        if (State::IDLE != state_)
            return false;

        return true;
    }

    Operation run();

    GlobalConfiguration * writeGlobalConfiguration()
    {
        return reinterpret_cast<GlobalConfiguration *>(buffer_.data());
    }

    AnimationConfigurationData * writeAnimationConfigurationData()
    {
        return reinterpret_cast<AnimationConfigurationData *>(buffer_.data());
    }

    const GlobalConfiguration * readGlobalConfiguration() const
    {
        return reinterpret_cast<const GlobalConfiguration *>(buffer_.data());
    }

    const AnimationConfigurationData * readAnimationConfigurationData() const
    {
        return reinterpret_cast<const AnimationConfigurationData *>(buffer_.data());
    }

    void noMoreAnimation()
    {
        switch (state_)
        {
        case State::SAVE_START_ANIMATION:
        case State::LOAD_START_ANIMATION:
            state_ = State::IDLE;
            break;
        default: break;
        }
    }

    uint8_t animationId() const { return step_; }

private:
    enum class State: uint8_t
    {
        IDLE,
        SAVE_GLOBAL,
        SAVE_START_ANIMATION,
        SAVE_ANIMATION,
        LOAD_GLOBAL,
        LOAD_START_ANIMATION,
        LOAD_ANIMATION,
    };

    EepromBuffer buffer_;

    uint8_t step_ = 0;
    State state_ = State::IDLE;
};


#endif  // NVM_STORAGE_H_
