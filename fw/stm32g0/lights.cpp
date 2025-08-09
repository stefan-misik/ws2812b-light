#include "lights.hpp"


bool Lights::initialize()
{
    return true;
}

void Lights::step(std::uint32_t current_time)
{
    input_.update(current_time, &event_queue_);
    rainbow_.update(leds_.abstarctPtr());
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
        status_leds_.setLed(driver::StatusLeds::LedId::LED_GREEN, green);
        status_leds_.setLed(driver::StatusLeds::LedId::LED_YELLOW, yellow);
    }
    event_queue_.discard();
    status_leds_.update();
}
