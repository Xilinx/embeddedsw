/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sbrk.c
 * @brief sbrk stub implementation for the RISC-V standalone BSP.
 */

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include "xil_types.h"

extern u8 _heap_start[]; /**< Start of heap region (defined by linker script). */
extern u8 _heap_end[];   /**< End of heap region (defined by linker script). */

#ifdef __cplusplus
extern "C" {
  /**
 * @brief Stub for _sbrk(); manages heap end pointer.
 * @param[in] incr Number of bytes by which to grow the heap.
 * @return Previous heap end address, or (char*)-1 on failure.
 */
char * _sbrk ( ptrdiff_t incr );
}
#endif

/**
 * @brief Stub for _sbrk(); manages heap end pointer.
 * @param[in] incr Number of bytes by which to grow the heap.
 * @return Previous heap end address, or (char*)-1 on failure.
 */
char * _sbrk ( ptrdiff_t incr )
{
  static u8 *heap = NULL;
  u8 *prev_heap;

  if (heap == NULL) {
    heap = (u8 *)&_heap_start;
  }
  prev_heap = heap;

  /*
   * Validate grow and shrink in integer space before updating heap.
   * Do not compare (heap + incr) to _heap_end; a large incr can wrap.
   */
  if (incr >= 0) {
    if ((UINTPTR)incr > ((UINTPTR)&_heap_end - (UINTPTR)heap)) {
      errno = ENOMEM;
      return (char *)-1;
    }
  } else {
    /* Cast incr to unsigned to get the shrink size; safe even for PTRDIFF_MIN. */
    if (((UINTPTR)0 - (UINTPTR)incr) > ((UINTPTR)heap - (UINTPTR)&_heap_start)) {
      errno = ENOMEM;
      return (char *)-1;
    }
  }

  heap += incr;
  return (char *)prev_heap;
}
