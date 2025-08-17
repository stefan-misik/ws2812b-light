#include "driver/ir_receiver.hpp"

#include <utility>

#include "stm32g0xx_ll_tim.h"
#include "stm32g0xx_ll_dma.h"


namespace driver
{
namespace
{

enum class Symbol
{
    BIT_ZERO = 0,
    BIT_ONE = 1,
    START,
    REPEAT,
    INVALID,
};


class BitBuffer
{
public:
    static const inline std::size_t BUFFER_LENGTH = IrReceiver::BUFFER_LENGTH;

    void * buffer() { return buffer_; }

    std::pair<const std::uint8_t *, std::size_t> peek(std::size_t dma_data_length) const
    {
        const std::size_t write_pos = BUFFER_LENGTH - dma_data_length;
        const std::uint8_t * const pos_ptr = buffer_ + pos_;

        if (write_pos >= pos_)
            return {pos_ptr, write_pos - pos_};
        else
            return {pos_ptr, BUFFER_LENGTH - pos_};
    }

    void process(std::size_t byte_cnt)
    {
        pos_ += byte_cnt;
        if (pos_ >= BUFFER_LENGTH)
            pos_ = 0;
    }

private:
    std::size_t pos_ = 0;
    std::uint8_t buffer_[BUFFER_LENGTH];
};


class PacketState
{
public:
    using Code = IrReceiver::Code;

    /**
     * @brief Process IR symbol
     * @param symbol Received symbol
     * @return Is done
     */
    [[nodiscard]] bool processSymbol(Symbol symbol)
    {
        if (-1 == pos_)
        {
            if (Symbol::START != symbol)
                return false;
        }

        switch (symbol)
        {
        case Symbol::BIT_ONE:
            buffer_[pos_ >> 3] |= (1 << (pos_ & 7));
            [[fallthrough]];
        case Symbol::BIT_ZERO:
            pos_ += 1;
            if (32 == pos_)
            {
                pos_ = -1;
                return true;
            }
            break;

        case Symbol::START:
            pos_ = 0;
            for (auto & b: buffer_)
                b = 0;
            break;

        case Symbol::REPEAT:
        case Symbol::INVALID:
            reset();
            break;
        }
        return false;
    }


    Code checkAndGet() const
    {
        if (!check(buffer_[0], buffer_[1]) || !check(buffer_[2], buffer_[3]))
            return {};
        return Code(buffer_[0], buffer_[2]);
    }

    void clear()
    {
        // Mark data as cleared by making sure an invalid code is stored in the
        // buffer
        buffer_[0] = 0;
        buffer_[1] = 0;
    }

    void reset()
    {
        pos_ = -1;
    }

private:
    std::int32_t pos_ = -1;
    std::uint8_t buffer_[4] = {0, 0, 0, 0};

    static bool check(std::uint8_t value, std::uint8_t inverse)
    {
        return value == static_cast<std::uint8_t>(~inverse);
    }
};


class RepeatTimer
{
public:
    void reset(std::uint32_t time)
    {
        time_ = time;
    }

    bool isWithinTime(std::uint32_t period, std::uint32_t time) const
    {
        return (time - time_) <= period;
    }

private:
    std::uint32_t time_;
};


const std::uint32_t PERIOD_CHANNEL = LL_TIM_CHANNEL_CH1;
const std::uint32_t WIDTH_CHANNEL = LL_TIM_CHANNEL_CH2;

const std::uint32_t REPEAT_TIME = 120;

// Sample at 64 MHz / 32 = 2 MHz, 8 consecutive events required to change value, i.e. signal
// needs to be stable for 0.5 us * 8 = 4 us, we are measuring time with frequency of 10 kHz, i.e. in multiples of
// 100 us.
const std::uint32_t IC_FILTER = LL_TIM_IC_FILTER_FDIV32_N8;


constexpr const std::uint8_t LEAD_WIDTH = 90;  // 9 ms
constexpr const std::uint8_t START_PERIOD = LEAD_WIDTH + 45;  // 9 ms + 4.5 ms
constexpr const std::uint8_t REPEAT_PERIOD = LEAD_WIDTH + 22;

constexpr const std::uint8_t BIT_WIDTH = 6;
constexpr const std::uint8_t BIT_ZERO_PERIOD = 11;
constexpr const std::uint8_t BIT_ONE_PERIOD = 22;



::TIM_TypeDef * toTimer(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_3: return TIM3;
    default: break;
    }
    return nullptr;
}

std::uint32_t toDmaRequest(TimerId tim_id)
{
    switch (tim_id)
    {
    case TimerId::TIM_3: return LL_DMAMUX_REQ_TIM3_CH1;
    default: break;
    }

    return 0;
}


bool initializeTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_InitTypeDef init;
    ::LL_TIM_StructInit(&init);
    init.Prescaler = 6400 - 1;  // 64 MHz / 6400 = 10 kHz
    init.CounterMode = LL_TIM_COUNTERDIRECTION_UP;
    init.Autoreload = 0xFF;  // Limit to 8-bits
    init.ClockDivision = LL_TIM_CLOCKDIVISION_DIV1;
    init.RepetitionCounter = 0;

    if (::ErrorStatus::SUCCESS != ::LL_TIM_Init(tim, &init))
        return false;

    ::LL_TIM_DisableARRPreload(tim);
    ::LL_TIM_SetClockSource(tim, LL_TIM_CLOCKSOURCE_INTERNAL);
    ::LL_TIM_SetTriggerOutput(tim, LL_TIM_TRGO_RESET);
    ::LL_TIM_SetTriggerOutput2(tim, LL_TIM_TRGO2_RESET);
    ::LL_TIM_DisableMasterSlaveMode(tim);

    // 1. Select TI1 source
    ::LL_TIM_SetRemap(tim, LL_TIM_TIM3_TI1_RMP_GPIO | LL_TIM_TIM3_TI2_RMP_GPIO | LL_TIM_TIM3_TI3_RMP_GPIO);

    // 2. - 3. Configure Period channel
    {
        ::LL_TIM_IC_InitTypeDef ic_init;
        ::LL_TIM_IC_StructInit(&ic_init);
        ic_init.ICPolarity = LL_TIM_IC_POLARITY_FALLING;
        ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_DIRECTTI;  // TI1
        ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
        ic_init.ICFilter = IC_FILTER;

        if (::ErrorStatus::SUCCESS != ::LL_TIM_IC_Init(tim, PERIOD_CHANNEL, &ic_init))
            return false;
    }

    // 4. - 5. Configure Width channel
    {
        ::LL_TIM_IC_InitTypeDef ic_init;
        ::LL_TIM_IC_StructInit(&ic_init);
        ic_init.ICPolarity = LL_TIM_IC_POLARITY_RISING;
        ic_init.ICActiveInput = LL_TIM_ACTIVEINPUT_INDIRECTTI;  // TI1
        ic_init.ICPrescaler = LL_TIM_ICPSC_DIV1;
        ic_init.ICFilter = IC_FILTER;

        if (::ErrorStatus::SUCCESS != ::LL_TIM_IC_Init(tim, WIDTH_CHANNEL, &ic_init))
            return false;
    }

    // 6. - 7. Slave mode - reset by edge of T1
    ::LL_TIM_SetTriggerInput(tim, LL_TIM_TS_TI1FP1);
    ::LL_TIM_SetSlaveMode(tim, LL_TIM_SLAVEMODE_RESET);

    // 8. Enable the captures
    ::LL_TIM_CC_EnableChannel(tim, PERIOD_CHANNEL);
    ::LL_TIM_CC_EnableChannel(tim, WIDTH_CHANNEL);

    return true;
}

inline void startTimer(::TIM_TypeDef * tim)
{
    ::LL_TIM_EnableCounter(tim);
}

bool initializeDma(std::uint32_t dma_channel, std::uint32_t request, ::TIM_TypeDef * tim)
{
    {
        const std::uint32_t dmamux_channel = toDmamuxChannel(dma_channel);
        ::LL_DMAMUX_SetRequestID(DMAMUX1, dmamux_channel, request);
        ::LL_DMAMUX_DisableEventGeneration(DMAMUX1, dmamux_channel);
        ::LL_DMAMUX_DisableSync(DMAMUX1, dmamux_channel);
    }

    ::LL_DMA_DisableChannel(DMA1, dma_channel);
    ::LL_DMA_ConfigTransfer(DMA1, dma_channel, LL_DMA_DIRECTION_PERIPH_TO_MEMORY | LL_DMA_MODE_CIRCULAR |
            LL_DMA_PERIPH_NOINCREMENT | LL_DMA_MEMORY_INCREMENT |
            LL_DMA_PDATAALIGN_HALFWORD | LL_DMA_MDATAALIGN_BYTE |
            LL_DMA_PRIORITY_MEDIUM);
    ::LL_DMA_SetPeriphAddress(DMA1, dma_channel, reinterpret_cast<std::uint32_t>(&(tim->DMAR)));
    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, 0);
    ::LL_DMA_SetDataLength(DMA1, dma_channel, 0);

    ::LL_DMA_DisableIT_TC(DMA1, dma_channel);
    ::LL_DMA_DisableIT_HT(DMA1, dma_channel);
    ::LL_DMA_DisableIT_TE(DMA1, dma_channel);

    ::LL_TIM_ConfigDMABurst(tim, LL_TIM_DMABURST_BASEADDR_CCR1, LL_TIM_DMABURST_LENGTH_2TRANSFERS);
    ::LL_TIM_EnableDMAReq_CC1(tim);
    return true;
}

inline void startDma(std::uint32_t dma_channel, const void * data, std::size_t length)
{
    ::LL_DMA_DisableChannel(DMA1, dma_channel);

    ::LL_DMA_SetMemoryAddress(DMA1, dma_channel, reinterpret_cast<std::uint32_t>(data));
    ::LL_DMA_SetDataLength(DMA1, dma_channel, length);

    ::LL_DMA_EnableChannel(DMA1, dma_channel);
}

inline std::size_t getDmaDataLength(std::uint32_t dma_channel)
{
    return ::LL_DMA_GetDataLength(DMA1, dma_channel);
}

template <std::uint8_t EXPECTED>
inline bool classifyTimeLength(std::uint8_t value)
{
    static const std::uint8_t NEIGHBORHOOD = 2;
    const std::uint8_t diff = value - (EXPECTED - NEIGHBORHOOD);
    return diff <= (NEIGHBORHOOD * 2);
}

Symbol classifySymbol(const std::uint8_t * buffer)
{
    const std::uint8_t period = buffer[0];
    const std::uint8_t width = buffer[1];

    if (classifyTimeLength<BIT_WIDTH>(width)) [[likely]]
    {
        if (classifyTimeLength<BIT_ZERO_PERIOD>(period))
            return Symbol::BIT_ZERO;
        else if (classifyTimeLength<BIT_ONE_PERIOD>(period))
            return Symbol::BIT_ONE;
    }
    else if (classifyTimeLength<LEAD_WIDTH>(width))
    {
        if (classifyTimeLength<START_PERIOD>(period))
            return Symbol::START;
        else if (classifyTimeLength<REPEAT_PERIOD>(period))
            return Symbol::REPEAT;
    }
    return Symbol::INVALID;
}

}  // namespace


struct IrReceiver::Private
{
    ::TIM_TypeDef * tim;

    std::uint32_t dma_channel;

    BitBuffer buffer;
    PacketState packet_state;
    RepeatTimer repeat_timer;
};


IrReceiver::IrReceiver()
{
}

IrReceiver::~IrReceiver()
{
}


bool IrReceiver::initialize(TimerId tim_id, DmaChannelId dma_channel_id)
{
    auto * const tim = toTimer(tim_id);
    if (nullptr == tim)
        return false;

    if (!initializeTimer(tim))
        return false;

    const std::uint32_t dma_request = toDmaRequest(tim_id);
    const std::uint32_t dma_channel = toDmaChannel(dma_channel_id);
    if (0 == dma_request || INVALID_DMA_CHANNEL == dma_channel)
        return false;

    if (!initializeDma(dma_channel, dma_request, tim))
        return false;

    auto & priv = *p_;
    priv.tim = tim;
    priv.dma_channel = dma_channel;

    startDma(dma_channel, priv.buffer.buffer(), priv.buffer.BUFFER_LENGTH);
    startTimer(tim);

    return true;
}

auto IrReceiver::read(std::uint32_t time) -> std::pair<Code, bool>
{
    auto & priv = *p_;

    bool is_new = false;
    while (true)
    {
        auto [buffer, length] = priv.buffer.peek(getDmaDataLength(priv.dma_channel));

        if (length & 0x1)
        {
            // Just in case we manage to hit in the middle of DMA burst, do not
            // process the odd byte at the end
            length -= 1;
        }

        if (0 == length)
        {
            // There is no more data, we are done
            break;
        }

        const std::uint8_t * pos = buffer;
        const std::uint8_t * const end = buffer + length;  // Length is guaranteed to be even number here
        for (; pos != end; pos += 2)
        {
            const Symbol symbol = classifySymbol(pos);

            if (Symbol::REPEAT == symbol)
                priv.repeat_timer.reset(time);

            if (priv.packet_state.processSymbol(symbol))
            {
                priv.repeat_timer.reset(time);
                is_new = true;
                current_code_ = priv.packet_state.checkAndGet();

                pos += 2;  // The current two bytes have been processed
                break;
            }
        }

        priv.buffer.process(pos - buffer);
        if (is_new)
            break;
    };

    // Clear button code, when button is released
    const Code code = current_code_;
    if (code.isValid())
    {
        if (!priv.repeat_timer.isWithinTime(REPEAT_TIME, time))
        {
            current_code_ = Code::Invalid();
            return {Code::Invalid(), true};
        }
    }

    return {code, is_new};
}


}  // namespace driver
