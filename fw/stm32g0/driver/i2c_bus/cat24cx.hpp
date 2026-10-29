/**
 * @file
 */

#ifndef DRIVER_I2C_BUS_CAT24CX_HPP_
#define DRIVER_I2C_BUS_CAT24CX_HPP_

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "tools/timer.hpp"
#include "driver/i2c_bus.hpp"


namespace driver
{
namespace i2c
{

/**
 * @brief Asynchronous, page-oriented CAT24C EEPROM driver
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
        SUCCESS,

        FIRST_ERROR_,
        ERR_NO_ANSWER = FIRST_ERROR_,
        ERR_ARBITRATION_LOST,
        ERR_BUS_ERROR,
        ERR_TIMEOUT,
        ERR_BAD_ANSWER,
    };

    Cat24cx() = default;
    Cat24cx(const Cat24cx &) = delete;
    Cat24cx & operator =(const Cat24cx &) = delete;
    Cat24cx(Cat24cx &&) = delete;
    Cat24cx & operator =(Cat24cx &&) = delete;

    /**
     * @brief Configure the EEPROM while no operation is active
     */
    bool configure(const Config & config);

    I2cBus::Address address() const { return I2cBus::Address(config_.device_address); }
    std::size_t pageSize() const { return config_.page_size; }
    std::uint32_t pageCount() const { return config_.capacity / config_.page_size; }
    Result result() const { return result_; }
    bool isBusy() const { return State::IDLE != state_; }

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
     *
     * @return False if the page is out of range or an operation is busy
     */
    [[nodiscard]]
    bool readPage(std::uint32_t page)
    {
        return startPage(page, State::READ_PENDING);
    }

    /**
     * @brief Schedule a whole-page write from the page buffer
     *
     * @return False if the page is out of range or an operation is busy
     */
    [[nodiscard]]
    bool writePage(std::uint32_t page)
    {
        return startPage(page, State::WRITE_PENDING);
    }

    /**
     * @brief Handle a completed transaction before the bus releases it
     *
     * Transactions for this device must be owned by this driver, and their
     * opaque values must remain unchanged until their responses are handled.
     *
     * @param current_time Current time in milliseconds
     */
    void handleResponse(const I2cBus::Transaction * transaction, std::uint32_t current_time);

    /**
     * @brief Queue the next transaction, if one is needed and available
     *
     * @param current_time Current time in milliseconds
     */
    void createRequest(I2cBus * bus, std::uint32_t current_time);

private:
    enum class State: std::uint8_t
    {
        IDLE,

        READ_PENDING,
        READ_IN_FLIGHT,
        READ_DATA_IN_FLIGHT,

        WRITE_PENDING,
        WRITE_IN_FLIGHT,
        POLL_PENDING,
        POLL_IN_FLIGHT,
    };

    enum TransactionKind: std::uint8_t
    {
        UNINITIALIZED = 0u,
        READ_ADDRESS = 1u,
        READ_DATA,
        WRITE,
        POLL,
    };

    static const inline std::uint32_t POLL_INTERVAL_MS = 1;

    bool startPage(std::uint32_t page, State state);

    void finish(Result result = Result::SUCCESS)
    {
        state_ = State::IDLE;
        result_ = result;
    }

    alignas(void *)
    std::array<std::uint8_t, MAX_PAGE_SIZE> buffer_{};

    Config config_;

    Timer poll_timeout_;
    Timer poll_interval_;
    std::uint16_t word_address_ = 0;
    State state_ = State::IDLE;
    Result result_ = Result::SUCCESS;
};

}  // namespace i2c
}  // namespace driver

#endif  // DRIVER_I2C_BUS_CAT24CX_HPP_
