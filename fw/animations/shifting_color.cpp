#include "animations/shifting_color.h"

#include <avr/pgmspace.h>

using Segment = ShiftingColorAnimation::Segment;

inline void readPgmSegment(Segment * segment, const Segment * pgm_segment)
{
    segment->color.red = pgm_read_byte(&pgm_segment->color.red);
    segment->color.green = pgm_read_byte(&pgm_segment->color.green);
    segment->color.blue = pgm_read_byte(&pgm_segment->color.blue);
    segment->length = pgm_read_byte(&pgm_segment->length);
}

uint8_t ShiftingColorAnimation::handleEvent(Event type, Param param, SharedStorage * storage)
{
    auto s = [=]() -> auto & { return *storage->get<Shared>(); };

    switch (type)
    {
    case Event::START:
        storage->create<Shared>();
        return Result::IS_OK;

    case Event::UPDATE:
        ++(s().step);
        if (4 != s().step)
            return Result::IGNORE_DEFAULT;
        s().step = 0;
        render(&(param.ledStrip()), s().offset);
        s().offset = param.ledStrip().nextId(s().offset);
        return Result::IS_OK;

    case Event::STOP:
        return Result::IS_OK;

    case Event::BUTTON:
        if (param.paramLo() & ButtonState::PRESS)
        {
            switch (param.buttonId())
            {
            case ButtonId::UP: break;
            case ButtonId::DOWN: break;
            }
        }
        return Result::IS_OK;
    }
    return Result::IS_OK;
}

void ShiftingColorAnimation::render(AbstractLedStrip * leds, LedSize offset)
{
    const LedSize last = leds->prevId(offset);
    LedSize pos = offset;
    const Segment * cur_seg = segments_;
    Segment seg;
    readPgmSegment(&seg, cur_seg);
    do
    {
        if (0 == seg.length)
        {
            ++cur_seg;
            readPgmSegment(&seg, cur_seg);
            if (0 == seg.length)
            {
                cur_seg = segments_;
                readPgmSegment(&seg, cur_seg);
            }
        }
        else
            --seg.length;

        (*leds)[pos] = seg.color;
        pos = leds->nextId(pos);
    } while (pos != last);
}
