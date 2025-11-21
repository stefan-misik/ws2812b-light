#include "app/animation/shifting_color.hpp"

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "tools/serdes.hpp"
#include "app/tools/animation_parameter.hpp"
#include "app/tools/color.hpp"

namespace
{

using Segment = ShiftingColorAnimation::Segment;


const Segment seq_0[] =
{
        {LedState{0xFF, 0x00, 0x00}, 30, 30},
        {LedState{0x55, 0x55, 0x55}, 70, 30},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment seq_1[] =
{
        {LedState{0xFF, 0x00, 0x00}, 25, 10},
        {LedState{0x00, 0xFF, 0x00}, 25, 10},
        {LedState{0xC0, 0x5F, 0x00}, 25, 10},
        {LedState{0x00, 0x00, 0xFF}, 25, 10},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment seq_2[] =
{
        {LedState{0xFF, 0x8F, 0x00}, 30, 30},
        {LedState{0x3F, 0x1C, 0x00}, 70, 30},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment seq_3[] =
{
        {LedState{0xFF, 0x00, 0x00}, 25, 25},
        {LedState{0x00, 0xFF, 0x00}, 25, 25},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment seq_4[] =
{
        {LedState{0xC0, 0x5F, 0x00}, 25, 10},
        {LedState{0x00, 0x00, 0xFF}, 25, 10},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment * const seqs[] =
{
        seq_0,
        seq_1,
        seq_2,
        seq_3,
        seq_4,
        nullptr
};


static_assert(ShiftingColorAnimation::VARIANT_CNT == (sizeof(seqs) / sizeof(seqs[0])) - 1, "Fix variant count");


class SegmentWalker
{
public:
    SegmentWalker(std::uint8_t type):
        start_(seqs[type]),
        next_ptr_(start_ + 1),
        current_()
    {
        readSegment(&current_, start_);
        readSegment(&next_, next_ptr_);
    }

    void next()
    {
        if (0 == current_.length)
        {
            current_ = next_;
            ++next_ptr_;
            if (!readSegment(&next_, next_ptr_))
            {
                next_ptr_ = start_;
                readSegment(&next_, next_ptr_);
            }
        }
        --current_.length;
    }

    const LedState & color() const { return current_.color; }
    LedSize transitionLength() const { return current_.transition_length + 1; }

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

    static bool readSegment(Segment * segment, const Segment * src_segment)
    {
        const LedSize length = src_segment->length;
        if (0 == length)
            return false;

        *segment = *src_segment;
        return true;
    }
};


template <std::size_t FB>
inline std::uint32_t wholePartFixedPoint(std::uint32_t num)
{
    return num >> FB;
}

template <std::size_t FB>
inline std::uint32_t fractionPartFixedPoint(std::uint32_t num)
{
    return num & (0xFFFFFFFFul >> (32 - FB));
}

template <std::size_t FB>
inline std::uint32_t changeWholeFixedPoint(std::uint32_t num, std::uint32_t whole)
{
    return fractionPartFixedPoint<FB>(num) | (whole << FB);
}

}  // namespace


void ShiftingColorAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    LedSize pos = wholePartFixedPoint<FRACTION_BITS>(state_.offset);
    const LedSize last = strip->prevId(pos);
    SegmentWalker seg_w{config_.variant};

    while (true)
    {
        seg_w.next();

        LedState color = seg_w.color();
        if (seg_w.isTransitioning())
        {
            blendColors(&color, seg_w.nextColor(),
                seg_w.transitionPosition(), seg_w.transitionLength());
        }
        (*strip)[pos] = color;

        if (pos == last)
            break;
        pos = strip->nextId(pos);
    }


    {
        std::uint32_t new_offset = state_.offset + config_.speed;
        std::uint32_t new_pos = wholePartFixedPoint<FRACTION_BITS>(new_offset);
        if (new_pos >= strip->led_count)
            new_pos -= strip->led_count;
        state_.offset = changeWholeFixedPoint<FRACTION_BITS>(new_offset, new_pos);
    }
    (void) flags;
}

bool ShiftingColorAnimation::setParamater(std::uint32_t param_id, int value, ChangeType type)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        config_.speed = setCyclicParameter<decltype(config_.speed), 16, 1>(
            config_.speed, value, type);
        return true;

    case ParamId::VARIANT:
        config_.variant = setCyclicParameter<decltype(config_.variant), VARIANT_CNT - 1>(
            config_.variant, value, type);
        step_ = 0;
        return true;

    default:
        return false;
    }
    return false;
}

std::optional<int> ShiftingColorAnimation::getParameter(std::uint32_t param_id)
{
    switch (param_id)
    {
    case Animation::ParamId::SECONDARY:
    case ParamId::SPEED:
        return static_cast<int>(config_.speed);

    case ParamId::VARIANT:
        return static_cast<int>(config_.variant);

    default:
        return {};
    }
}

std::size_t ShiftingColorAnimation::store(void * buffer, std::size_t capacity, DataType type) const
{
    Serializer ser(buffer, capacity);
    ser.serialize(&config_);
    if (type == DataType::BOTH)
        ser.serialize(&state_);
    return ser.processed(buffer);
}

std::size_t ShiftingColorAnimation::restore(const void * buffer, std::size_t max_size, DataType type)
{
    Deserializer de_ser(buffer, max_size);
    de_ser.deserialize(&config_);
    if (type == DataType::BOTH)
        de_ser.deserialize(&state_);
    return de_ser.processed(buffer);
}
