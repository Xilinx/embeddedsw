/******************************************************************************
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file getentropy.c
 * @brief getentropy stub implementation for standalone BSP.
 */


#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) sint32 getentropy(void *buffer, sint32 length);

#ifdef __cplusplus
}
#endif

/**
 * @brief Stub for getentropy(); always returns -1 (not supported).
 * @param[out] buffer Buffer for entropy data (unused).
 * @param[in] length Buffer length in bytes (unused).
 * @return Always -1.
 */
__attribute__((weak)) sint32 getentropy(void *buffer, sint32 length)
{
  (void)buffer;
  (void)length;

  errno = ENOSYS;
  return -1;
}
