# Repository guidance

## Layout
- `fw/stm32g0/`: STM32G070 firmware; application code is under `app/`.
- `fw/att85/`: ATtiny85 firmware (separate implementation and toolchain).
- `fw/shared/`: shared code between the `stm32f0` and `att85`
- `fw/tests/`: host-side Catch2 tests for firmware logic.
- `tool/`: Python utilities for testing selected components on desktop
- `hw/`, `mech/`: hardware designs and FreeCAD enclosure files.

## Build and validate
Run commands from the indicated directory; there is no root build. Use the existing per-folder VS Code tasks when available. Builds require GNU Make and a Unix-like shell (Bash on Windows).

| Directory | Command | Requirements |
| --- | --- | --- |
| `fw/stm32g0/` | `make -j8 -O DBG=yes` (or `DBG=no`) | ARM GNU toolchain (`arm-none-eabi-*`) |
| `fw/att85/` | `make -j8 -O` | AVR GNU toolchain |
| `fw/tests/` | `make -j8 -O`, then `./_build/tests.exe` | Native C++ compiler and Catch2 libraries |
| `tool/` | `make -j8 -O DBG=yes PYTHON=<interpreter>` | Native C++ compiler, Python development libraries, pybind11 |

- Firmware builds generate song includes using Python. Edit `.tune` inputs or `fw/shared/songs/compose.py`, not generated `.inc` files.
- For logic changes, add/update Catch2 cases under `fw/tests/app/` and register new sources in its Makefile. Building tests does not execute them.
- Build affected targets; changes to shared code may affect both firmware targets and the host tools. Report validation that could not run because of missing toolchains or hardware.
- Do not flash devices, program fuses, or start hardware debug sessions unless explicitly requested.
- After removing source files, run the affected project's Clean task before its next Build task to remove stale dependency files (`*.d`).

## Change conventions
- Use C++20 for STM32, tests, and the extension; ATtiny85 uses C++14. Keep shared code compatible with its consumers, and use types defined in `fw/shared/defs.hpp`.
- Embedded builds disable exceptions and RTTI (AVR-GCC does not support RTTI).
- Match surrounding naming, indentation, brace style, and documentation; existing files use different indentation styles. Avoid unrelated reformatting.
- Prefer application-level changes over modifications to STM32 vendor libraries. Preserve CubeMX user-code sections when editing generated support code.
- Do not hand-edit build outputs, caches, or compilation databases (`_build/`, `Debug/`, `Release/`, `compile_commands.json`). Keep editor build/debug configuration in each project's `.vscode/` folder; root configuration is for shared operations.

## Code style
- When a function performs an action and returns a value used for control flow, call it in a separate statement and store the result in a descriptively named `const` variable before testing it. This makes the action and the subsequent decision explicit.