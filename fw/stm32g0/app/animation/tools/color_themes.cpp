#include "app/animation/tools/color_themes.hpp"

#include <utility>


namespace
{

using ParamId = Animation::ParamId;


template <typename... Ts>
inline bool applyTheme(Animation * animation, Ts &&... args)
{
    using Setter = anim::ColorParameterSetter;
    return anim::applyParameterGroup<ParamId::COLOR_THEME_LENGTH, ParamId::COLOR_THEME_FIRST, Setter>(animation,
        std::forward<Ts>(args)...);
}

}  // namespace


bool applyColorTheme(Animation * animation, ColorTheme theme)
{
    switch (theme)
    {
        case ColorTheme::CANDY_CANE:
            return applyTheme(animation,
                LedState(0x55, 0x55, 0x55),
                ColorId::RED
            );

        case ColorTheme::BASIC_FOUR_COLORS:
            return applyTheme(animation,
                ColorId::RED,
                ColorId::GREEN,
                ColorId::YELLOW,
                ColorId::BLUE
            );

        case ColorTheme::GOLDEN:
            return applyTheme(animation,
                LedState(0x3F, 0x1C, 0x00),
                LedState(0xFF, 0x8F, 0x00)
            );

        case ColorTheme::GREEN_AND_RED:
            return applyTheme(animation,
                ColorId::GREEN,
                ColorId::RED
            );

        case ColorTheme::NIGHT_SKY:
            return applyTheme(animation,
                ColorId::BLUE,
                ColorId::YELLOW
            );

        case ColorTheme::ICE_AND_MAGENTA:
            return applyTheme(animation,
                LedState(0x02, 0xF5, 0xC4),
                LedState(0xE9, 0x45, 0xCB)
            );

        case ColorTheme::COUNT_:
            break;
    }
    return false;
}