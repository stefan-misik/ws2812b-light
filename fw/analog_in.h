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

    void initialize();
    void update();

    uint16_t keypad() const { return keypad_; }
    uint16_t batteryVmon() const { return bat_vmon_; }

private:
    uint16_t keypad_ = INVALID_VALUE;
    uint16_t bat_vmon_ = INVALID_VALUE;
};




#endif  // ANALOG_IN_H_
