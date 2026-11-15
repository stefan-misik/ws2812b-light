#include "eeprom_harness.hpp"
#include "app/records/crc.hpp"

using namespace test;

TEST_CASE("CCITT-FALSE agrees with independent golden vectors", "[records][crc]")
{
    constexpr char input[] = "123456789";
    Crc crc;
    crc.update(input, 4);
    crc.update(input + 4, 5);
    REQUIRE(static_cast<std::uint16_t>(crc) == 0x29b1);
    REQUIRE(referenceCrc(Bytes(input, input + 9)) == 0x29b1);
    const auto encoded = page(5, 1, {0x11, 0x22, 0x33});
    REQUIRE(Bytes(encoded.begin(), encoded.begin() + 9) ==
            Bytes{5, 3, 0, 1, 0xe1, 0x80, 0x11, 0x22, 0x33});
}

TEST_CASE("Verification fixture produces a genuine same-header CRC collision", "[crc]")
{
    const auto original = page(1, 2, {0x20, 0x21, 0x22});
    auto collision = collisionPage(1, 2, {0x20, 0x21, 0x22});
    REQUIRE(collision != original);
    REQUIRE(collision[4] == original[4]);
    REQUIRE(collision[5] == original[5]);
    collision[4] = 0xff;
    collision[5] = 0xff;
    Crc crc;
    crc.update(collision.data(), 9);
    REQUIRE(static_cast<std::uint16_t>(crc) ==
            static_cast<std::uint16_t>((original[4] << 8) | original[5]));
}