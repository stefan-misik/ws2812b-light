/**
 * @file
 */

#ifndef APP_ANIMATION_TOOLS_COLOR_THEMES_HPP_
#define APP_ANIMATION_TOOLS_COLOR_THEMES_HPP_

#include <cstddef>

#include "app/animation.hpp"
#include "app/animation/tools/animation_tools.hpp"



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

