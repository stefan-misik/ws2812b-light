#include "eeprom_harness.hpp"

#include <limits>
#include <type_traits>

using namespace test;

static_assert(!std::is_copy_constructible_v<Records>);
static_assert(!std::is_copy_assignable_v<Records>);
static_assert(!std::is_move_constructible_v<Records>);
static_assert(!std::is_move_assignable_v<Records>);

TEST_CASE("Blank initialization and first write use the six-byte format", "[records][write]")
{
    Harness h(4);
    REQUIRE(Records::RECORD_HEADER_SIZE == 6);
    const Records & const_records = h.records;
    REQUIRE(const_records.getNextPage() == 0);
    REQUIRE_FALSE(h.records.setNextPage(1));
    REQUIRE_FALSE(h.records.beginRead(0));
    REQUIRE_FALSE(h.records.beginWrite(0, 0));
    REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE_FALSE(h.records.isBusy());
    REQUIRE(h.eeprom.count(Kind::READ_DATA) == 4);
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    const auto [buffer, capacity] = h.records.getRecordBuffer(&h.eeprom.driver);
    REQUIRE(buffer != nullptr);
    REQUIRE(capacity == 58);
    h.eeprom.poll_nacks = 2;
    REQUIRE(h.write(5, {0x11, 0x22, 0x33}) == Records::SUCCESS);
    const auto encoded = h.eeprom.getPage(0);
    REQUIRE(Bytes(encoded.begin(), encoded.begin() + 9) ==
            Bytes{5, 3, 0, 1, 0xe1, 0x80, 0x11, 0x22, 0x33});
    REQUIRE(h.eeprom.count(Kind::WRITE) == 1);
    REQUIRE(h.eeprom.count(Kind::POLL) == 3);
    REQUIRE(h.eeprom.count(Kind::READ_DATA) == 5); // scan plus readback
    REQUIRE(h.records.getNextPage() == 1);
    REQUIRE(h.data() == Bytes{0x11, 0x22, 0x33});
    REQUIRE(h.read(5) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x11, 0x22, 0x33});
}

TEST_CASE("Zero and maximum-length records are valid", "[records][bounds]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    SECTION("empty record has a non-null data pointer")
    {
        REQUIRE(h.write(0, {}) == Records::SUCCESS);
        const auto [pointer, length] = h.records.getRecordData(&h.eeprom.driver);
        REQUIRE(pointer != nullptr);
        REQUIRE(length == 0);
        REQUIRE(h.read(0) == Records::SUCCESS);
        REQUIRE(h.records.getRecordData(&h.eeprom.driver).first != nullptr);
        REQUIRE(h.data().empty());
    }
    SECTION("full 58-byte payload survives write and read")
    {
        Bytes payload(58);
        for (std::size_t i = 0; i < payload.size(); ++i)
            payload[i] = static_cast<Byte>(i);
        REQUIRE(h.write(63, payload) == Records::SUCCESS);
        REQUIRE(h.eeprom.getPage(0) == page(63, 1, payload));
        REQUIRE(h.read(63) == Records::SUCCESS);
        REQUIRE(h.data() == payload);
    }
}

TEST_CASE("Invalid IDs, lengths, and EEPROM configurations fail safely", "[records][bounds]")
{
    SECTION("null driver and unsupported small pages")
    {
        Records records;
        REQUIRE(records.run(nullptr, 0) == Records::ERR_BAD_CONFIG);
        REQUIRE(records.getRecordData(nullptr).first == nullptr);
        REQUIRE(records.getRecordBuffer(nullptr).first == nullptr);
        Harness h(4, 5);
        REQUIRE(h.run() == Records::ERR_BAD_CONFIG);
        REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    }
    SECTION("absolute and configured page length limits")
    {
        Harness h(4, 8);
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE_FALSE(h.records.beginRead(64));
        REQUIRE_FALSE(h.records.beginRead(Records::INVALID_RECORD_ID));
        REQUIRE_FALSE(h.records.beginWrite(64, 0));
        REQUIRE_FALSE(h.records.beginWrite(0, 59));
        REQUIRE_FALSE(h.records.beginWrite(0, std::numeric_limits<std::size_t>::max()));
        const auto [buffer, capacity] = h.records.getRecordBuffer(&h.eeprom.driver);
        REQUIRE(buffer != nullptr);
        REQUIRE(capacity == 2);
        REQUIRE(h.records.beginWrite(0, 3));
        REQUIRE(h.run() == Records::ERR_BAD_DATA_LENGTH);
        REQUIRE(h.eeprom.count(Kind::WRITE) == 0);
        REQUIRE(h.records.getNextPage() == 0);
    }
    SECTION("header-only pages permit empty payloads")
    {
        Harness h(3, 6);
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.write(0, {}) == Records::SUCCESS);
        REQUIRE(h.read(0) == Records::SUCCESS);
        REQUIRE(h.data().empty());
    }
    SECTION("a bound store rejects a different driver")
    {
        Harness h;
        Eeprom other;
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.records.run(&other.driver, 0) == Records::ERR_BAD_CONFIG);
        REQUIRE(h.records.getRecordBuffer(&other.driver).first == nullptr);
        REQUIRE(h.records.getRecordData(&other.driver).first == nullptr);
    }
    SECTION("a bound store rejects a reconfigured driver")
    {
        Harness h;
        REQUIRE(h.run() == Records::SUCCESS);
        auto config = Driver::Config{};
        config.capacity = 8 * 64;
        config.page_size = 32;
        REQUIRE(h.eeprom.driver.configure(config));
        REQUIRE(h.records.run(&h.eeprom.driver, 0) == Records::ERR_BAD_CONFIG);
        REQUIRE_FALSE(h.records.setNextPage(1));
        REQUIRE_FALSE(h.records.beginRead(0));
        REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    }
}

TEST_CASE("Scans ignore invalid pages, lengths, CRCs, and padding", "[records][scan]")
{
    Harness h(7);
    auto old = page(2, 5, {0x10});
    old[63] = 0; // Padding does not participate in CRC.
    h.eeprom.setPage(0, old);
    auto corrupt_new = page(2, 6, {0x20});
    corrupt_new[6] ^= 0x40;
    h.eeprom.setPage(1, corrupt_new);
    auto bad_length = page(3, 1, {});
    bad_length[1] = 59;
    h.eeprom.setPage(2, bad_length);
    h.eeprom.setPage(3, page(64, 1, {0x66}));
    h.eeprom.setPage(4, page(0xff, 1, {0x77}));
    auto torn = page(4, 1, {0x55});
    torn[4] = 0xff;
    torn[5] = 0xff;
    h.eeprom.setPage(5, torn);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.read(2) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x10});
    REQUIRE(h.read(3) == Records::NOT_FOUND);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.read(4) == Records::NOT_FOUND);
}

TEST_CASE("Scan epoch ordering wraps and resolves ambiguity by first page", "[records][scan][epoch]")
{
    Harness h(6);
    SECTION("zero is newer than FFFF")
    {
        h.eeprom.setPage(0, page(1, 0xffff, {1}));
        h.eeprom.setPage(1, page(1, 0, {2}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.read(1) == Records::SUCCESS);
        REQUIRE(h.data() == Bytes{2});
        REQUIRE(h.write(1, {3}) == Records::SUCCESS);
        requirePage(h.eeprom.getPage(0), 1, 1, {3});
    }
    SECTION("equal epochs retain the first scanned page")
    {
        h.eeprom.setPage(0, page(1, 10, {1}));
        h.eeprom.setPage(1, page(1, 10, {2}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.read(1) == Records::SUCCESS);
        REQUIRE(h.data() == Bytes{1});
    }
    SECTION("exactly half-range epochs retain the first scanned page")
    {
        h.eeprom.setPage(0, page(1, 0, {1}));
        h.eeprom.setPage(1, page(1, 0x8000, {2}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.read(1) == Records::SUCCESS);
        REQUIRE(h.data() == Bytes{1});
    }
    SECTION("half-range is ambiguous in the opposite direction too")
    {
        h.eeprom.setPage(0, page(1, 0x8000, {1}));
        h.eeprom.setPage(1, page(1, 0, {2}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.read(1) == Records::SUCCESS);
        REQUIRE(h.data() == Bytes{1});
    }
    SECTION("a modular difference beyond half-range is older")
    {
        h.eeprom.setPage(0, page(1, 0, {1}));
        h.eeprom.setPage(1, page(1, 0x8001, {2}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.read(1) == Records::SUCCESS);
        REQUIRE(h.data() == Bytes{1});
    }
    SECTION("increment wraps on a verified write and restart")
    {
        h.eeprom.setPage(0, page(1, 0xffff, {1}));
        REQUIRE(h.run() == Records::SUCCESS);
        REQUIRE(h.write(1, {2}) == Records::SUCCESS);
        requirePage(h.eeprom.getPage(1), 1, 0, {2});
        Harness restart(6);
        for (std::size_t i = 0; i < 6; ++i)
            restart.eeprom.setPage(i, h.eeprom.getPage(i));
        REQUIRE(restart.run() == Records::SUCCESS);
        REQUIRE(restart.read(1) == Records::SUCCESS);
        REQUIRE(restart.data() == Bytes{2});
    }
}

TEST_CASE("Cursor restoration skips live pages and wraps", "[records][cursor]")
{
    Harness h(5);
    // The configuration record stores an old cursor, captured before its own write.
    h.eeprom.setPage(0, page(0, 1, {0}));
    h.eeprom.setPage(1, page(1, 4, {0xa1}));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.read(0) == Records::SUCCESS);
    REQUIRE(h.records.setNextPage(h.data().at(0)));
    REQUIRE_FALSE(h.records.setNextPage(5));
    REQUIRE_FALSE(h.records.setNextPage(Records::INVALID_PAGE_ID));
    REQUIRE(h.records.getNextPage() == 0);
    const auto config_page = h.eeprom.getPage(0);
    const auto live_page = h.eeprom.getPage(1);
    REQUIRE(h.write(2, {0xa2}) == Records::SUCCESS);
    REQUIRE(h.eeprom.getPage(0) == config_page);
    REQUIRE(h.eeprom.getPage(1) == live_page);
    requirePage(h.eeprom.getPage(2), 2, 1, {0xa2});
    REQUIRE(h.records.getNextPage() == 3);
    REQUIRE(h.records.setNextPage(4));
    REQUIRE(h.write(3, {0xa3}) == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.write(4, {0xa4}) == Records::SUCCESS);
    requirePage(h.eeprom.getPage(3), 4, 1, {0xa4});
    REQUIRE(h.records.getNextPage() == 4);
}

TEST_CASE("No space preserves all live pages and the committed cursor", "[records][cursor]")
{
    Harness h(3);
    for (Byte id = 0; id < 3; ++id)
        h.eeprom.setPage(id, page(id, 1, {id}));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.setNextPage(1));
    const auto before = h.eeprom.media();
    REQUIRE(h.write(0, {0xfe}) == Records::ERR_NO_SPACE);
    REQUIRE(h.eeprom.media() == before);
    REQUIRE(h.eeprom.count(Kind::WRITE) == 0);
    REQUIRE(h.records.getNextPage() == 1);
    REQUIRE(h.read(0) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0});
}

TEST_CASE("A stale persisted cursor after restart cannot overwrite live records", "[records][cursor]")
{
    Harness before(5);
    REQUIRE(before.run() == Records::SUCCESS);
    REQUIRE(before.write(0, {0}) == Records::SUCCESS); // cursor captured before this write
    REQUIRE(before.write(1, {0x11}) == Records::SUCCESS);
    const auto config_page = before.eeprom.getPage(0);
    const auto live_page = before.eeprom.getPage(1);
    Harness after(5);
    for (std::size_t i = 0; i < 5; ++i)
        after.eeprom.setPage(i, before.eeprom.getPage(i));
    REQUIRE(after.run() == Records::SUCCESS);
    REQUIRE(after.records.getNextPage() == 0);
    REQUIRE(after.read(0) == Records::SUCCESS);
    REQUIRE(after.records.setNextPage(after.data().at(0)));
    REQUIRE(after.write(2, {0x22}) == Records::SUCCESS);
    REQUIRE(after.eeprom.getPage(0) == config_page);
    REQUIRE(after.eeprom.getPage(1) == live_page);
    requirePage(after.eeprom.getPage(2), 2, 1, {0x22});
    REQUIRE(after.read(1) == Records::SUCCESS);
    REQUIRE(after.data() == Bytes{0x11});
}

TEST_CASE("Page search yields before examining more than ten occupied pages", "[records][async]")
{
    Harness h(22);
    for (Byte id = 0; id < 21; ++id)
        h.eeprom.setPage(id, page(id, 1, {id}));
    REQUIRE(h.run() == Records::SUCCESS);
    const auto [buffer, capacity] = h.records.getRecordBuffer(&h.eeprom.driver);
    REQUIRE(buffer != nullptr);
    REQUIRE(capacity >= 1);
    static_cast<Byte *>(buffer)[0] = 0xa5;
    REQUIRE(h.records.beginWrite(20, 1));
    for (unsigned i = 0; i < 2; ++i)
    {
        REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
        h.eeprom.tick();
        REQUIRE(h.eeprom.count(Kind::WRITE) == 0);
    }
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.run() == Records::SUCCESS);
    requirePage(h.eeprom.getPage(21), 20, 2, {0xa5});
    REQUIRE(h.records.getNextPage() == 0);
}

TEST_CASE("Getters and cursor setter respect operation and buffer ownership", "[records][async]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x12}) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x12});
    REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first != nullptr);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.read(1) == Records::SUCCESS);
    REQUIRE(h.records.beginRead(1));
    REQUIRE_FALSE(h.records.beginRead(1));
    REQUIRE_FALSE(h.records.beginWrite(1, 0));
    REQUIRE_FALSE(h.records.setNextPage(3));
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x12});
    REQUIRE(h.eeprom.driver.readPage(3));
    REQUIRE_FALSE(h.records.setNextPage(3));
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    h.eeprom.tick();
    h.eeprom.tick();
    REQUIRE_FALSE(h.eeprom.driver.isBusy());
    REQUIRE(h.records.setNextPage(3));
}

TEST_CASE("Reads reject changed page identity, bounds, and CRC", "[records][read][error]")
{
    Harness h(4);
    h.eeprom.setPage(0, page(1, 2, {0x42}));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.read(1) == Records::SUCCESS);
    auto replacement = page(1, 2, {0x42});
    SECTION("different ID with a valid CRC") { replacement = page(2, 2, {0x42}); }
    SECTION("different epoch with a valid CRC") { replacement = page(1, 3, {0x42}); }
    SECTION("corrupt CRC") { replacement[4] ^= 1; }
    SECTION("length beyond the page") { replacement[1] = 59; }
    h.eeprom.setPage(0, replacement);
    REQUIRE(h.read(1) == Records::ERR_CORRUPT_RECORD);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
}

TEST_CASE("Records waits for read-address error to drain", "[records][read][error]")
{
    Harness h(4);
    h.eeprom.setPage(0, page(1, 1, {0x42}));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.read(1) == Records::SUCCESS);
    h.eeprom.faults.push_back({Kind::READ_ADDRESS, Status::ERR_NO_ANSWER});
    REQUIRE(h.records.beginRead(1));
    const auto address_count = h.eeprom.count(Kind::READ_ADDRESS);
    for (unsigned i = 0; i < 10 && h.eeprom.count(Kind::READ_ADDRESS) == address_count; ++i)
    {
        REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
        h.eeprom.tick();
    }
    REQUIRE(h.eeprom.count(Kind::READ_ADDRESS) == address_count + 1);
    REQUIRE(h.eeprom.bus.pendingCount() == 1);
    REQUIRE(h.eeprom.driver.isBusy());
    REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.records.getRecordBuffer(&h.eeprom.driver).first == nullptr);
    REQUIRE_FALSE(h.records.setNextPage(2));
    h.eeprom.completeOne();
    REQUIRE(h.run() == Records::EEPROM_ERROR);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
}

TEST_CASE("Records propagates data read failures without exposing stale data", "[records][read][error]")
{
    Harness h(4);
    h.eeprom.setPage(0, page(1, 1, {0x42}));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.read(1) == Records::SUCCESS);
    h.eeprom.faults.push_back({Kind::READ_DATA, Status::ERR_BUS_ERROR});
    REQUIRE(h.read(1) == Records::EEPROM_ERROR);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE(h.read(1) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x42});
}

TEST_CASE("Write remains pending until verification data completes", "[records][write][async]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    const auto [buffer, capacity] = h.records.getRecordBuffer(&h.eeprom.driver);
    REQUIRE(buffer != nullptr);
    REQUIRE(capacity >= 1);
    static_cast<Byte *>(buffer)[0] = 0x42;
    REQUIRE(h.records.beginWrite(1, 1));
    const auto reads_before = h.eeprom.count(Kind::READ_ADDRESS);
    for (unsigned i = 0; i < 30 && h.eeprom.count(Kind::READ_ADDRESS) == reads_before; ++i)
    {
        REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
        h.eeprom.tick();
    }
    REQUIRE(h.eeprom.count(Kind::WRITE) == 1);
    REQUIRE(h.eeprom.count(Kind::POLL) >= 1);
    REQUIRE(h.eeprom.count(Kind::READ_ADDRESS) == reads_before + 1);
    REQUIRE(h.eeprom.bus.pendingCount() == 1);
    REQUIRE(h.records.getNextPage() == 0);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE_FALSE(h.records.setNextPage(3));
    REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
    h.eeprom.completeOne();
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 1);
    REQUIRE(h.data() == Bytes{0x42});
}

TEST_CASE("A failed scan discards partial state and may be restarted", "[records][scan][error]")
{
    Harness h(4);
    h.eeprom.setPage(0, page(1, 1, {0x12}));
    // Let page 0 complete, then fail a later page after the partial TOC exists.
    for (unsigned i = 0; i < 20 && h.eeprom.count(Kind::READ_DATA) == 0; ++i)
    {
        REQUIRE(h.records.run(&h.eeprom.driver, h.eeprom.now) == Records::PENDING);
        h.eeprom.tick();
    }
    REQUIRE(h.eeprom.count(Kind::READ_DATA) == 1);
    h.eeprom.faults.push_back({Kind::READ_DATA, Status::ERR_BUS_ERROR});
    REQUIRE(h.run() == Records::EEPROM_ERROR);
    REQUIRE(h.records.isBusy());
    REQUIRE_FALSE(h.records.beginRead(1));
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    h.eeprom.setPage(0, Bytes(64, 0xff));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.read(1) == Records::NOT_FOUND);
}

TEST_CASE("A torn newest page falls back to the previous copy after restart", "[records][scan][error]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x10}) == Records::SUCCESS);
    REQUIRE(h.write(1, {0x20}) == Records::SUCCESS);
    auto torn = h.eeprom.getPage(1);
    torn[6] ^= 1;
    h.eeprom.setPage(1, torn);
    Harness restart(4);
    for (std::size_t i = 0; i < 4; ++i)
        restart.eeprom.setPage(i, h.eeprom.getPage(i));
    REQUIRE(restart.run() == Records::SUCCESS);
    REQUIRE(restart.read(1) == Records::SUCCESS);
    REQUIRE(restart.data() == Bytes{0x10});
}

TEST_CASE("Readback ignores padding while verifying meaningful bytes", "[records][write]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x10}) == Records::SUCCESS);
    h.eeprom.read_override = page(1, 2, {0x20});
    h.eeprom.read_override[63] = 0;
    REQUIRE(h.write(1, {0x20}) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x20});
}

TEST_CASE("Verification errors force a rescan without advancing the cursor", "[records][write][error]")
{
    Harness h(5);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x10}) == Records::SUCCESS);
    REQUIRE(h.records.setNextPage(3));
    const auto old_page = h.eeprom.getPage(0);
    Records::Result expected = Records::ERR_WRITE_VERIFY;
    SECTION("write-protected device leaves the target unchanged")
    {
        h.eeprom.write_protected = true;
    }
    SECTION("corrupted verification data fails validation")
    {
        h.eeprom.faults.push_back({Kind::READ_DATA, Status::DONE, false, 6});
    }
    SECTION("a CRC collision with different payload still fails byte comparison")
    {
        const Bytes original{0x20, 0x21, 0x22};
        const auto original_page = page(1, 2, original);
        h.eeprom.read_override = collisionPage(1, 2, original);
        REQUIRE(h.eeprom.read_override != original_page);
        REQUIRE(h.eeprom.read_override[4] == original_page[4]);
        REQUIRE(h.eeprom.read_override[5] == original_page[5]);
    }
    SECTION("verification read fails")
    {
        h.eeprom.faults.push_back({Kind::READ_DATA, Status::ERR_BUS_ERROR});
        expected = Records::EEPROM_ERROR;
    }
    REQUIRE(h.write(1, {0x20, 0x21, 0x22}) == expected);
    REQUIRE(h.records.isBusy());
    REQUIRE(h.records.getNextPage() == 3);
    REQUIRE(h.records.getRecordData(&h.eeprom.driver).first == nullptr);
    REQUIRE_FALSE(h.records.beginRead(1));
    REQUIRE(h.eeprom.getPage(0) == old_page);
    h.eeprom.write_protected = false;
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 3);
    REQUIRE(h.read(1) == Records::SUCCESS);
    // A verification failure need not mean the underlying write was lost.
    const auto expected_data = h.eeprom.getPage(3) == Bytes(64, 0xff) ? Bytes{0x10} : Bytes{0x20, 0x21, 0x22};
    REQUIRE(h.data() == expected_data);
}

TEST_CASE("A committed but reported-failed write is discovered on rescan", "[records][write][error]")
{
    Harness h(5);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x10}) == Records::SUCCESS);
    REQUIRE(h.records.setNextPage(3));
    SECTION("write transfer reports an error after committing")
    {
        h.eeprom.faults.push_back({Kind::WRITE, Status::ERR_BUS_ERROR, true});
    }
    SECTION("ACK poll reports an error after the transfer")
    {
        h.eeprom.faults.push_back({Kind::POLL, Status::ERR_ARBITRATION_LOST});
    }
    REQUIRE(h.write(1, {0x20}) == Records::EEPROM_ERROR);
    REQUIRE(h.records.isBusy());
    REQUIRE(h.records.getNextPage() == 3);
    requirePage(h.eeprom.getPage(0), 1, 1, {0x10});
    requirePage(h.eeprom.getPage(3), 1, 2, {0x20});
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 3);
    REQUIRE(h.read(1) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x20});
    REQUIRE(h.write(2, {0x30}) == Records::SUCCESS);
    requirePage(h.eeprom.getPage(3), 1, 2, {0x20});
    requirePage(h.eeprom.getPage(4), 2, 1, {0x30});
}

TEST_CASE("An uncommitted write error preserves the previous record after rescan", "[records][write][error]")
{
    Harness h(4);
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.write(1, {0x10}) == Records::SUCCESS);
    REQUIRE(h.records.setNextPage(2));
    h.eeprom.faults.push_back({Kind::WRITE, Status::ERR_BUS_ERROR});
    REQUIRE(h.write(1, {0x20}) == Records::EEPROM_ERROR);
    REQUIRE(h.records.isBusy());
    REQUIRE(h.records.getNextPage() == 2);
    REQUIRE(h.eeprom.getPage(2) == Bytes(64, 0xff));
    REQUIRE(h.run() == Records::SUCCESS);
    REQUIRE(h.records.getNextPage() == 2);
    REQUIRE(h.read(1) == Records::SUCCESS);
    REQUIRE(h.data() == Bytes{0x10});
}