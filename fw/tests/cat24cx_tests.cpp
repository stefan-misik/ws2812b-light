#include "eeprom_harness.hpp"

using namespace test;

TEST_CASE("Cat24cx reads complete address and data separately", "[cat24cx]")
{
    Eeprom eeprom(4);
    const Bytes expected(64, 0x5a);
    eeprom.setPage(2, expected);
    REQUIRE(eeprom.driver.readPage(2));
    REQUIRE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.buffer().empty());
    eeprom.submit();
    REQUIRE(eeprom.bus.pendingCount() == 2);
    eeprom.completeOne();
    REQUIRE(eeprom.driver.isBusy());
    REQUIRE(eeprom.bus.pendingCount() == 1);
    eeprom.completeOne();
    REQUIRE_FALSE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.result() == Driver::Result::SUCCESS);
    const auto buffer = eeprom.driver.buffer();
    REQUIRE(Bytes(buffer.begin(), buffer.end()) == expected);
}

TEST_CASE("Cat24cx uses the configured EEPROM word address width", "[cat24cx]")
{
    SECTION("two-byte address crosses the 256-byte boundary")
    {
        Eeprom eeprom(8, 64, 2);
        eeprom.setPage(5, Bytes(64, 0x37));
        REQUIRE(eeprom.driver.readPage(5));
        eeprom.tick();
        eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        const auto buffer = eeprom.driver.buffer();
        REQUIRE(Bytes(buffer.begin(), buffer.end()) == Bytes(64, 0x37));
    }
    SECTION("one-byte word addresses work on small devices")
    {
        Eeprom eeprom(8, 16, 1);
        eeprom.setPage(6, Bytes(16, 0x64));
        REQUIRE(eeprom.driver.readPage(6));
        eeprom.tick();
        eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        const auto buffer = eeprom.driver.buffer();
        REQUIRE(Bytes(buffer.begin(), buffer.end()) == Bytes(16, 0x64));
    }
}

TEST_CASE("Cat24cx drains queued data after an address error", "[cat24cx][error]")
{
    Eeprom eeprom(4);
    eeprom.setPage(1, Bytes(64, 0xa5));
    eeprom.faults.push_back({Kind::READ_ADDRESS, Status::ERR_ARBITRATION_LOST});
    bool data_succeeds = true;
    SECTION("data later succeeds and may modify the buffer") { }
    SECTION("data later also fails")
    {
        data_succeeds = false;
        eeprom.faults.push_back({Kind::READ_DATA, Status::ERR_BUS_ERROR});
    }
    REQUIRE(eeprom.driver.readPage(1));
    eeprom.submit();
    REQUIRE(eeprom.bus.pendingCount() == 2);
    eeprom.completeOne();
    REQUIRE(eeprom.bus.pendingCount() == 1);
    REQUIRE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.buffer().empty());
    REQUIRE_FALSE(eeprom.driver.readPage(0));
    REQUIRE_FALSE(eeprom.driver.writePage(0));
    REQUIRE_FALSE(eeprom.driver.configure(Driver::Config{}));
    // Nothing completes automatically while the continuation is delayed.
    eeprom.submit();
    REQUIRE(eeprom.bus.pendingCount() == 1);
    REQUIRE(eeprom.driver.isBusy());
    eeprom.completeOne();
    REQUIRE_FALSE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.result() == Driver::Result::ERR_ARBITRATION_LOST);
    if (data_succeeds)
    {
        const auto buffer = eeprom.driver.buffer();
        REQUIRE(Bytes(buffer.begin(), buffer.end()) == Bytes(64, 0xa5));
    }
    REQUIRE(eeprom.driver.readPage(0));
    eeprom.tick();
    eeprom.tick();
    REQUIRE_FALSE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.result() == Driver::Result::SUCCESS);
}

TEST_CASE("Cat24cx drains both read transactions after an unexpected response", "[cat24cx][error]")
{
    Eeprom eeprom(4);
    REQUIRE(eeprom.driver.readPage(1));
    eeprom.submit();
    REQUIRE(eeprom.bus.pendingCount() == 2);

    SECTION("address response has the wrong opaque value")
    {
        eeprom.bus.front()->setOpaque(0xff);
        eeprom.completeOne();
        REQUIRE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.buffer().empty());
        eeprom.completeOne();
    }
    SECTION("data response arrives before the address response")
    {
        auto * address = eeprom.bus.pop();
        auto * data = eeprom.bus.pop();
        data->setStatus(Status::DONE);
        eeprom.driver.handleResponse(data, eeprom.now);
        eeprom.bus.release(data);
        REQUIRE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.buffer().empty());
        address->setStatus(Status::DONE);
        eeprom.driver.handleResponse(address, eeprom.now);
        eeprom.bus.release(address);
    }
    REQUIRE_FALSE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.result() == Driver::Result::ERR_BAD_ANSWER);
}

TEST_CASE("Cat24cx retries read allocation without leaking a transaction", "[cat24cx]")
{
    Eeprom eeprom(4);
    auto * first = eeprom.bus.allocate();
    auto * second = eeprom.bus.allocate();
    auto * third = eeprom.bus.allocate();
    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);
    REQUIRE(third != nullptr);
    REQUIRE(eeprom.driver.readPage(0));
    eeprom.submit();
    REQUIRE(eeprom.driver.isBusy());
    REQUIRE(eeprom.bus.pendingCount() == 0);
    eeprom.bus.release(first);
    eeprom.submit();
    REQUIRE(eeprom.bus.pendingCount() == 2);
    eeprom.completeOne();
    eeprom.completeOne();
    REQUIRE_FALSE(eeprom.driver.isBusy());
    REQUIRE(eeprom.driver.result() == Driver::Result::SUCCESS);
    eeprom.bus.release(second);
    eeprom.bus.release(third);
}

TEST_CASE("Cat24cx propagates data errors and retries ACK polling", "[cat24cx][error]")
{
    Eeprom eeprom(4);
    SECTION("data transaction fails")
    {
        eeprom.faults.push_back({Kind::READ_DATA, Status::ERR_BUS_ERROR});
        REQUIRE(eeprom.driver.readPage(0));
        eeprom.tick();
        REQUIRE(eeprom.driver.isBusy());
        eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.result() == Driver::Result::ERR_BUS_ERROR);
    }
    SECTION("write polls until the device acknowledges")
    {
        auto buffer = eeprom.driver.buffer();
        std::fill(buffer.begin(), buffer.end(), 0x42);
        eeprom.poll_nacks = 2;
        REQUIRE(eeprom.driver.writePage(3));
        for (unsigned i = 0; i < 20 && eeprom.driver.isBusy(); ++i)
            eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.result() == Driver::Result::SUCCESS);
        REQUIRE(eeprom.count(Kind::WRITE) == 1);
        REQUIRE(eeprom.count(Kind::POLL) == 3);
        REQUIRE(eeprom.getPage(3) == Bytes(64, 0x42));
    }
    SECTION("poll failure is reported after a committed write")
    {
        auto buffer = eeprom.driver.buffer();
        std::fill(buffer.begin(), buffer.end(), 0x39);
        eeprom.faults.push_back({Kind::POLL, Status::ERR_BUS_ERROR});
        REQUIRE(eeprom.driver.writePage(2));
        for (unsigned i = 0; i < 20 && eeprom.driver.isBusy(); ++i)
            eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.result() == Driver::Result::ERR_BUS_ERROR);
        REQUIRE(eeprom.getPage(2) == Bytes(64, 0x39));
    }
    SECTION("repeated poll NACKs eventually time out")
    {
        eeprom.poll_nacks = 100;
        REQUIRE(eeprom.driver.writePage(1));
        for (unsigned i = 0; i < 30 && eeprom.driver.isBusy(); ++i)
            eeprom.tick();
        REQUIRE_FALSE(eeprom.driver.isBusy());
        REQUIRE(eeprom.driver.result() == Driver::Result::ERR_TIMEOUT);
        REQUIRE(eeprom.count(Kind::POLL) > 1);
    }
}