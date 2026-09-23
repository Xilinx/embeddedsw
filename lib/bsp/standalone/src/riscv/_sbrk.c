/******************************************************************************
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <sys/types.h>
#include <errno.h>
#include <stddef.h>
#include "xil_types.h"

extern u8 _heap_start[];
extern u8 _heap_end[];

#ifdef __cplusplus
extern "C" {
  char * _sbrk ( ptrdiff_t incr );
}
#endif

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
