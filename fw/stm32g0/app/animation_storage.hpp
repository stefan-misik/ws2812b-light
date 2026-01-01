/**
 * @file
 */

#ifndef APP_ANIMATION_STORAGE_HPP_
#define APP_ANIMATION_STORAGE_HPP_

#include "tools/polymorphic_storage.hpp"
#include "app/animation_register.hpp"


class AnimationStorage
{
public:
    using AnimationId = AnimationRegister::AnimationId;
    using Storage = PolymorphicStorage<Animation, 256>;

    static const inline std::size_t ANIMATION_COUNT = 26;

    AnimationStorage();

    Animation * get() { return storage_.get(); }
    const Animation * get() const { return storage_.get(); }
    Animation * operator ->() { return get(); }
    const Animation * operator ->() const { return get(); }
    Animation & operator *() { return *get(); }
    const Animation & operator *() const { return *get(); }

    void change(AnimationId id);

private:
    Storage storage_;
};


#endif  // APP_ANIMATION_STORAGE_HPP_
