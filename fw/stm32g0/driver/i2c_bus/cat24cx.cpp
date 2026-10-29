#include "driver/i2c_bus/cat24cx.hpp"

#include "driver/tools/irq.hpp"


namespace driver
{
namespace i2c
{

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
    result_ = Result::IDLE;
    return true;
}

bool Cat24cx::startPage(std::uint32_t page, State state)
{
    if (isBusy() || page >= pageCount())
        return false;

    word_address_ = static_cast<std::uint16_t>(page * config_.page_size);
    read_pending_ = 0;
    read_result_ = Result::SUCCESS;
    state_ = state;
    result_ = Result::BUSY;
    return true;
}

bool Cat24cx::readPage(std::uint32_t page)
{
    return startPage(page, State::READ_PENDING);
}

bool Cat24cx::writePage(std::uint32_t page)
{
    return startPage(page, State::WRITE_PENDING);
}

void Cat24cx::setWordAddress(I2cBus::Transaction * transaction) const
{
    auto * const bytes = static_cast<std::uint8_t *>(transaction->localBuffer());
    if (2 == config_.address_length)
    {
        bytes[0] = static_cast<std::uint8_t>(word_address_ >> 8);
        bytes[1] = static_cast<std::uint8_t>(word_address_);
    }
    else
    {
        bytes[0] = static_cast<std::uint8_t>(word_address_);
    }
}

auto Cat24cx::toResult(I2cBus::Transaction::Status status) -> Result
{
    using Status = I2cBus::Transaction::Status;
    switch (status)
    {
    case Status::DONE: return Result::SUCCESS;
    case Status::ERR_NO_ANSWER: return Result::ERR_NO_ANSWER;
    case Status::ERR_ARBITRATION_LOST: return Result::ERR_ARBITRATION_LOST;
    case Status::ERR_BUS_ERROR: return Result::ERR_BUS_ERROR;
    }
    return Result::ERR_BUS_ERROR;
}

void Cat24cx::finish(Result result)
{
    state_ = State::IDLE;
    result_ = result;
}

void Cat24cx::handleResponse(const I2cBus::Transaction * transaction, std::uint32_t current_time)
{
    if (nullptr == transaction || transaction->address() != address())
        return;

    const auto request = transaction->opaque();
    if (State::READ_IN_FLIGHT == state_)
    {
        const auto address_request = static_cast<std::uint8_t>(TransactionKind::READ_ADDRESS);
        const auto data_request = static_cast<std::uint8_t>(TransactionKind::READ_DATA);
        if ((request != address_request && request != data_request) || 0 == (read_pending_ & request))
            return;

        read_pending_ &= static_cast<std::uint8_t>(~request);
        const Result response = toResult(transaction->status());
        if (Result::SUCCESS == read_result_ && Result::SUCCESS != response)
            read_result_ = response;
        if (0 == read_pending_)
            finish(read_result_);
        return;
    }

    if (State::WRITE_IN_FLIGHT == state_ && request == static_cast<std::uint8_t>(TransactionKind::WRITE))
    {
        const Result response = toResult(transaction->status());
        if (Result::SUCCESS != response)
        {
            finish(response);
            return;
        }

        poll_started_at_ = current_time;
        last_poll_at_ = current_time - POLL_INTERVAL_MS;
        state_ = State::POLL_PENDING;
    }
    else if (State::POLL_IN_FLIGHT == state_ && request == static_cast<std::uint8_t>(TransactionKind::POLL))
    {
        const Result response = toResult(transaction->status());
        if (Result::ERR_NO_ANSWER == response)
        {
            if (poll_after_deadline_)
                finish(Result::ERR_TIMEOUT);
            else
                state_ = State::POLL_PENDING;
        }
        else
        {
            finish(response);
        }
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

        const auto address_request = static_cast<std::uint8_t>(TransactionKind::READ_ADDRESS);
        const auto data_request = static_cast<std::uint8_t>(TransactionKind::READ_DATA);
        setWordAddress(address_transaction);
        address_transaction->write(address(), config_.address_length);
        address_transaction->setOpaque(address_request);
        read_transaction->read(address(), 0, buffer_.data(), pageSize());
        read_transaction->setOpaque(data_request);
        read_pending_ = static_cast<std::uint8_t>(address_request | data_request);
        state_ = State::READ_IN_FLIGHT;

        // Keep the first transaction from completing before the second is queued.
        IrqGuard guard;
        bus->enqueue(address_transaction);
        bus->enqueue(read_transaction);
    }
    else if (State::WRITE_PENDING == state_)
    {
        auto * const transaction = bus->allocate();
        if (nullptr == transaction)
            return;

        setWordAddress(transaction);
        transaction->write(address(), config_.address_length, buffer_.data(), pageSize());
        transaction->setOpaque(static_cast<std::uint8_t>(TransactionKind::WRITE));
        state_ = State::WRITE_IN_FLIGHT;
        bus->enqueue(transaction);
    }
    else if (State::POLL_PENDING == state_)
    {
        if (current_time - last_poll_at_ < POLL_INTERVAL_MS)
            return;

        auto * const transaction = bus->allocate();
        if (nullptr == transaction)
            return;

        // Address-only writes do not modify EEPROM data and avoid a zero-byte DMA transfer.
        setWordAddress(transaction);
        transaction->write(address(), config_.address_length);
        transaction->setOpaque(static_cast<std::uint8_t>(TransactionKind::POLL));
        last_poll_at_ = current_time;
        poll_after_deadline_ = current_time - poll_started_at_ >= WRITE_READY_TIMEOUT_MS;
        state_ = State::POLL_IN_FLIGHT;
        bus->enqueue(transaction);
    }
}

}  // namespace i2c
}  // namespace driver
