#include "app/lights.hpp"

#include "app/tools/animation_parameter.hpp"
#include "app/input/keypad.hpp"
#include "app/input/ir_remote.hpp"
#include "app/animation/rainbow.hpp"


namespace
{

inline std::size_t changeAnimation(std::size_t current, int dir)
{
    return setCyclicParameter<std::size_t, AnimationStorage::SLOT_COUNT - 1>(current, dir,
        Animation::ChangeType::RELATIVE);
}

}  // namespace


Lights::Lights()
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
    handleEvents();
    {
        const auto music_result = handleMusic();
        Flags<Animation::RenderFlag> flags;
        switch (music_result)
        {
        case Music::Result::CHANGE: flags.setFlag(Animation::RenderFlag::NOTE_CHANGED); break;
        case Music::Result::STOPPED: flags.setFlag(Animation::RenderFlag::MUSIC_STOPPED); break;
        default: break;
        }
        animation_->render(leds_.abstractPtr(), flags);
        modifier_.modify(leds_.abstractPtr());
    }
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
            case EventType::KEY_EVENT:
                handleInputEvent(e->param<Input::EventParam>());
                break;
            default: break;
            }
        }
        event_queue_.release(cnt);
    }
}

Music::Result Lights::handleMusic()
{
    const auto result = music_.play();
    switch (result)
    {
    case Music::Result::CHANGE:
        io_.buzzer().playNote(music_.currentNote());
        break;
    case Music::Result::NONE:
    case Music::Result::STOPPED:
    case Music::Result::PRE_CHANGE:
        io_.buzzer().playNote(MusicNote::InvalidNote());
        break;
    case Music::Result::PLAYING:
        break;
    }
    return result;
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
        music_.change(1);
        return true;
    case Input::KeyId::KEY_X:
        music_.change(-1);
        return true;
    case Input::KeyId::KEY_0:
        modifier_.next();
        return true;
    default:
        return false;
    }

    return false;
}

void Lights::switchAnimation(int dir)
{
    const std::size_t next_id = changeAnimation(animation_.slotId(), dir);
    animation_.change(next_id);
}

