/**
 * @file
 */

#ifndef INPUT_HPP_
#define INPUT_HPP_

#include <cstddef>
#include <cstdint>
#include <utility>

#include "tools/polymorphic_storage.hpp"


/**
 * @brief Key-input object
 */
class Input
{
public:
    enum class KeyId
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

    /**
     * @brief Type used to represent a list of pressed buttons
     */
    class PressedButtonList
    {
    public:
        using KeyId = Input::KeyId;

        static const std::size_t MAX_LENGTH = 16;

        void resetAll()
        {
            count_ = 0;
        }

        bool addKey(KeyId key)
        {
            if (count_ >= MAX_LENGTH)
                return false;
            list_[count_++] = key;
            return true;
        }

        std::size_t count() const { return count_; }
        const KeyId * list() const { return list_; }

    private:
        KeyId list_[MAX_LENGTH];
        std::size_t count_ = 0;
    };


    /**
     * @brief Type representing state of single button
     */
    class KeyState
    {
    public:
    private:
        std::uint32_t state_ = 0;
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

    static const inline std::size_t SOURCE_COUNT = 2;

    template <typename T, typename... Ts>
    void createSource(std::size_t source, Ts &&... args)
    {
        sources_[source].create<T>(std::forward<Ts>(args)...);
    }

    /**
     * @brief Call periodically to obtain list of pressed keys
     *
     * @param time Current time
     */
    void update(std::uint32_t time);

private:
    using SourceStorage = PolymorphicStorage<Source, 16>;

    SourceStorage sources_[SOURCE_COUNT];

    KeyState keys_[KEY_COUNT];
};


#endif  // INPUT_HPP_
