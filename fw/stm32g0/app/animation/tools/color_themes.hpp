/**
 * @file
 */

#ifndef APP_ANIMATION_TOOLS_COLOR_THEMES_HPP_
#define APP_ANIMATION_TOOLS_COLOR_THEMES_HPP_

#include "app/animation.hpp"
#include "app/tools/color.hpp"


namespace anim
{

struct ColorParameterSetter
{
    static bool set(Animation * animation, std::uint32_t offset, ColorId color_id)
    {
        LedState color;
        getColor(&color, color_id);
        return animation->setParamater(offset, Animation::ColorParam::make(color));
    }

    static bool set(Animation * animation, std::uint32_t offset, const LedState & color)
    {
        return animation->setParamater(offset, Animation::ColorParam::make(color));
    }
};

template <typename SETTER, typename T, typename... Ts>
inline bool applyParameter(Animation * animation, std::uint32_t offset, T && color_spec, Ts &&... args)
{
    if (!SETTER::set(animation, offset, std::forward<T>(color_spec)))
        return false;

    if constexpr (sizeof...(args) > 0)
        return applyParameter<SETTER>(animation, offset + 1ul, std::forward<Ts>(args)...);
    return true;
}

template <std::uint32_t CNT_PRM, std::uint32_t FST_PRM, typename SETTER, typename... Ts>
inline bool applyParameterGroup(Animation * animation, Ts &&... args)
{
    if (!animation->setParamater(CNT_PRM, sizeof...(args)))
        return false;
    if constexpr (sizeof...(args) > 0)
        return applyParameter<SETTER>(animation, FST_PRM, std::forward<Ts>(args)...);
    return true;
}

}  // namespace anim


/** @brief Color theme names */
enum class ColorTheme
{
    CANDY_CANE,
    BASIC_FOUR_COLORS,
    GOLDEN,
    GREEN_AND_RED,
    NIGHT_SKY,
    ICE_AND_MAGENTA,

    COUNT_
};

static const inline std::size_t COLOR_THEME_COUNT = static_cast<std::size_t>(ColorTheme::COUNT_);


/**
 * @brief Apply color theme to an animation
 *
 * @param animation Animation on which to apply the theme
 * @param theme Theme to apply

 * @return Success
 */
bool applyColorTheme(Animation * animation, ColorTheme theme);



#endif  // APP_ANIMATION_TOOLS_COLOR_THEMES_HPP_

