/**
 * @file
 */

#ifndef APP_ANIMATION_RETRO_HPP_
#define APP_ANIMATION_RETRO_HPP_

#include "app/animation.hpp"
#include <cstdint>


class RetroAnimation final:
        public Animation
{
public:
    static const inline std::uint8_t VARIANT_CNT = 4;

    enum ParamId: std::uint32_t
    {
        VARIANT = Animation::ParamId::FIRST_CUSTOM_ID_,
    };

    void render(AbstractLedStrip * strip, Flags<RenderFlag> flags) override;

    bool setParamater(std::uint32_t param_id, int value, ChangeType type = ChangeType::ABSOLUTE) override;
    std::optional<int> getParameter(std::uint32_t param_id) override;

    std::size_t store(void * buffer, std::size_t capacity, DataType type) const override;
    std::size_t restore(const void * buffer, std::size_t max_size, DataType type) override;

private:
    struct Configuration
    {
        std::uint8_t variant = 0u;
    };

    struct State
    {
        std::uint8_t state = 0u;
    };

    Configuration config_;
    State state_;

    std::uint16_t delay_ = 0u;
    bool is_playing_ = false;

    void reset()
    {
        state_.state = 0u;
        delay_ = 0u;
        is_playing_ = false;
    }
};



#endif  // APP_ANIMATION_RETRO_HPP_
