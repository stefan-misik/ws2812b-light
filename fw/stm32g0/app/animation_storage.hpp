/**
 * @file
 */

#ifndef APP_ANIMATION_STORAGE_HPP_
#define APP_ANIMATION_STORAGE_HPP_

#include <array>

#include "tools/polymorphic_storage.hpp"
#include "app/animation.hpp"


/**
 * @brief Class managing the state of the animations
 */
class AnimationStorage
{
public:
    class Register
    {
    public:
        static const inline std::size_t MAX_STATE_SIZE = 64;

        using AnimationId = std::uint16_t;

        AnimationId animationId() const { return id_; }
        void setAnimationId(AnimationId anim_id) { id_ = anim_id; }

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

    using AnimationSlotId = std::uint16_t;
    using Storage = PolymorphicStorage<Animation, 256>;

    static const inline std::size_t SLOT_COUNT = 26;

    AnimationStorage();

    Animation * get() { return storage_.get(); }
    const Animation * get() const { return storage_.get(); }
    Animation * operator ->() { return get(); }
    const Animation * operator ->() const { return get(); }
    Animation & operator *() { return *get(); }
    const Animation & operator *() const { return *get(); }

    void initialize();
    void initializeCurrentSlot();

    AnimationSlotId slotId() const { return slot_id_; }
    bool change(AnimationSlotId new_slot_id);

private:
    AnimationSlotId slot_id_;
    Storage storage_;
    std::array<Register, SLOT_COUNT> slots_;
};


#endif  // APP_ANIMATION_STORAGE_HPP_
