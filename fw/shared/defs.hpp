/**
 * @file
 */

#ifndef SHARED_DEFS_HPP_
#define SHARED_DEFS_HPP_

#ifdef STM32
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif


#ifdef STM32
#define AUTOSELECT_VALUE(stm32, att) stm32
#else
#define AUTOSELECT_VALUE(stm32, att) att
#endif

namespace def
{

using SmallSize = AUTOSELECT_VALUE(std::uint16_t, uint8_t);
using Size = AUTOSELECT_VALUE(std::size_t, size_t);

using Int8 = AUTOSELECT_VALUE(std::int8_t, int8_t);
using Uint8 = AUTOSELECT_VALUE(std::uint8_t, uint8_t);

using Int16 = AUTOSELECT_VALUE(std::int16_t, int16_t);
using Uint16 = AUTOSELECT_VALUE(std::uint16_t, uint16_t);

using Int32 = AUTOSELECT_VALUE(std::int32_t, int32_t);
using Uint32 = AUTOSELECT_VALUE(std::uint32_t, uint32_t);

using IntPtr = AUTOSELECT_VALUE(std::intptr_t, intptr_t);
using UintPtr = AUTOSELECT_VALUE(std::uintptr_t, uintptr_t);

}  // namespace def

#endif  // SHARED_DEFS_HPP_
