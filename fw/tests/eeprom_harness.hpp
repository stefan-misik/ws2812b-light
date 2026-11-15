#ifndef TEST_EEPROM_HARNESS_HPP_
#define TEST_EEPROM_HARNESS_HPP_

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <initializer_list>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "app/records.hpp"
#include "driver/i2c_bus/cat24cx.hpp"

namespace test
{
using Byte = std::uint8_t;
using Bytes = std::vector<Byte>;
using Driver = driver::i2c::Cat24cx;
using Bus = driver::I2cBus;
using Status = Bus::Transaction::Status;

// Deliberately independent, bit-at-a-time CCITT-FALSE reference codec.
inline std::uint16_t referenceCrc(const Bytes & data)
{
    std::uint16_t crc = 0xffffu;
    for (Byte byte : data)
    {
        crc ^= static_cast<std::uint16_t>(byte) << 8;
        for (unsigned bit = 0; bit < 8; ++bit)
            crc = static_cast<std::uint16_t>((crc << 1) ^ ((crc & 0x8000u) ? 0x1021u : 0u));
    }
    return crc;
}

inline Bytes page(Byte id, std::uint16_t epoch, const Bytes & payload, std::size_t page_size = 64)
{
    REQUIRE(payload.size() + 6 <= page_size);
    Bytes meaningful{id, static_cast<Byte>(payload.size()), static_cast<Byte>(epoch >> 8),
                     static_cast<Byte>(epoch), 0xff, 0xff};
    meaningful.insert(meaningful.end(), payload.begin(), payload.end());
    const auto crc = referenceCrc(meaningful);
    meaningful[4] = static_cast<Byte>(crc >> 8);
    meaningful[5] = static_cast<Byte>(crc);
    meaningful.resize(page_size, 0xff);
    return meaningful;
}

inline void requirePage(const Bytes & actual, Byte id, std::uint16_t epoch, const Bytes & payload)
{
    const auto expected = page(id, epoch, payload, actual.size());
    REQUIRE(std::equal(expected.begin(), expected.begin() + 6 + payload.size(), actual.begin()));
}

inline Bytes collisionPage(Byte id, std::uint16_t epoch, const Bytes & payload)
{
    REQUIRE(payload.size() == 3);
    const auto original = page(id, epoch, payload);
    const auto target = static_cast<std::uint16_t>((original[4] << 8) | original[5]);
    Bytes candidate{id, 3, static_cast<Byte>(epoch >> 8), static_cast<Byte>(epoch),
                    0xff, 0xff, static_cast<Byte>(payload[0] ^ 1u), 0, 0};
    for (unsigned suffix = 0; suffix <= 0xffffu; ++suffix)
    {
        candidate[7] = static_cast<Byte>(suffix >> 8);
        candidate[8] = static_cast<Byte>(suffix);
        if (referenceCrc(candidate) == target)
            return page(id, epoch, {candidate[6], candidate[7], candidate[8]});
    }
    FAIL("Could not construct a CRC collision");
    return {};
}

enum class Kind { READ_ADDRESS, READ_DATA, WRITE, POLL };

struct Fault
{
    Kind kind;
    Status status = Status::DONE;
    bool commit_on_error = false;
    // For READ_DATA with DONE, change only the returned buffer, not the media.
    std::size_t corrupt_offset = static_cast<std::size_t>(-1);
};

// Calls createRequest and handleResponse on the production Cat24cx. Unlike an
// immediate mock, each queued transaction completes independently and later;
// the second read can still transfer data after the first transaction fails.
class Eeprom
{
public:
    explicit Eeprom(std::size_t page_count = 8, std::size_t page_size = 64, Byte address_length = 2):
        page_size_(page_size), media_(page_count * page_size, 0xff)
    {
        Driver::Config config;
        config.capacity = static_cast<std::uint32_t>(media_.size());
        config.page_size = static_cast<Byte>(page_size);
        config.address_length = address_length;
        REQUIRE(driver.configure(config));
    }

    void submit()
    {
        ++now;
        driver.createRequest(&bus, now);
    }

    void tick()
    {
        submit();
        if (bus.pendingCount() != 0)
            completeOne();
    }

    void completeOne()
    {
        REQUIRE(bus.pendingCount() != 0);
        const bool address_read = bus.nextIsRead();
        auto * transaction = bus.pop();
        const auto [local, local_size] = transaction->local();
        const auto [remote, remote_size] = transaction->remote();
        const auto * local_bytes = static_cast<const Byte *>(local);
        Kind kind;
        if (transaction->operation() == Bus::Transaction::Operation::READ)
            kind = Kind::READ_DATA;
        else if (remote_size != 0)
            kind = Kind::WRITE;
        else
            kind = address_read ? Kind::READ_ADDRESS : Kind::POLL;

        Fault fault{kind};
        if (!faults.empty() && faults.front().kind == kind)
        {
            fault = faults.front();
            faults.pop_front();
        }
        else if (kind == Kind::POLL && poll_nacks != 0)
        {
            --poll_nacks;
            fault.status = Status::ERR_NO_ANSWER;
        }
        if (kind == Kind::READ_ADDRESS || kind == Kind::WRITE)
        {
            REQUIRE((local_size == 1 || local_size == 2));
            address_ = local_bytes[0];
            if (local_size == 2)
                address_ = (address_ << 8) | local_bytes[1];
        }
        if (kind == Kind::READ_DATA && fault.status == Status::DONE)
        {
            REQUIRE(address_ + remote_size <= media_.size());
            auto * destination = static_cast<Byte *>(transaction->mutableRemote());
            std::copy_n(media_.begin() + address_, remote_size, destination);
            if (!read_override.empty())
            {
                REQUIRE(read_override.size() == remote_size);
                std::copy(read_override.begin(), read_override.end(), destination);
                read_override.clear();
            }
            if (fault.corrupt_offset < remote_size)
                destination[fault.corrupt_offset] ^= 0x01u;
        }
        if (kind == Kind::WRITE && !write_protected &&
                (fault.status == Status::DONE || fault.commit_on_error))
        {
            REQUIRE(address_ + remote_size <= media_.size());
            const auto * source = static_cast<const Byte *>(remote);
            std::copy_n(source, remote_size, media_.begin() + address_);
        }
        events.push_back(kind);
        transaction->setStatus(fault.status);
        driver.handleResponse(transaction, now);
        bus.release(transaction);
    }

    void setPage(std::size_t index, const Bytes & bytes)
    {
        REQUIRE(bytes.size() == page_size_);
        REQUIRE((index + 1) * page_size_ <= media_.size());
        std::copy(bytes.begin(), bytes.end(), media_.begin() + index * page_size_);
    }
    Bytes getPage(std::size_t index) const
    {
        REQUIRE((index + 1) * page_size_ <= media_.size());
        return {media_.begin() + index * page_size_, media_.begin() + (index + 1) * page_size_};
    }
    const Bytes & media() const { return media_; }
    std::size_t count(Kind kind) const { return std::count(events.begin(), events.end(), kind); }

    Driver driver;
    Bus bus;
    std::uint32_t now = 0;
    std::deque<Fault> faults;
    std::vector<Kind> events;
    Bytes read_override;
    unsigned poll_nacks = 0;
    bool write_protected = false;

private:
    std::size_t page_size_;
    Bytes media_;
    std::size_t address_ = 0;
};

struct Harness
{
    explicit Harness(std::size_t pages = 8, std::size_t page_size = 64): eeprom(pages, page_size) { }

    Records::Result run()
    {
        for (std::size_t i = 0; i < 100000; ++i)
        {
            const auto result = records.run(&eeprom.driver, eeprom.now);
            if (result != Records::PENDING)
                return result;
            eeprom.tick();
        }
        FAIL("Records did not complete within the host harness step limit");
        return Records::PENDING;
    }

    Records::Result read(Byte id)
    {
        REQUIRE(records.beginRead(id));
        return run();
    }

    Records::Result write(Byte id, const Bytes & bytes)
    {
        const auto [buffer, capacity] = records.getRecordBuffer(&eeprom.driver);
        REQUIRE(buffer != nullptr);
        REQUIRE(bytes.size() <= capacity);
        std::copy(bytes.begin(), bytes.end(), static_cast<Byte *>(buffer));
        REQUIRE(records.beginWrite(id, bytes.size()));
        return run();
    }

    Bytes data()
    {
        const auto [buffer, length] = records.getRecordData(&eeprom.driver);
        REQUIRE(buffer != nullptr);
        const auto * first = static_cast<const Byte *>(buffer);
        return {first, first + length};
    }

    Eeprom eeprom;
    Records records;
};
}  // namespace test

#endif