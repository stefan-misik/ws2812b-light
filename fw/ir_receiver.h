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

    void initialize();
    ButtonId run();

private:
};

#endif  // IR_RECEIVER_H_
