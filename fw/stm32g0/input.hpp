/**
 * @file
 */

#ifndef INPUT_HPP_
#define INPUT_HPP_

#include <cstddef>
#include <cstdint>

#include "tools/polymorphic_storage.hpp"


class Input
{
public:
    enum class KeyId
    {
        KEY_O, KEY_LEFT, KEY_UP, KEY_DOWN, KEY_RIGHT, KEY_X,
        KEY_POWER,
        KEY_RED, KEY_GREEN, KEY_YELLOW, KEY_BLUE,
        KEY_HOME, KEY_BACK,
        KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,
        KEY_MUTE, KEY_BACKSPACE,

        KEY_COUNT_
    };

    class KeyState
    {
    public:
    private:
        std::uint32_t state_ = 0;
    };

    class Source
    {
    public:
        virtual void process() = 0;
        virtual ~Source() = default;
    };

    static const inline std::size_t KEY_COUNT = static_cast<std::size_t>(KeyId::KEY_COUNT_);

private:
    using SourceStorage = PolymorphicStorage<Source, 16>;

    KeyState keys_[KEY_COUNT];
};


#endif  // INPUT_HPP_
