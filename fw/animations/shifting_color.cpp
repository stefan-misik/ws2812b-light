#include "animations/shifting_color.h"

#include <avr/pgmspace.h>

#include "tools/color.h"
#include "tools/transition_sequence.h"

using Segment = ShiftingColorAnimation::Segment;

inline void readPgmSegment(Segment * segment, const Segment * pgm_segment)
{
    segment->color.red = pgm_read_byte(&pgm_segment->color.red);
    segment->color.green = pgm_read_byte(&pgm_segment->color.green);
    segment->color.blue = pgm_read_byte(&pgm_segment->color.blue);
    segment->length = pgm_read_byte(&pgm_segment->length);

    segment->secondary.red = pgm_read_byte(&pgm_segment->secondary.red);
    segment->secondary.green = pgm_read_byte(&pgm_segment->secondary.green);
    segment->secondary.blue = pgm_read_byte(&pgm_segment->secondary.blue);
    segment->transition = pgm_read_byte(&pgm_segment->transition);
}

inline uint8_t nextType(const Segment * const * segments, uint8_t current)
{
    const uint8_t next = current + 1;
    if (nullptr == pgm_read_ptr(segments + next))
        return 0;
    return next;
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
        if (delay_ != s().step)
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
            case ButtonId::UP: type_ = nextType(segments_, type_); break;
            case ButtonId::DOWN: delay_ = 15 == delay_ ? 0 : delay_ + 1; break;
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
    const Segment * const type_seg = reinterpret_cast<const Segment *>(pgm_read_ptr(segments_ + type_));
    const Segment * cur_seg = type_seg;
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
                cur_seg = type_seg;
                readPgmSegment(&seg, cur_seg);
            }
        }
        else
            --seg.length;

        LedState color = seg.color;
        if (seg.length < seg.transition)
        {
            blendColors(&color, seg.secondary, makeTransitionSequence(seg.transition - seg.length, seg.transition));
        }
        (*leds)[pos] = color;
        pos = leds->nextId(pos);
    } while (pos != last);
}
