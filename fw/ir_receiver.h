/**
 * @file
 */

#ifndef IR_RECEIVER_H_
#define IR_RECEIVER_H_

#include <stdint.h>

#include "buttons.h"


class IrReceiver
{
public:
    using ButtonId = Buttons::ButtonId;

    static const uint8_t REPEAT_SKIP = 1;


    ButtonId button() const { return current_button_; }

    void initialize();
    ButtonId run();

private:
    ButtonId current_button_ = ButtonId::NONE;
    uint8_t repeat_count_ = 0;
};

#endif  // IR_RECEIVER_H_
