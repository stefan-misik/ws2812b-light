#ifndef TESTS_STUBS_CMSIS_COMPILER_H_
#define TESTS_STUBS_CMSIS_COMPILER_H_

#include <cstdint>

extern std::uint32_t test_i2c_primask;

inline void __DMB() { }
inline void __ISB() { }
inline std::uint32_t __get_PRIMASK() { return test_i2c_primask; }
inline void __set_PRIMASK(std::uint32_t value) { test_i2c_primask = value; }
inline void __disable_irq() { test_i2c_primask = 1; }

#endif  // TESTS_STUBS_CMSIS_COMPILER_H_