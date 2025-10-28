#include "app/animation_storage.hpp"

#include "animation/rainbow.hpp"
#include "animation/color.hpp"


AnimationStorage::AnimationStorage():
    storage_{TypeTag<RainbowAnimation>{}}
{
}

auto AnimationStorage::change(AnimationId id) -> AnimationId
{
    switch (id)
    {
    case 0u:
    default:
        storage_.create<RainbowAnimation>();
        return 0u;
    case 1u:
    case static_cast<AnimationId>(-1):
        storage_.create<ColorAnimation>();
        return 1u;
    }
    return id;
}
