#include <catch2/catch_test_macros.hpp>

#include "app/storage/block_counter.hpp"

TEST_CASE("BlockCounter starts at zero")
{
	const BlockCounter counter;

	CHECK(counter.current() == 0);
	CHECK(counter.address() == 0);
}

TEST_CASE("BlockCounter reset clears the count and selects the base address")
{
	BlockCounter counter;

	counter.next();
	counter.next();
	counter.reset(17);

	CHECK(counter.current() == 0);
	CHECK(counter.address() == 17);

	counter.reset();

	CHECK(counter.current() == 0);
	CHECK(counter.address() == 0);
}

TEST_CASE("BlockCounter next advances address and count in lockstep")
{
	BlockCounter counter;

	counter.reset(9);

	for (BlockCounter::CounterType step = 1; step <= 4; ++step)
	{
		counter.next();

		CHECK(counter.current() == step);
		CHECK(counter.address() == static_cast<std::uint16_t>(9 + step));
	}
}

TEST_CASE("BlockCounter rewind ignores missing counter values")
{
	BlockCounter counter;

	counter.reset(4);
	counter.next();
	counter.next();

	counter.rewind(std::nullopt);

	CHECK(counter.current() == 2);
	CHECK(counter.address() == 6);
}
