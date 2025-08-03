#include "input.hpp"


namespace
{

}  // namespace


void Input::update(std::uint32_t time)
{
    PressedButtonList list;
    for (std::size_t src_id = 0; src_id != SOURCE_COUNT; ++src_id)
    {
        list.resetAll();
        sources_[src_id]->getPressedKeys(time, &list);
    }
}
