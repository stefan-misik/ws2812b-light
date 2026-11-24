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

inline Flags<Animation::RenderFlag> makeRenderFlags(Music::Result music_result)
{
    Flags<Animation::RenderFlag> flags;
    switch (music_result)
    {
    case Music::Result::CHANGE: flags.setFlag(Animation::RenderFlag::NOTE_CHANGED); break;
    case Music::Result::STOPPED: flags.setFlag(Animation::RenderFlag::MUSIC_STOPPED); break;
    default: break;
    }
    return flags;
}

}  // namespace


Lights::Lights():
    settings_(animation_, records_, io_.eeprom())
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

    settings_.initialize();

    return true;
}

void Lights::step(std::uint32_t current_time)
{
    // Update the LED controller with the buffer rendered in the previous step
    io_.ledController().update(leds_[flip_controller_.updateBuffer()].abstractPtr());
    // Update the input sources and push events to the event queue
    input_.update(current_time, &event_queue_);

    handleEvents(current_time);
    {
        Flags<Animation::RenderFlag> flags;

        // Handle music
        {
            const auto music_result = handleMusic();
            flags |= makeRenderFlags(music_result);
        }

        // Render animation
        {
            auto & render_buffer = leds_[flip_controller_.renderBuffer()];
            const bool new_frame = animation_->render(render_buffer.abstractPtr(), flags);
            if (new_frame)
            {
                modifier_.modify(render_buffer.abstractPtr());
                flip_controller_.flip();
            }
        }
    }
    handleStatusLeds();
}

void Lights::runBackgroundTasks(std::uint32_t current_time)
{
    io_.run(current_time);
    const auto previous_error = settings_.error();
    settings_.run(current_time);
    const auto settings_error = settings_.error();
    if (Records::SUCCESS == previous_error && Records::SUCCESS != settings_error)
        logger().log<Logger::Level::ERROR, "Settings persistence failed: %u">(
            static_cast<std::uint32_t>(settings_error));
}

void Lights::handleEvents(std::uint32_t current_time)
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
                handleInputEvent(e->param<Input::EventParam>(), current_time);
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
        io_.buzzer().stopNote();
        break;
    case Music::Result::PLAYING:
        break;
    }
    return result;
}

bool Lights::handleInputEvent(const Input::EventParam & e, std::uint32_t current_time)
{
    if (0 == (e.flags & Input::KeyState::PRESS))
        return false;
    switch (e.key)
    {
    case Input::KeyId::KEY_RIGHT:
    {
        const bool accepted = settings_.setAnimationParameter(Animation::PRIMARY, 1, Animation::ChangeType::RELATIVE,
            current_time);
        if (!accepted)
            switchAnimation(1, current_time);
        return true;
    }
    case Input::KeyId::KEY_LEFT:
    {
        const bool accepted = settings_.setAnimationParameter(Animation::PRIMARY, -1, Animation::ChangeType::RELATIVE,
            current_time);
        if (!accepted)
            switchAnimation(-1, current_time);
        return true;
    }
    case Input::KeyId::KEY_UP:
        settings_.setAnimationParameter(Animation::SECONDARY, 1, Animation::ChangeType::RELATIVE, current_time);
        return true;
    case Input::KeyId::KEY_DOWN:
        settings_.setAnimationParameter(Animation::SECONDARY, -1, Animation::ChangeType::RELATIVE, current_time);
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

void Lights::handleStatusLeds()
{
    const bool new_status_leds = status_leds_.run();
    if (new_status_leds)
    {
        auto & status_leds = io_.statusLeds();
        auto led_states = status_leds_.ledStates();
        for (std::size_t i = 0; i != StatusLeds::LED_COUNT; ++i)
        {
            const bool led_on = 0u != (led_states & 0x1u);
            led_states >>= 1u;
            status_leds.setLed(static_cast<StatusLeds::LedId>(i), led_on);
        }
        status_leds.update();
    }
}

void Lights::switchAnimation(int dir, std::uint32_t current_time)
{
    const std::size_t next_id = changeAnimation(animation_.slotId(), dir);
    settings_.changeAnimation(next_id, current_time);
}

