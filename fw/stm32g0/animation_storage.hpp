/**
 * @file
 */

#ifndef ANIMATION_STORAGE_HPP_
#define ANIMATION_STORAGE_HPP_

#include "tools/polymorphic_storage.hpp"
#include "animation_register.hpp"


class AnimationStorage
{
public:
    using AnimationId = AnimationRegister::AnimationId;
    using Storage = PolymorphicStorage<Animation, 64>;

    AnimationStorage();

    Animation * get() { return storage_.get(); }
    const Animation * get() const { return storage_.get(); }
    Animation * operator ->() { return get(); }
    const Animation * operator ->() const { return get(); }
    Animation & operator *() { return *get(); }
    const Animation & operator *() const { return *get(); }

    AnimationId change(AnimationId id);

private:
    Storage storage_;
};


#endif  // ANIMATION_STORAGE_HPP_
