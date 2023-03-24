/******************************************************************************
* Copyright (c) 2023 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include <sys/types.h>
#include <errno.h>
#include "xil_types.h"

extern u8 _heap_start[];
extern u8 _heap_end[];

#ifdef __cplusplus
extern "C" {
  char * _sbrk ( int incr );
}
#endif

char * _sbrk ( int incr )
{
  static u8 *heap = NULL;
  u8 *prev_heap;
  static u8 *HeapEndPtr = (u8 *)&_heap_end;
  char * Status;

  if (heap == NULL) {
    heap = (u8 *)&_heap_start;
  }
  prev_heap = heap;

  if (((heap + incr) <= HeapEndPtr) && (prev_heap != NULL)) {
    heap += incr;
    Status = (char *)prev_heap;
  } else {
    errno = ENOMEM;
    Status = (char *)-1;
  }

  return Status;
}
