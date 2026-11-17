/**
 * @file
 */

#ifndef SUPPORT_SYSMEM_H_
#define SUPPORT_SYSMEM_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Get the current end of the heap.
 *
 * @return The current end of the heap.
 */
size_t heap_end();

#ifdef __cplusplus
}
#endif

#endif  // SUPPORT_SYSMEM_H_