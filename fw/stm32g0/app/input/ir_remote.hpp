/**
 * @file
 */

#ifndef APP_INPUT_IR_REMOTE_HPP_
#define APP_INPUT_IR_REMOTE_HPP_

#include "app/input.hpp"
#include "driver/ir_receiver.hpp"


class IrRemoteSource final:
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


#endif  // APP_INPUT_IR_REMOTE_HPP_
