#include "animations/shifting_color.h"

#include <avr/pgmspace.h>

#include "tools/color.h"

using Segment = ShiftingColorAnimation::Segment;

class SegmentWalker
{
public:
    SegmentWalker(const Segment * const * all, uint8_t type):
        start_(reinterpret_cast<const Segment *>(pgm_read_ptr(all + type))),
        next_ptr_(start_ + 1)
    {
        readPgm(&current_, start_);
        readPgm(&next_, next_ptr_);
    }

    void next()
    {
        if (0 == current_.length)
        {
            current_ = next_;
            ++next_ptr_;
            if (!readPgm(&next_, next_ptr_))
            {
                next_ptr_ = start_;
                readPgm(&next_, next_ptr_);
            }
        }
        --current_.length;
    }

    const LedState & color() const { return current_.color; }
    LedSize transitionLength() const { return current_.transition_length; }

    bool isTransitioning() const
    {
        return current_.length < current_.transition_length;
    }

    LedSize transitionPosition() const
    {
        return current_.transition_length - current_.length;
    }

    const LedState & nextColor() const { return next_.color; }

private:
    const Segment * const start_;
    const Segment * next_ptr_;
    Segment current_;
    Segment next_;

    static bool readPgm(Segment * segment, const Segment * pgm_segment)
    {
        const LedSize length = pgm_read_byte(&pgm_segment->length);
        if (0 == length)
            return false;

        segment->color.red = pgm_read_byte(&pgm_segment->color.red);
        segment->color.green = pgm_read_byte(&pgm_segment->color.green);
        segment->color.blue = pgm_read_byte(&pgm_segment->color.blue);
        segment->length = length;
        segment->transition_length = pgm_read_byte(&pgm_segment->transition_length);

        return true;
    }
};

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
            case ButtonId::DOWN: delay_ = 15 == delay_ ? 1 : delay_ + 1; s().step = 0; break;
            }
        }
        return Result::IS_OK;

    case Event::SAVE_CONFIG:
    {
        auto & data = param.saveConfigurationData();
        data.data[0] = delay_;
        data.data[1] = type_;
        return Result::IS_OK;

    }

    case Event::LOAD_CONFIG:
    {
        const auto & data = param.loadConfigurationData();
        delay_ = data.data[0];
        type_ = data.data[1];
        return Result::IS_OK;
    }
    }
    return Result::IS_OK;
}

void ShiftingColorAnimation::render(AbstractLedStrip * leds, LedSize offset)
{
    const LedSize last = leds->prevId(offset);
    LedSize pos = offset;
    SegmentWalker segw{segments_, type_};

    while (true)
    {
        segw.next();

        LedState color = segw.color();
        if (segw.isTransitioning())
            blendColors(&color, segw.nextColor(), segw.transitionPosition(), segw.transitionLength());
        (*leds)[pos] = color;

        if (pos == last)
            break;
        pos = leds->nextId(pos);
    }
}
