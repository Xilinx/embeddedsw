/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file lseek.c
 * @brief lseek stub implementation for standalone BSP.
 */


#include <sys/types.h>
#include <errno.h>
#include "xil_types.h"

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) off_t _lseek(s32 fd, off_t offset, s32 whence);
__attribute__((weak)) off_t lseek(s32 fd, off_t offset, s32 whence);

#ifdef __cplusplus
}
#endif
/*
 * lseek --  Since a serial port is non-seekable, we return an error.
 */
/**
 * @brief Stub for lseek(); always returns -1 (serial port is non-seekable).
 * @param[in] fd File descriptor (unused).
 * @param[in] offset Seek offset (unused).
 * @param[in] whence Seek origin (unused).
 * @return Always (off_t)-1.
 */
__attribute__((weak)) off_t lseek(s32 fd, off_t offset, s32 whence)
{
  (void)fd;
  (void)offset;
  (void)whence;
  errno = ESPIPE;
  return ((off_t)-1);
}

/**
 * @brief Stub for _lseek(); always returns -1 (serial port is non-seekable).
 * @param[in] fd File descriptor (unused).
 * @param[in] offset Seek offset (unused).
 * @param[in] whence Seek origin (unused).
 * @return Always (off_t)-1.
 */
__attribute__((weak)) off_t _lseek(s32 fd, off_t offset, s32 whence)
{
  (void)fd;
  (void)offset;
  (void)whence;
  errno = ESPIPE;
  return ((off_t)-1);
}
