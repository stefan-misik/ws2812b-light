/**
 * @file
 */

#ifndef INPUT_IR_REMOTE_HPP_
#define INPUT_IR_REMOTE_HPP_

#include "input.hpp"
#include "driver/ir_receiver.hpp"


class IrRemoteSource:
        public Input::Source
{
public:
    IrRemoteSource(driver::IrReceiver * receiver):
        receiver_(receiver)
    { }

    void getPressedKeys(std::uint32_t time, Input::PressedButtonList * buttons) final;

private:
    driver::IrReceiver * const receiver_;
};


#endif  // INPUT_IR_REMOTE_HPP_
