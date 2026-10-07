/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file _sbrk.c
 * @brief sbrk stub implementation for ARM compilers.
 */


#include <sys/types.h>
#include <stddef.h>
#include "xil_types.h"

extern u8 _heap_start[]; /**< Start of heap region (defined by linker script). */
extern u8 _heap_end[];   /**< End of heap region (defined by linker script). */

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) caddr_t _sbrk ( ptrdiff_t incr );

#ifdef __cplusplus
}
#endif

/**
 * @brief Stub for _sbrk(); manages heap by returning the previous heap end.
 * @param[in] incr Increment in bytes to grow (positive) or shrink (negative) the heap.
 * @return Previous heap end address, or (caddr_t)-1 on failure.
 */
__attribute__((weak)) caddr_t _sbrk ( ptrdiff_t incr )
{
  static u8 *heap = NULL;
  u8 *prev_heap;

  if (heap == NULL) {
    heap = (u8 *)&_heap_start;
  }
  prev_heap = heap;

  /*
   * Check both bounds in integer space (no pointer overflow/UB). Unsigned
   * negation of incr gives its magnitude, avoiding PTRDIFF_MIN overflow.
   */
  if (incr >= 0) {
    if ((UINTPTR)incr > ((UINTPTR)&_heap_end - (UINTPTR)heap)) {
      return (caddr_t) -1;
    }
  } else {
    if (((UINTPTR)0 - (UINTPTR)incr) > ((UINTPTR)heap - (UINTPTR)&_heap_start)) {
      return (caddr_t) -1;
    }
  }

  heap += incr;

  return (caddr_t) ((void *)prev_heap);
}
