/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) sint32 getentropy(void *buffer, sint32 length);

#ifdef __cplusplus
}
#endif

__attribute__((weak)) sint32 getentropy(void *buffer, sint32 length)
{
  (void)buffer;
  (void)length;

  errno = ENOSYS;
  return -1;
}
