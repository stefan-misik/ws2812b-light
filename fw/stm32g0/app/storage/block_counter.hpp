/**
 * @file
 */

 #include <cstddef>
 #include <cstdint>

 #include <optional>


class BlockCounter
{
public:
    using CounterType = std::uint16_t;

    void reset(std::uint16_t address = 0)
    {
        state_ = State::UNKNOWN;
        address_ = address;
        counter_ = 0;
    }

    CounterType current() const { return counter_; }
    std::uint16_t address() const { return address_; }

    void next()
    {
        address_ += 1;
        counter_ = current() + 1;
    }

    void rewind(std::optional<CounterType> number);

private:
    enum class State: std::uint8_t
    {
        UNKNOWN,
    };

    State state_ = State::UNKNOWN;
    std::uint16_t address_ = 0;
    CounterType counter_ = 0;
};
