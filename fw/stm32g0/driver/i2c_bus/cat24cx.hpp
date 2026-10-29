/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_CAT24CX_HPP_
#define DRIVER_I2C_BUS_CAT24CX_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "driver/i2c_bus.hpp"


namespace driver
{
namespace i2c
{

/**
 * @brief Asynchronous, page-oriented CAT24C EEPROM driver
 *
 * The default configuration is for a CAT24C256. The driver owns one page
 * buffer and does not allocate memory. Call createRequest() and pass completed
 * transactions to handleResponse() from the same background task. The driver
 * must outlive all transactions that reference its buffer.
 */
class Cat24cx
{
public:
    static const inline std::size_t MAX_PAGE_SIZE = 64;
    static const inline std::uint32_t WRITE_READY_TIMEOUT_MS = 10;

    struct Config
    {
        std::uint32_t capacity = 32768;
        std::uint8_t page_size = 64;
        std::uint8_t address_length = 2;
        std::uint8_t device_address = 0x50;
    };

    enum class Result: std::uint8_t
    {
        IDLE,
        BUSY,
        SUCCESS,
        ERR_NO_ANSWER,
        ERR_ARBITRATION_LOST,
        ERR_BUS_ERROR,
        ERR_TIMEOUT,
    };

    Cat24cx() = default;
    Cat24cx(const Cat24cx &) = delete;
    Cat24cx & operator =(const Cat24cx &) = delete;
    Cat24cx(Cat24cx &&) = delete;
    Cat24cx & operator =(Cat24cx &&) = delete;

    /**
     * @brief Configure the EEPROM while no operation is active
     *
     * Only one- and two-byte internal addresses are supported; devices which
     * encode additional memory address bits in the I2C address are not.
     * Invalid configurations leave the current configuration unchanged.
     */
    [[nodiscard]]
    bool configure(const Config & config);

    I2cBus::Address address() const { return I2cBus::Address(config_.device_address); }
    std::size_t pageSize() const { return config_.page_size; }
    std::uint32_t pageCount() const { return config_.capacity / config_.page_size; }
    Result result() const { return result_; }
    bool isBusy() const { return Result::BUSY == result_; }

    /**
     * @brief Get direct access to the driver-owned page buffer
     *
     * Returns an empty span while busy. A previously obtained span must not be
     * accessed while an operation is busy, and must not outlive the driver.
     * After a failed read the buffer contents are unspecified.
     */
    std::span<std::uint8_t> buffer()
    {
        if (isBusy())
            return {};
        return {buffer_.data(), pageSize()};
    }

    std::span<const std::uint8_t> buffer() const
    {
        if (isBusy())
            return {};
        return {buffer_.data(), pageSize()};
    }

    /**
     * @brief Schedule a whole-page read into the page buffer
     * @return False if the page is out of range or an operation is busy
     */
    [[nodiscard]]
    bool readPage(std::uint32_t page);

    /**
     * @brief Schedule a whole-page write from the page buffer
     * @return False if the page is out of range or an operation is busy
     */
    [[nodiscard]]
    bool writePage(std::uint32_t page);

    /**
     * @brief Handle a completed transaction before the bus releases it
     *
     * Transactions for this device must be owned by this driver, and their
     * opaque values must remain unchanged until their responses are handled.
     * @param current_time Current time in milliseconds
     */
    void handleResponse(const I2cBus::Transaction * transaction, std::uint32_t current_time);

    /**
     * @brief Queue the next transaction, if one is needed and available
     * @param current_time Current time in milliseconds
     */
    void createRequest(I2cBus * bus, std::uint32_t current_time);

private:
    enum class State: std::uint8_t
    {
        IDLE,
        READ_PENDING,
        READ_IN_FLIGHT,
        WRITE_PENDING,
        WRITE_IN_FLIGHT,
        POLL_PENDING,
        POLL_IN_FLIGHT,
    };

    enum class TransactionKind: std::uint8_t
    {
        READ_ADDRESS = 0x01,
        READ_DATA = 0x02,
        WRITE = 0x04,
        POLL = 0x08,
    };

    static const inline std::uint32_t POLL_INTERVAL_MS = 1;

    bool startPage(std::uint32_t page, State state);
    void setWordAddress(I2cBus::Transaction * transaction) const;
    void finish(Result result);
    static Result toResult(I2cBus::Transaction::Status status);

    Config config_;
    std::array<std::uint8_t, MAX_PAGE_SIZE> buffer_{};
    std::uint32_t poll_started_at_ = 0;
    std::uint32_t last_poll_at_ = 0;
    std::uint16_t word_address_ = 0;
    std::uint8_t read_pending_ = 0;
    State state_ = State::IDLE;
    Result result_ = Result::IDLE;
    Result read_result_ = Result::SUCCESS;
    bool poll_after_deadline_ = false;
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_CAT24CX_HPP_
