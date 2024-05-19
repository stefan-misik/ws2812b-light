/**
 * @file
 */

#ifndef ANIMATIONS_TWINKLE_H_
#define ANIMATIONS_TWINKLE_H_

#include "animation.h"

class TwinkleAnimation: public Animation
{
public:
    TwinkleAnimation(const LedState * sequence):
        sequence_(sequence),
        frequency_(3)
    { }

    uint8_t handleEvent(Event type, Param param, SharedStorage * storage) override;

private:
    const LedState * const sequence_;
    uint8_t frequency_;

    class Blink
    {
    public:
        bool isDone() const { return 0 == state_; }
        uint8_t position() const { return state_ - 1; }
        void next() { ++state_; }
        void done() { state_ = 0; }

        void start(LedSize led) { led_ = led; state_ = 1; }

        LedSize led() const { return led_; }

    private:
        LedSize led_;
        uint8_t state_;
    };

    struct Shared
    {
        static const uint8_t BLINK_CNT = 7;
        Blink blinks[BLINK_CNT];
        uint8_t step = 0;
    };

    static bool isSentinel(const LedState & led)
    {
        return 0 == led.red && 0 == led.green && 0 == led.blue;
    }

    void paintBackground(AbstractLedStrip * leds);

    void paintTwinkles(AbstractLedStrip * leds, Shared * shared);
};

#endif  // ANIMATIONS_TWINKLE_H_
