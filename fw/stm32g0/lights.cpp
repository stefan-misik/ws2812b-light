#include "lights.hpp"

#include "input/keypad.hpp"
#include "input/ir_remote.hpp"
#include "animation/rainbow.hpp"


namespace
{

}  // namespace


Lights::Lights():
    animation_{TypeTag<RainbowAnimation>{}}
{
}

bool Lights::initialize()
{
    if (!io_.initialize())
        return false;

    // Keypad and IR Receiver are now managed by Input and their respective
    // Input Sources, no need to manage them further here
    input_.createSource<KeypadSource>(0, &io_.keypad());
    input_.createSource<IrRemoteSource>(1, &io_.irReceiver());

    return true;
}

void Lights::step(std::uint32_t current_time)
{
    input_.update(current_time, &event_queue_);
    animation_->render(leds_.abstarctPtr(), {});
    {
        const auto [ev, cnt] = event_queue_.peek();
        bool green = false, yellow = false;
        if (cnt > 0 && ev[0].type() == EventQueue::EventType::KEY_EVENT)
        {
            const auto & param = ev[0].param<Input::EventParam>();
            if (param.flags & Input::EventParam::KeyFlags::PRESS) {
                green = param.source_id == 0;
                yellow = param.source_id == 1;
            }
        }
        io_.statusLeds().setLed(driver::StatusLeds::LedId::LED_GREEN, green);
        io_.statusLeds().setLed(driver::StatusLeds::LedId::LED_YELLOW, yellow);
    }
    event_queue_.discard();
    io_.statusLeds().update();
    io_.ledController().update(leds_.abstarctPtr());
}
