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

    enum Status
    {
        /**
         * @brief Successfully registered a button press
         */
        PRESS = (1 << 0),

        /**
         * @brief Received something, thus the run function blocked and timing
         *        is going to be broken
         */
        RECEIVED = (1 << 1),
    };

    static const uint8_t REPEAT_SKIP = 1;


    ButtonId button() const { return current_button_; }

    void initialize();
    uint8_t run();

private:
    ButtonId current_button_ = ButtonId::NONE;
    uint8_t repeat_count_ = 0;
};

#endif  // IR_RECEIVER_H_
