/**
 * @file
 */

#ifndef MUSIC_H_
#define MUSIC_H_

#include <stdint.h>

#include "buttons.h"


class Music
{
public:
    static void initialize();

    void play(Buttons::ButtonId btn);

private:
    uint8_t octave_ = 0;
    uint8_t note_ = 0;
};


#endif  // MUSIC_H_
