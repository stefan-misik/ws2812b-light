/**
 * @file
 */

#ifndef ANALOG_IN_H_
#define ANALOG_IN_H_

#include <stdint.h>


class AnalogIn
{
public:
    static const uint16_t INVALID_VALUE = 0xFFFF;

    enum class Channel: uint8_t
    {
        NONE, KEYPAD, BATTERY_VMON
    };

    void initialize();

    /**
     * @brief Try to finish AD conversion
     *
     * @param next_channel Channel to change to when current conversion finishes
     *
     * @return Currently finished conversion
     * @retval Channel::NONE conversion is underway
     */
    Channel convert(Channel next_channel);

    uint16_t keypad() const { return keypad_; }
    uint16_t batteryVmon() const { return bat_vmon_; }

private:
    uint16_t keypad_ = INVALID_VALUE;
    uint16_t bat_vmon_ = INVALID_VALUE;
};




#endif  // ANALOG_IN_H_
