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

const Segment seq_5[] =
{
        {LedState{0x02, 0xF5, 0xC4}, 40, 20},
        {LedState{0xE9, 0x45, 0xCB}, 40, 20},
        {LedState{0x00, 0x00, 0x00}, 0, 0},
};

const Segment * const seqs[] =
{
        seq_0,
        seq_1,
        seq_2,
        seq_3,
        seq_4,
        seq_5,
        nullptr
};


static_assert(ShiftingColorAnimation::VARIANT_CNT == (sizeof(seqs) / sizeof(seqs[0])) - 1, "Fix variant count");


class SequenceWalker
{
public:
    static const inline std::uint8_t FRACTION_BITS = ShiftingColorAnimation::FRACTION_BITS;

    SequenceWalker(const Segment * sequence):
        begin_(sequence),
        end_(findEnd(sequence)),
        current_(sequence)
    { }

    std::uint32_t rewind(std::uint32_t offset)
    {
        const std::uint32_t actual_offset = wrapByTotalLength(offset);

        const Segment * segment = begin_;
        std::uint32_t remaining = actual_offset;

        while (0 != remaining)
        {
            segment = prev(segment);
            const std::uint32_t segment_len = (segment->length << FRACTION_BITS);
            if (remaining < segment_len)
                break;
            remaining -= segment_len;
        }

        current_ = segment;
        state_.fill(segment);
        if (0 != remaining)  // If remaining is 0, the whole segment is actually remaining
            state_.remaining = remaining;
        return actual_offset;
    }

    void step(LedState * color, std::uint32_t post_increment)
    {
        const auto & state = state_;
        const Segment * next_seg = next(current_);

        *color = current_->color;
        if (state.remaining < state.transition_length)
            blendColors(color, next_seg->color, state.transition_length - state.remaining, state.transition_length);


        if (state.remaining <= post_increment)
        {
            post_increment -= state.remaining;
            current_ = next_seg;
            state_.fill(next_seg);
        }
        state_.remaining -= post_increment;
    }

private:
    struct State
    {
        std::uint32_t remaining = 0ul;
        std::uint32_t transition_length = 0ul;

        void fill(const Segment * seg)
        {
            remaining = seg->length << FRACTION_BITS;
            transition_length = seg->transition_length << FRACTION_BITS;
        }
    };

    const Segment * const begin_;
    const Segment * const end_;
    const Segment * current_;
    State state_;

    static const Segment * findEnd(const Segment * sequence)
    {
        const Segment * segment = sequence;
        while (segment->length != 0)
            ++segment;
        return segment;
    }

    std::uint32_t wrapByTotalLength(std::uint32_t offset) const
    {
        const std::uint32_t total_length = totalLength();
        return offset % total_length;
    }

    std::uint32_t totalLength() const
    {
        std::uint32_t total = 0;
        for (auto it = begin_; it != end_; ++it)
            total += (it->length << FRACTION_BITS);
        return total;
    }

    const Segment * next(const Segment * it) const
    {
        ++it;
        if (it == end_)
            return begin_;
        return it;
    }

    const Segment * prev(const Segment * it) const
    {
        if (it == begin_)
            return end_ - 1;
        --it;
        return it;
    }
};

}  // namespace


void ShiftingColorAnimation::render(AbstractLedStrip * strip, Flags<RenderFlag> flags)
{
    SequenceWalker seq_walker(seqs[config_.variant]);

    state_.offset = seq_walker.rewind(state_.offset + (config_.speed << 4));

    for (auto & led : *strip)
        seq_walker.step(&led, 1ul << FRACTION_BITS);

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
        state_.offset = 0;
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
