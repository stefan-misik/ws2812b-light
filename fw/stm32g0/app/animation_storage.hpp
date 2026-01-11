/**
 * @file
 */

#ifndef APP_ANIMATION_STORAGE_HPP_
#define APP_ANIMATION_STORAGE_HPP_

#include <array>

#include "tools/polymorphic_storage.hpp"
#include "app/animation.hpp"
#include "app/animation/tools/color_themes.hpp"


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

    enum AnimationSlotName: AnimationSlotId
    {
        ANIM_SLOT_COLOR,
        ANIM_SLOT_COLOR_LAST = ANIM_SLOT_COLOR,
        ANIM_SLOT_RAINBOW,
        ANIM_SLOT_RAINBOW_LAST = ANIM_SLOT_RAINBOW,
        ANIM_SLOT_RETRO,
        ANIM_SLOT_RETRO_LAST = ANIM_SLOT_RETRO + (4 - 1),
        ANIM_SLOT_TWINKLE,
        ANIM_SLOT_TWINKLE_LAST = ANIM_SLOT_TWINKLE + (3 - 1),
        ANIM_SLOT_SHIFTING_COLOR,
        ANIM_SLOT_SHIFTING_COLOR_LAST = ANIM_SLOT_SHIFTING_COLOR + (COLOR_THEME_COUNT - 1),
        ANIM_SLOT_LIGHTS,
        ANIM_SLOT_LIGHTS_LAST = ANIM_SLOT_LIGHTS + ((COLOR_THEME_COUNT * 2) - 1),

        ANIM_SLOT_COUNT_,
    };

    static const inline std::size_t SLOT_COUNT = static_cast<std::size_t>(AnimationSlotName::ANIM_SLOT_COUNT_);

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
    bool change(AnimationSlotName new_slot_name)
    {
        return change(static_cast<AnimationSlotId>(new_slot_name));
    }

private:
    AnimationSlotId slot_id_;
    Storage storage_;
    std::array<Register, SLOT_COUNT> slots_;
};


#endif  // APP_ANIMATION_STORAGE_HPP_
