/**
 * @file
 */

#ifndef APP_INPUT_HPP_
#define APP_INPUT_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>

#include "tools/polymorphic_storage.hpp"
#include "app/event_queue.hpp"


/**
 * @brief Key-input object
 */
class Input
{
public:
    enum class KeyId: uint8_t
    {
        KEY_O, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_X,
        KEY_POWER,
        KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_BLUE,
        KEY_HOME, KEY_OK, KEY_BACK,
        KEY_MENU,
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
        KEY_MUTE, KEY_BACKSPACE,

        KEY_NONE,
        KEY_COUNT_ = KEY_NONE,
    };

    static const inline std::size_t KEY_COUNT = static_cast<std::size_t>(KeyId::KEY_COUNT_);

    static const inline std::size_t SOURCE_COUNT = 2;
    static const inline std::size_t INVALID_SOURCE_ID = SOURCE_COUNT;

    /**
     * @brief Type used to represent a list of pressed buttons
     */
    class PressedButtonList
    {
    public:
        using KeyId = Input::KeyId;

        struct PressedButton
        {
            KeyId key_id;
            bool is_new;
        };

        static const std::size_t MAX_LENGTH = 16;

        void resetAll()
        {
            count_ = 0;
        }

        bool addKey(KeyId key, bool is_new = false)
        {
            if (count_ >= MAX_LENGTH)
                return false;
            list_[count_++] = {key, is_new};
            return true;
        }

        std::size_t count() const { return count_; }
        const PressedButton * list() const { return list_; }

    private:
        PressedButton list_[MAX_LENGTH];
        std::size_t count_ = 0;
    };


    /**
     * @brief Type representing state of single button
     */
    class KeyState
    {
    private:
        enum class PressState: std::uint8_t
        {
            NONE, PRESSED, NEWLY_PRESSED
        };

    public:
        enum StateFlags : std::uint8_t
        {
            /** @brief Indicates current state of the button */
            PRESSED = 0x01,
            /** @brief Set in an update cycle when the button is first pressed */
            DOWN = 0x02,
            /** @brief Set in an update cycle when the button is released */
            UP = 0x04,
            /** @brief Set every update cycle a press event is generated */
            PRESS = 0x08
        };

        static const inline std::uint8_t REPEAT_THRESHOLD = 1 + 50;
        static const inline std::uint8_t NEXT_REPEAT_THRESHOLD = REPEAT_THRESHOLD + 8;

        KeyState() = default;

        std::size_t sourceId() const { return source_id_; }
        std::uint8_t flags() const { return flags_; }
        std::uint8_t repeat() const { return repeat_; }

        /**
         * @brief Mark key-press from given source
         *
         * This only marks down the key-press, it is necessary to call
         * @ref process() to update the state of the key.
         *
         * @param source_id Source reporting the key-press
         * @param is_new Source is reporting freshly pressed button
         */
        void markPressedBy(std::size_t source_id, bool is_new)
        {
            if (INVALID_SOURCE_ID == source_id_)
            {
                source_id_ = source_id;
                state_ = PressState::NEWLY_PRESSED;
            }
            else if (source_id == source_id_)
            {
                if (is_new)
                    state_ = PressState::NEWLY_PRESSED;
                else if (PressState::NEWLY_PRESSED != state_)  // Prevent regular press to mask-out newly pressed event
                    state_ = PressState::PRESSED;
            }
        }

        /**
         * @brief Process marked key presses
         */
        void process();

    private:
        std::uint8_t counter_ = 0;
        std::uint8_t source_id_ = INVALID_SOURCE_ID;
        std::uint8_t flags_ = 0;
        std::uint8_t repeat_ = 0;
        PressState state_ = PressState::NONE;

        PressState readPressState()
        {
            const auto state = state_;
            state_ = PressState::NONE;
            return state;
        }
    };

    struct EventParam
    {
        using KeyFlags = KeyState::StateFlags;

        static const inline EventType TYPE = EventType::KEY_EVENT;

        KeyId key;
        std::uint8_t flags;
        std::uint8_t repeat;
        std::uint8_t source_id;
    };

    /**
     * @brief Interface for input sources
     */
    class Source
    {
    public:
        /**
         * @brief Function called periodically to obtain list of pressed keys
         *
         * @param time Current time
         * @param[out] buttons List which will obtain pressed keys
         */
        virtual void getPressedKeys(std::uint32_t time, PressedButtonList * buttons)
        {
            (void) time; (void) buttons;
        }

        virtual ~Source() = default;
    };

    template <typename T, typename... Ts>
    void createSource(std::size_t source, Ts &&... args)
    {
        sources_[source].create<T>(std::forward<Ts>(args)...);
    }

    /**
     * @brief Call periodically to obtain list of pressed keys
     *
     * @param time Current time
     * @param[out] event_queue Event queue that will receive input events
     */
    void update(std::uint32_t time, EventQueue * event_queue);

private:
    using SourceStorage = PolymorphicStorage<Source, 16>;

    SourceStorage sources_[SOURCE_COUNT];

    KeyState keys_[KEY_COUNT];
};


#endif  // APP_INPUT_HPP_
