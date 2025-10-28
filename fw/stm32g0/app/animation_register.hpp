/**
 * @file
 */

#ifndef APP_ANIMATION_REGISTER_HPP_
#define APP_ANIMATION_REGISTER_HPP_

#include <cstddef>
#include <cstdint>

#include "app/animation.hpp"


class AnimationRegister
{
public:
    static const inline std::size_t MAX_STATE_SIZE = 64;

    using AnimationId = std::uint16_t;

    AnimationId animationId() const { return id_; }
    void setAnimationId(AnimationId animid) { id_ = animid; }

    bool store(const Animation * anim, Animation::DataType type)
    {
        size_ = anim->store(data_, sizeof(data_), type);
        return 0u != size_;
    }

    bool restore(Animation * anim, Animation::DataType type) const
    {
        const auto processed = anim->restore(data_, size_, type);
        return 0u != processed;
    }

    void reset()
    {
        size_ = 0;
    }

private:
    AnimationId id_;
    std::uint16_t size_ = 0;

    alignas (void *)
    char data_[MAX_STATE_SIZE];
};



#endif  // APP_ANIMATION_REGISTER_HPP_
