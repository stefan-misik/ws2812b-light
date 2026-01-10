#include "app/animation/tools/color_themes.hpp"

#include <utility>
#include <initializer_list>

#include "app/tools/color.hpp"


namespace
{

template <typename... Ts>
inline bool applyTheme(Animation * animation, std::initializer_list<LedState> colors)
{
    using ParamId = Animation::ParamId;
    return setParameterGroup<LedState, Animation::ColorParam, ParamId::COLOR_THEME_FIRST, ParamId::COLOR_THEME_LENGTH>(
        animation, colors
    );
}

}  // namespace


bool applyColorTheme(Animation * animation, ColorTheme theme)
{
    switch (theme)
    {
        case ColorTheme::CANDY_CANE:
            return applyTheme(animation, {
                LedState(0x55, 0x55, 0x55),
                getColor(ColorId::RED)
            });

        case ColorTheme::BASIC_FOUR_COLORS:
            return applyTheme(animation, {
                getColor(ColorId::RED),
                getColor(ColorId::GREEN),
                getColor(ColorId::YELLOW),
                getColor(ColorId::BLUE)
            });

        case ColorTheme::GOLDEN:
            return applyTheme(animation, {
                LedState(0x3F, 0x1C, 0x00),
                LedState(0xFF, 0x8F, 0x00)
            });

        case ColorTheme::GREEN_AND_RED:
            return applyTheme(animation, {
                getColor(ColorId::GREEN),
                getColor(ColorId::RED)
            });

        case ColorTheme::NIGHT_SKY:
            return applyTheme(animation, {
                getColor(ColorId::BLUE),
                getColor(ColorId::YELLOW)
            });

        case ColorTheme::ICE_AND_MAGENTA:
            return applyTheme(animation, {
                LedState(0x02, 0xF5, 0xC4),
                LedState(0xE9, 0x45, 0xCB)
            });

        case ColorTheme::COUNT_:
            break;
    }
    return false;
}