#include "app/lights.hpp"

#include "app/input/keypad.hpp"
#include "app/input/ir_remote.hpp"
#include "app/animation/rainbow.hpp"


namespace
{

inline std::size_t changeAnimation(std::size_t current, int dir)
{
    if (dir > 0)
    {
        if (current == (Lights::ANIMATION_SLOT_CNT - 1))
            return 0;
        return current + 1;
    }
    else if (dir < 0)
    {
        if (0 == current)
            return Lights::ANIMATION_SLOT_CNT - 1;
        return current - 1;
    }
    return current;
}

}  // namespace


Lights::Lights():
        current_register_(0u)
{
    {
        std::size_t animation_id = 0ul;
        for (auto & anim : animation_file_)
        {
            anim.setAnimationId(animation_id++);
            anim.reset();
        }
    }
    animation_.change(animation_file_[current_register_].animationId());
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
    handleEvents();
    animation_->render(leds_.abstractPtr(), {});
    io_.statusLeds().update();
    io_.ledController().update(leds_.abstractPtr());
}

void Lights::runBackgroundTasks(std::uint32_t current_time)
{
    io_.run();
    (void) current_time;
}

void Lights::handleEvents()
{
    while (true)
    {
        const auto [ev, cnt] = event_queue_.peek();
        if (0 == cnt)
            break;
        const auto * const e_end = ev + cnt;
        for (const auto * e = ev; e != e_end; ++e)
        {
            switch (e->type())
            {
            case EventQueue::EventType::KEY_EVENT:
                handleInputEvent(e->param<Input::EventParam>());
                break;
            default: break;
            }
        }
        event_queue_.release(cnt);
    }
}

bool Lights::handleInputEvent(const Input::EventParam & e)
{
    if (0 == (e.flags & Input::KeyState::PRESS))
        return false;
    switch (e.key)
    {
    case Input::KeyId::KEY_RIGHT:
        if (!animation_->setParamater(Animation::PRIMARY, 1, Animation::ChangeType::RELATIVE))
            switchAnimation(1);
        return true;
    case Input::KeyId::KEY_LEFT:
        if (!animation_->setParamater(Animation::PRIMARY, -1, Animation::ChangeType::RELATIVE))
            switchAnimation(-1);
        return true;
    case Input::KeyId::KEY_UP:
        animation_->setParamater(Animation::SECONDARY, 1, Animation::ChangeType::RELATIVE);
        return true;
    case Input::KeyId::KEY_DOWN:
        animation_->setParamater(Animation::SECONDARY, -1, Animation::ChangeType::RELATIVE);
        return true;
    case Input::KeyId::KEY_O:
        return true;
    case Input::KeyId::KEY_X:
        return true;
    default:
        return false;
    }

    return false;
}

void Lights::switchAnimation(int dir)
{
    const std::size_t next_id = changeAnimation(current_register_, dir);
    if (next_id == current_register_)
        return;

    animation_file_[current_register_].store(animation_.get(), Animation::DataType::BOTH);

    auto & reg = animation_file_[next_id];
    animation_.change(reg.animationId());
    reg.restore(animation_.get(), Animation::DataType::BOTH);
    current_register_ = next_id;
}

