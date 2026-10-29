#include <catch2/catch_test_macros.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <span>
#include <type_traits>

#include "driver/i2c_bus/cat24cx.hpp"
#include "driver/i2c_bus_fake.hpp"


namespace
{

using Bus = driver::I2cBus;
using Transaction = Bus::Transaction;
using Status = Transaction::Status;
using Cat24cx = driver::i2c::Cat24cx;
using Result = Cat24cx::Result;

static_assert(!std::is_copy_constructible_v<Cat24cx>);
static_assert(!std::is_move_constructible_v<Cat24cx>);

class Fixture
{
public:
    explicit Fixture(std::size_t transaction_count = 4)
    {
        test_i2c::reset();
        REQUIRE(bus.initialize(driver::I2cId::I2C_1, driver::DmaChannelId::DMA_3, transaction_count));
    }

    void complete(Status status, std::uint32_t now)
    {
        test_i2c::completeNext(bus, status);
        auto * const transaction = bus.get();
        REQUIRE(nullptr != transaction);
        eeprom.handleResponse(transaction, now);
        bus.release(transaction);
    }

    Bus bus;
    Cat24cx eeprom;
};

std::span<const std::uint8_t> localBytes(const Transaction * transaction)
{
    const auto [data, size] = transaction->local();
    return {static_cast<const std::uint8_t *>(data), size};
}

}  // namespace


TEST_CASE("CAT24C256 reads a whole page with consecutive transactions")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    std::uint32_t page = 0;
    std::uint8_t high_byte = 0;
    std::uint8_t low_byte = 0;

    SECTION("first page")
    {
    }
    SECTION("last page")
    {
        page = 511;
        high_byte = 0x7F;
        low_byte = 0xC0;
    }

    CHECK(eeprom.pageSize() == 64);
    CHECK(eeprom.pageCount() == 512);
    CHECK(eeprom.address() == Bus::Address(0x50));
    CHECK(eeprom.result() == Result::IDLE);
    auto * const buffer = eeprom.buffer().data();
    REQUIRE(nullptr != buffer);

    REQUIRE(eeprom.readPage(page));
    CHECK(eeprom.isBusy());
    CHECK(eeprom.buffer().empty());
    CHECK_FALSE(eeprom.readPage(page));
    CHECK_FALSE(eeprom.writePage(page));

    eeprom.createRequest(&fixture.bus, 0);
    const auto & enqueued = test_i2c::enqueued();
    REQUIRE(enqueued.size() == 2);
    CHECK(enqueued[0].interrupts_disabled);
    CHECK(enqueued[1].interrupts_disabled);

    const auto * const address_transaction = enqueued[0].transaction;
    const auto * const read_transaction = enqueued[1].transaction;
    CHECK(address_transaction->address() == Bus::Address(0x50));
    CHECK(address_transaction->operation() == Transaction::Operation::WRITE);
    REQUIRE(localBytes(address_transaction).size() == 2);
    CHECK(localBytes(address_transaction)[0] == high_byte);
    CHECK(localBytes(address_transaction)[1] == low_byte);
    CHECK(address_transaction->remote().second == 0);
    CHECK(read_transaction->operation() == Transaction::Operation::READ);
    CHECK(read_transaction->local().second == 0);
    CHECK(read_transaction->remote().first == buffer);
    CHECK(read_transaction->remote().second == 64);

    fixture.complete(Status::DONE, 0);
    CHECK(eeprom.isBusy());
    std::memset(const_cast<void *>(read_transaction->remote().first), 0xA5, 64);
    fixture.complete(Status::DONE, 0);
    CHECK(eeprom.result() == Result::SUCCESS);
    CHECK(eeprom.buffer().data() == buffer);
    for (const auto byte : eeprom.buffer())
        CHECK(byte == 0xA5);

    CHECK_FALSE(eeprom.readPage(512));
    CHECK_FALSE(eeprom.writePage(512));
    CHECK(eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx supports smaller one-byte addressed pages")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    Cat24cx::Config config;
    config.capacity = 256;
    config.page_size = 8;
    config.address_length = 1;
    config.device_address = 0x54;
    auto * const buffer = eeprom.buffer().data();

    REQUIRE(eeprom.configure(config));
    CHECK(eeprom.pageSize() == 8);
    CHECK(eeprom.pageCount() == 32);
    CHECK(eeprom.buffer().size() == 8);
    CHECK(eeprom.buffer().data() == buffer);
    CHECK(eeprom.address() == Bus::Address(0x54));
    CHECK_FALSE(eeprom.readPage(32));

    REQUIRE(eeprom.readPage(31));
    eeprom.createRequest(&fixture.bus, 5);
    const auto & enqueued = test_i2c::enqueued();
    REQUIRE(enqueued.size() == 2);
    REQUIRE(localBytes(enqueued[0].transaction).size() == 1);
    CHECK(localBytes(enqueued[0].transaction)[0] == 0xF8);
    CHECK(enqueued[0].transaction->address() == Bus::Address(0x54));
    CHECK(enqueued[1].transaction->remote().first == buffer);
    CHECK(enqueued[1].transaction->remote().second == 8);
    fixture.complete(Status::DONE, 5);
    fixture.complete(Status::DONE, 5);
    CHECK(eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx validates geometry without changing the existing configuration")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    Cat24cx::Config valid;
    valid.capacity = 256;
    valid.page_size = 8;
    valid.address_length = 1;
    valid.device_address = 0x54;
    REQUIRE(eeprom.configure(valid));
    auto invalid = valid;

    SECTION("zero capacity") { invalid.capacity = 0; }
    SECTION("capacity exceeds the address length") { invalid.capacity = 264; }
    SECTION("capacity is not a whole number of pages") { invalid.capacity = 252; }
    SECTION("zero page size") { invalid.page_size = 0; }
    SECTION("page exceeds the fixed buffer") { invalid.page_size = 65; }
    SECTION("zero address bytes") { invalid.address_length = 0; }
    SECTION("too many address bytes") { invalid.address_length = 3; }
    SECTION("device address is too low") { invalid.device_address = 0x4F; }
    SECTION("device address is too high") { invalid.device_address = 0x58; }

    CHECK_FALSE(eeprom.configure(invalid));
    CHECK(eeprom.pageSize() == 8);
    CHECK(eeprom.pageCount() == 32);
    CHECK(eeprom.address() == Bus::Address(0x54));
    CHECK(eeprom.buffer().size() == 8);
}

TEST_CASE("CAT24Cx accepts the full two-byte word address range")
{
    Fixture fixture;
    Cat24cx::Config config;
    config.capacity = 65536;
    config.page_size = 1;
    REQUIRE(fixture.eeprom.configure(config));
    CHECK(fixture.eeprom.pageCount() == 65536);
    REQUIRE(fixture.eeprom.readPage(65535));
    fixture.eeprom.createRequest(&fixture.bus, 0);
    const auto & enqueued = test_i2c::enqueued();
    REQUIRE(enqueued.size() == 2);
    REQUIRE(localBytes(enqueued[0].transaction).size() == 2);
    CHECK(localBytes(enqueued[0].transaction)[0] == 0xFF);
    CHECK(localBytes(enqueued[0].transaction)[1] == 0xFF);
    fixture.complete(Status::DONE, 0);
    fixture.complete(Status::DONE, 0);
    CHECK(fixture.eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx retries a read when the transaction pool is exhausted")
{
    Fixture fixture(2);
    auto * const reserved = fixture.bus.allocate();
    REQUIRE(nullptr != reserved);
    REQUIRE(fixture.eeprom.readPage(1));

    fixture.eeprom.createRequest(&fixture.bus, 0);
    CHECK(test_i2c::enqueued().empty());
    CHECK(fixture.eeprom.isBusy());
    auto * const returned = fixture.bus.allocate();
    REQUIRE(nullptr != returned);
    fixture.bus.release(returned);
    fixture.bus.release(reserved);

    fixture.eeprom.createRequest(&fixture.bus, 1);
    REQUIRE(test_i2c::enqueued().size() == 2);
    fixture.complete(Status::DONE, 1);
    fixture.complete(Status::DONE, 1);
    CHECK(fixture.eeprom.result() == Result::SUCCESS);

    REQUIRE(fixture.eeprom.readPage(2));
    fixture.eeprom.createRequest(&fixture.bus, 2);
    REQUIRE(test_i2c::enqueued().size() == 4);
    fixture.complete(Status::DONE, 2);
    fixture.complete(Status::DONE, 2);
    CHECK(fixture.eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx waits for both read responses and preserves the first error")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    REQUIRE(eeprom.readPage(0));
    eeprom.createRequest(&fixture.bus, 0);
    REQUIRE(test_i2c::enqueued().size() == 2);

    Transaction unrelated;
    eeprom.handleResponse(&unrelated, 0);
    CHECK(eeprom.isBusy());

    fixture.complete(Status::ERR_NO_ANSWER, 0);
    CHECK(eeprom.isBusy());
    CHECK(eeprom.buffer().empty());
    fixture.complete(Status::ERR_BUS_ERROR, 0);
    CHECK(eeprom.result() == Result::ERR_NO_ANSWER);
    CHECK_FALSE(eeprom.isBusy());
    CHECK(eeprom.buffer().size() == 64);
}

TEST_CASE("CAT24Cx identifies read responses by their opaque tags")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    REQUIRE(eeprom.readPage(0));
    eeprom.createRequest(&fixture.bus, 0);
    REQUIRE(test_i2c::enqueued().size() == 2);

    const auto address_tag = test_i2c::enqueued()[0].transaction->opaque();
    const auto data_tag = test_i2c::enqueued()[1].transaction->opaque();
    CHECK(address_tag != 0);
    CHECK(data_tag != 0);
    CHECK(address_tag != data_tag);

    test_i2c::completeNext(fixture.bus, Status::ERR_NO_ANSWER);
    auto * const address_response = fixture.bus.get();
    REQUIRE(nullptr != address_response);
    test_i2c::completeNext(fixture.bus, Status::ERR_BUS_ERROR);
    auto * const data_response = fixture.bus.get();
    REQUIRE(nullptr != data_response);

    // The local buffers are no longer in use; initialize them before copying.
    std::memset(address_response->localBuffer(), 0, Transaction::STORAGE_SIZE);
    std::memset(data_response->localBuffer(), 0, Transaction::STORAGE_SIZE);
    const Transaction address_copy = *address_response;
    const Transaction data_copy = *data_response;
    Transaction unexpected = address_copy;
    unexpected.setOpaque(0xFF);
    Transaction foreign = address_copy;
    foreign.setAddress(Bus::Address(0x51));

    eeprom.handleResponse(nullptr, 0);
    eeprom.handleResponse(&unexpected, 0);
    eeprom.handleResponse(&foreign, 0);
    CHECK(eeprom.isBusy());

    eeprom.handleResponse(&data_copy, 0);
    CHECK(eeprom.isBusy());
    eeprom.handleResponse(&data_copy, 0);
    CHECK(eeprom.isBusy());
    eeprom.handleResponse(&address_copy, 0);
    CHECK(eeprom.result() == Result::ERR_BUS_ERROR);

    fixture.bus.release(address_response);
    fixture.bus.release(data_response);
}

TEST_CASE("CAT24Cx reports a failed read even when the address phase succeeded")
{
    Fixture fixture;
    REQUIRE(fixture.eeprom.readPage(0));
    fixture.eeprom.createRequest(&fixture.bus, 0);
    fixture.complete(Status::DONE, 0);
    fixture.complete(Status::ERR_ARBITRATION_LOST, 0);
    CHECK(fixture.eeprom.result() == Result::ERR_ARBITRATION_LOST);
}

TEST_CASE("CAT24Cx keeps the address error when the read phase succeeds")
{
    Fixture fixture;
    REQUIRE(fixture.eeprom.readPage(0));
    fixture.eeprom.createRequest(&fixture.bus, 0);
    fixture.complete(Status::ERR_NO_ANSWER, 0);
    CHECK(fixture.eeprom.isBusy());
    fixture.complete(Status::DONE, 0);
    CHECK(fixture.eeprom.result() == Result::ERR_NO_ANSWER);
}

TEST_CASE("CAT24Cx retries writes and polls when the transaction pool is exhausted")
{
    Fixture fixture(1);
    auto & eeprom = fixture.eeprom;
    auto * const buffer = eeprom.buffer().data();
    auto * reserved = fixture.bus.allocate();
    REQUIRE(nullptr != reserved);
    REQUIRE(eeprom.writePage(511));

    eeprom.createRequest(&fixture.bus, 0);
    CHECK(test_i2c::enqueued().empty());
    CHECK(eeprom.isBusy());
    fixture.bus.release(reserved);

    eeprom.createRequest(&fixture.bus, 1);
    REQUIRE(test_i2c::enqueued().size() == 1);
    CHECK(test_i2c::enqueued().back().transaction->remote().first == buffer);
    fixture.complete(Status::DONE, 1);

    reserved = fixture.bus.allocate();
    REQUIRE(nullptr != reserved);
    eeprom.createRequest(&fixture.bus, 1);
    CHECK(test_i2c::enqueued().size() == 1);
    CHECK(eeprom.isBusy());
    fixture.bus.release(reserved);

    eeprom.createRequest(&fixture.bus, 2);
    REQUIRE(test_i2c::enqueued().size() == 2);
    fixture.complete(Status::DONE, 2);
    CHECK(eeprom.result() == Result::SUCCESS);
    CHECK(eeprom.buffer().data() == buffer);
}

TEST_CASE("CAT24Cx uses its fixed page buffer for writes and polls until ready")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    auto page = eeprom.buffer();
    auto * const buffer = page.data();
    for (std::size_t i = 0; i < page.size(); ++i)
        page[i] = static_cast<std::uint8_t>(i);

    REQUIRE(eeprom.writePage(1));
    CHECK(eeprom.buffer().empty());
    CHECK(static_cast<const Cat24cx &>(eeprom).buffer().empty());
    Cat24cx::Config config;
    CHECK_FALSE(eeprom.configure(config));
    eeprom.createRequest(&fixture.bus, 100);
    REQUIRE(test_i2c::enqueued().size() == 1);
    const auto * const write = test_i2c::enqueued().back().transaction;
    CHECK(write->operation() == Transaction::Operation::WRITE);
    REQUIRE(localBytes(write).size() == 2);
    CHECK(localBytes(write)[0] == 0);
    CHECK(localBytes(write)[1] == 64);
    CHECK(write->remote().first == buffer);
    CHECK(write->remote().second == 64);

    fixture.complete(Status::DONE, 100);
    CHECK(eeprom.isBusy());
    eeprom.createRequest(&fixture.bus, 100);
    REQUIRE(test_i2c::enqueued().size() == 2);
    const auto * const poll = test_i2c::enqueued().back().transaction;
    CHECK(poll->operation() == Transaction::Operation::WRITE);
    REQUIRE(localBytes(poll).size() == 2);
    CHECK(localBytes(poll)[0] == 0);
    CHECK(localBytes(poll)[1] == 64);
    CHECK(poll->remote().second == 0);

    fixture.complete(Status::ERR_NO_ANSWER, 100);
    eeprom.createRequest(&fixture.bus, 100);
    CHECK(test_i2c::enqueued().size() == 2);
    eeprom.createRequest(&fixture.bus, 101);
    REQUIRE(test_i2c::enqueued().size() == 3);
    fixture.complete(Status::DONE, 101);
    CHECK(eeprom.result() == Result::SUCCESS);
    REQUIRE(eeprom.buffer().data() == buffer);
    for (std::size_t i = 0; i < eeprom.buffer().size(); ++i)
        CHECK(eeprom.buffer()[i] == static_cast<std::uint8_t>(i));
}

TEST_CASE("CAT24Cx distinguishes write and poll responses by opaque tag")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    REQUIRE(eeprom.writePage(0));
    eeprom.createRequest(&fixture.bus, 10);
    REQUIRE(test_i2c::enqueued().size() == 1);
    const auto write_tag = test_i2c::enqueued().back().transaction->opaque();
    CHECK(write_tag != 0);

    test_i2c::completeNext(fixture.bus, Status::DONE);
    auto * const write_response = fixture.bus.get();
    REQUIRE(nullptr != write_response);
    std::memset(write_response->localBuffer(), 0, Transaction::STORAGE_SIZE);
    const Transaction write_copy = *write_response;
    Transaction unexpected = write_copy;
    unexpected.setOpaque(0xFF);
    eeprom.handleResponse(&unexpected, 10);
    eeprom.createRequest(&fixture.bus, 10);
    REQUIRE(test_i2c::enqueued().size() == 1);

    eeprom.handleResponse(&write_copy, 10);
    fixture.bus.release(write_response);
    eeprom.createRequest(&fixture.bus, 10);
    REQUIRE(test_i2c::enqueued().size() == 2);
    const auto poll_tag = test_i2c::enqueued().back().transaction->opaque();
    CHECK(poll_tag != 0);
    CHECK(poll_tag != write_tag);

    eeprom.handleResponse(&write_copy, 10);
    CHECK(eeprom.isBusy());
    test_i2c::completeNext(fixture.bus, Status::DONE);
    auto * const poll_response = fixture.bus.get();
    REQUIRE(nullptr != poll_response);
    std::memset(poll_response->localBuffer(), 0, Transaction::STORAGE_SIZE);
    const Transaction poll_copy = *poll_response;
    eeprom.handleResponse(&poll_copy, 10);
    CHECK(eeprom.result() == Result::SUCCESS);
    fixture.bus.release(poll_response);
}

TEST_CASE("CAT24Cx polls the configured one-byte address after writes")
{
    Fixture fixture;
    Cat24cx::Config config;
    config.capacity = 256;
    config.page_size = 8;
    config.address_length = 1;
    REQUIRE(fixture.eeprom.configure(config));
    REQUIRE(fixture.eeprom.writePage(31));
    fixture.eeprom.createRequest(&fixture.bus, 0);
    REQUIRE(localBytes(test_i2c::enqueued().back().transaction).size() == 1);
    CHECK(localBytes(test_i2c::enqueued().back().transaction)[0] == 0xF8);
    fixture.complete(Status::DONE, 0);
    fixture.eeprom.createRequest(&fixture.bus, 0);
    REQUIRE(localBytes(test_i2c::enqueued().back().transaction).size() == 1);
    CHECK(localBytes(test_i2c::enqueued().back().transaction)[0] == 0xF8);
    fixture.complete(Status::DONE, 0);
    CHECK(fixture.eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx times out repeated NACKs across clock wraparound")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    const std::uint32_t start = std::numeric_limits<std::uint32_t>::max() - 4;
    REQUIRE(eeprom.writePage(0));
    eeprom.createRequest(&fixture.bus, start);
    fixture.complete(Status::DONE, start);

    for (std::uint32_t elapsed = 0; elapsed <= Cat24cx::WRITE_READY_TIMEOUT_MS; ++elapsed)
    {
        const std::uint32_t now = start + elapsed;
        eeprom.createRequest(&fixture.bus, now);
        REQUIRE(test_i2c::enqueued().size() == 2 + elapsed);
        fixture.complete(Status::ERR_NO_ANSWER, now);
        if (elapsed < Cat24cx::WRITE_READY_TIMEOUT_MS)
            CHECK(eeprom.isBusy());
    }
    CHECK(eeprom.result() == Result::ERR_TIMEOUT);
    CHECK_FALSE(eeprom.isBusy());
}

TEST_CASE("CAT24Cx waits for an in-flight poll and accepts ACK after the deadline")
{
    Fixture fixture;
    auto & eeprom = fixture.eeprom;
    REQUIRE(eeprom.writePage(0));
    eeprom.createRequest(&fixture.bus, 0);
    fixture.complete(Status::DONE, 0);
    eeprom.createRequest(&fixture.bus, 0);
    fixture.complete(Status::ERR_NO_ANSWER, 0);

    eeprom.createRequest(&fixture.bus, 10);
    REQUIRE(test_i2c::enqueued().size() == 3);
    eeprom.createRequest(&fixture.bus, 100);
    CHECK(test_i2c::enqueued().size() == 3);
    CHECK(eeprom.isBusy());
    fixture.complete(Status::DONE, 100);
    CHECK(eeprom.result() == Result::SUCCESS);
}

TEST_CASE("CAT24Cx propagates write and polling bus errors")
{
    Fixture fixture;
    Status status = Status::ERR_NO_ANSWER;
    Result expected = Result::ERR_NO_ANSWER;
    bool during_poll = false;

    SECTION("write NACK") { }
    SECTION("write arbitration loss")
    {
        status = Status::ERR_ARBITRATION_LOST;
        expected = Result::ERR_ARBITRATION_LOST;
    }
    SECTION("write bus error")
    {
        status = Status::ERR_BUS_ERROR;
        expected = Result::ERR_BUS_ERROR;
    }
    SECTION("poll arbitration loss")
    {
        status = Status::ERR_ARBITRATION_LOST;
        expected = Result::ERR_ARBITRATION_LOST;
        during_poll = true;
    }
    SECTION("poll bus error")
    {
        status = Status::ERR_BUS_ERROR;
        expected = Result::ERR_BUS_ERROR;
        during_poll = true;
    }

    REQUIRE(fixture.eeprom.writePage(0));
    fixture.eeprom.createRequest(&fixture.bus, 0);
    if (during_poll)
    {
        fixture.complete(Status::DONE, 0);
        fixture.eeprom.createRequest(&fixture.bus, 0);
    }
    fixture.complete(status, 0);
    CHECK(fixture.eeprom.result() == expected);
    CHECK_FALSE(fixture.eeprom.isBusy());
}