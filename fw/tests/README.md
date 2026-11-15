# Host tests

From this directory, build with `make -j8 -O` and build and run with `make test`.
The build requires a C++20 compiler and Catch2 v3 (including its main library).
To run a subset after building, use `_build/tests.exe '[cat24cx]'` or
`_build/tests.exe '[records]'`. The existing test cases are retained.

The harness compiles the production `Records`, CRC and `Cat24cx` sources. Only
the I2C bus interface and interrupt guard are shadowed in `host_include`.
`test::Eeprom` owns the driver, bus, independent EEPROM bytes and simulated
clock. `submit()` advances the clock and lets the driver enqueue requests;
`completeOne()` finishes exactly one queued transaction; `tick()` does both.
The two transactions of a page read remain independently pending, including
after an address failure. Tests can inspect `events`, set pages, queue `Fault`
objects by transaction kind, enable write protection, return altered readback
data, and choose a number of NACKs before the ACK poll succeeds. A write fault
can optionally commit to media before reporting failure. `test::Harness::run()`
alternates Records calls with driver ticks, with a bounded step limit.

This is a deterministic single-threaded transaction model, not an emulation of
STM32 interrupts, electrical timing, arbitration, page-write power-loss
behavior, or actual bus hardware. Hardware validation is still required. The
Records tests target the revised six-byte-header and cursor API and will not
compile against the previous unfinished Records interface.