#include "driver/i2c_bus/cat24cx.hpp"

#include "driver/tools/irq.hpp"


namespace driver
{
namespace i2c
{
namespace
{

auto toResult(I2cBus::Transaction::Status status) -> Cat24cx::Result
{
    using Status = I2cBus::Transaction::Status;
    using Result = Cat24cx::Result;
    switch (status)
    {
    case Status::DONE: return Result::SUCCESS;
    case Status::ERR_NO_ANSWER: return Result::ERR_NO_ANSWER;
    case Status::ERR_ARBITRATION_LOST: return Result::ERR_ARBITRATION_LOST;
    case Status::ERR_BUS_ERROR: return Result::ERR_BUS_ERROR;
    }
    return Result::ERR_BUS_ERROR;
}

void setWordAddress(I2cBus::Transaction * transaction, std::uint16_t word_address, std::uint8_t address_length)
{
    auto * const bytes = static_cast<std::uint8_t *>(transaction->localBuffer());
    if (2 == address_length)
    {
        bytes[0] = static_cast<std::uint8_t>(word_address >> 8);
        bytes[1] = static_cast<std::uint8_t>(word_address);
    }
    else
    {
        bytes[0] = static_cast<std::uint8_t>(word_address);
    }
}

}  // namespace


bool Cat24cx::configure(const Config & config)
{
    if (isBusy() || (1 != config.address_length && 2 != config.address_length) ||
            0 == config.page_size || config.page_size > MAX_PAGE_SIZE ||
            0 == config.capacity || config.capacity % config.page_size != 0 ||
            config.device_address < 0x50 || config.device_address > 0x57)
        return false;

    const std::uint32_t address_capacity = (1 == config.address_length) ? 256u : 65536u;
    if (config.capacity > address_capacity)
        return false;

    config_ = config;
    result_ = Result::SUCCESS;
    state_ = State::IDLE;
    return true;
}

bool Cat24cx::startPage(std::uint32_t page, State state)
{
    if (isBusy() || page >= pageCount())
        return false;

    word_address_ = static_cast<std::uint16_t>(page * config_.page_size);
    state_ = state;
    result_ = Result::SUCCESS;
    return true;
}

void Cat24cx::handleResponse(const I2cBus::Transaction * transaction, std::uint32_t current_time)
{
    if (nullptr == transaction || transaction->address() != address())
        return;

    if (!isBusy())
        return;  // Ignore unexpected responses

    const Result response = toResult(transaction->status());
    const auto request = static_cast<TransactionKind>(transaction->opaque());

    const auto check = [&, this](TransactionKind expected_request) -> bool
    {
        if (request != expected_request)
        {
            finish(Result::ERR_BAD_ANSWER);
            return false;
        }
        if (response >= Result::FIRST_ERROR_)
        {
            finish(response);
            return false;
        }
        return true;
    };

    switch (state_)
    {
    case State::READ_IN_FLIGHT:
        if (!check(TransactionKind::READ_ADDRESS))
            return;
        state_ = State::READ_DATA_IN_FLIGHT;
        break;

    case State::READ_DATA_IN_FLIGHT:
        if (!check(TransactionKind::READ_DATA))
            return;
        finish();
        break;

    case State::WRITE_IN_FLIGHT:
        if (!check(TransactionKind::WRITE))
            return;
        poll_interval_.start(current_time);
        poll_timeout_.start(current_time);
        state_ = State::POLL_PENDING;
        break;

    case State::POLL_IN_FLIGHT:
        if (request != TransactionKind::POLL)
        {
            finish(Result::ERR_BAD_ANSWER);
            return;
        }

        switch (response)
        {
        case Result::SUCCESS:
            finish();
            return;

        case Result::ERR_NO_ANSWER:
            state_ = State::POLL_PENDING;
            break;

        default:
            finish(response);
            return;
        }
        break;

    default:
        return;
    }
}

void Cat24cx::createRequest(I2cBus * bus, std::uint32_t current_time)
{
    if (nullptr == bus)
        return;

    if (State::READ_PENDING == state_)
    {
        auto * const address_transaction = bus->allocate();
        if (nullptr == address_transaction)
            return;
        auto * const read_transaction = bus->allocate();
        if (nullptr == read_transaction)
        {
            bus->release(address_transaction);
            return;
        }

        setWordAddress(address_transaction, word_address_, config_.address_length);
        address_transaction->write(address(), config_.address_length);
        address_transaction->setOpaque(TransactionKind::READ_ADDRESS);
        read_transaction->read(address(), 0, buffer_.data(), pageSize());
        read_transaction->setOpaque( TransactionKind::READ_DATA);
        state_ = State::READ_IN_FLIGHT;

        {
            // Keep the first transaction from completing before the second is queued.
            IrqGuard guard;
            bus->enqueue(address_transaction);
            bus->enqueue(read_transaction);
        }
    }
    else if (State::WRITE_PENDING == state_)
    {
        auto * const transaction = bus->allocate();
        if (nullptr == transaction)
            return;

        setWordAddress(transaction, word_address_, config_.address_length);
        transaction->write(address(), config_.address_length, buffer_.data(), pageSize());
        transaction->setOpaque(TransactionKind::WRITE);
        state_ = State::WRITE_IN_FLIGHT;
        bus->enqueue(transaction);
    }
    else if (State::POLL_PENDING == state_)
    {
        if (poll_timeout_.hasElapsed(current_time, WRITE_READY_TIMEOUT_MS))
        {
            finish(Result::ERR_TIMEOUT);
            return;
        }

        if (!poll_interval_.hasElapsed(current_time, POLL_INTERVAL_MS))
            return;
        poll_interval_.shift(POLL_INTERVAL_MS);

        auto * const transaction = bus->allocate();
        if (nullptr == transaction)
            return;

        // Address-only writes do not modify EEPROM data
        setWordAddress(transaction, word_address_, config_.address_length);
        transaction->write(address(), config_.address_length);
        transaction->setOpaque(TransactionKind::POLL);
        state_ = State::POLL_IN_FLIGHT;
        bus->enqueue(transaction);
    }
}

}  // namespace i2c
}  // namespace driver
