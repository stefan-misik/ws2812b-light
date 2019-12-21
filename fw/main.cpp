#include "leds.h"

#include <stdint.h>
#include <util/delay.h>

Leds::LedState led_states[30];
Leds leds;

struct Rainbow
{
public:
    uint8_t ca;
    uint8_t cb;
    uint8_t cc;

    Rainbow():
        ca(255),
        cb(0),
        cc(0)
    { }

    void next()
    {
        if (ca > 0)
        {
            if (cc > 0)
            {
                ++ca;
                --cc;
            }
            else
            {
                --ca;
                ++cb;
            }
        }
        else if (cb > 0)
        {
            --cb;
            ++cc;
        }
        else
        {
            ++ca;
            --cc;
        }
    }
};


int main(void)
{
    leds.initialize(led_states, sizeof(led_states) / sizeof(led_states[0]));

    Rainbow rb;

    while(1)
    {
        leds[0] = {rb.ca, rb.cb, rb.cc};
        leds[1] = {rb.cc, rb.ca, rb.cb};
        leds[2] = {rb.cb, rb.cc, rb.ca};
        leds[3] = {rb.ca, rb.cb, rb.cc};
        leds[4] = {rb.cc, rb.ca, rb.cb};
        leds[5] = {rb.cb, rb.cc, rb.ca};
        leds[6] = {rb.ca, rb.cb, rb.cc};
        leds[7] = {rb.cc, rb.ca, rb.cb};
        leds[8] = {rb.cb, rb.cc, rb.ca};
        leds[9] = {rb.ca, rb.cb, rb.cc};
        leds[10] = {rb.cc, rb.ca, rb.cb};
        leds[11] = {rb.cb, rb.cc, rb.ca};
        leds[12] = {rb.ca, rb.cb, rb.cc};
        leds[13] = {rb.cc, rb.ca, rb.cb};
        leds[14] = {rb.cb, rb.cc, rb.ca};
        leds[15] = {rb.ca, rb.cb, rb.cc};
        leds[16] = {rb.cc, rb.ca, rb.cb};
        leds[17] = {rb.cb, rb.cc, rb.ca};
        leds[18] = {rb.ca, rb.cb, rb.cc};
        leds[19] = {rb.cc, rb.ca, rb.cb};
        leds[20] = {rb.cb, rb.cc, rb.ca};
        leds[21] = {rb.ca, rb.cb, rb.cc};
        leds[22] = {rb.cc, rb.ca, rb.cb};
        leds[23] = {rb.cb, rb.cc, rb.ca};
        leds[24] = {rb.ca, rb.cb, rb.cc};
        leds[25] = {rb.cc, rb.ca, rb.cb};
        leds[26] = {rb.cb, rb.cc, rb.ca};
        leds[27] = {rb.ca, rb.cb, rb.cc};
        leds[28] = {rb.cc, rb.ca, rb.cb};
        leds[29] = {rb.cb, rb.cc, rb.ca};
        leds.update();
        _delay_ms(1);
        rb.next();
    }
}
