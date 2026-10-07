/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file sbrk.c
 * @brief sbrk memory allocation stub for standalone BSP.
 */


#include <errno.h>
#include <stddef.h>
#include "xil_types.h"
#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) char8 *sbrk (ptrdiff_t nbytes);

#ifdef __cplusplus
}
#endif

extern u8 _heap_start[];   /**< Start of heap region (defined by linker script). */
extern u8 _heap_end[];     /**< End of heap region (defined by linker script). */
extern char8 HeapBase[];   /**< Heap base address (defined by linker script). */
extern char8 HeapLimit[];  /**< Heap limit address (defined by linker script). */



/**
 * @brief Stub for sbrk(); manages heap end pointer.
 * @param[in] nbytes Number of bytes to grow (positive) or shrink (negative) the heap.
 * @return Pointer to previous heap end, or (char8*)-1 on failure.
 */
__attribute__((weak)) char8 *sbrk (ptrdiff_t nbytes)
{
  char8 *base;
  static char8 *heap_ptr = HeapBase;

  base = heap_ptr;

  /*
   * Check both bounds in integer space (no pointer overflow/UB). Unsigned
   * negation of nbytes gives its magnitude, avoiding PTRDIFF_MIN overflow.
   */
  if (nbytes >= 0) {
    if ((UINTPTR)nbytes > ((UINTPTR)HeapLimit - (UINTPTR)heap_ptr)) {
      errno = ENOMEM;
      return ((char8 *)-1);
    }
  } else {
    if (((UINTPTR)0 - (UINTPTR)nbytes) > ((UINTPTR)heap_ptr - (UINTPTR)HeapBase)) {
      errno = ENOMEM;
      return ((char8 *)-1);
    }
  }

  heap_ptr += nbytes;
  return base;
}
