/******************************************************************************
* Copyright (c) 2009 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file read.c
 * @brief File read stub implementation for standalone BSP.
 */


/* read.c -- read bytes from a input device.
 */
#ifndef UNDEFINE_FILE_OPS
#include "xil_printf.h"
#include "bspconfig.h"
#ifndef SDT
#include "xparameters.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((weak)) s32 _read (s32 fd, char8* buf, s32 nbytes);
__attribute__((weak)) s32 read (s32 fd, char8* buf, s32 nbytes);

#ifdef __cplusplus
}
#endif

/*
 * read  -- read bytes from the serial port. Ignore fd, since
 *          we only have stdin.
 */
/**
 * @brief Stub for read(); reads from STDIN if configured, otherwise returns 0.
 * @param[in] fd File descriptor (unused; reads from STDIN).
 * @param[out] buf Buffer to store the bytes read.
 * @param[in] nbytes Maximum number of bytes to read.
 * @return Number of bytes read.
 */
__attribute__((weak)) s32
read (s32 fd, char8* buf, s32 nbytes)
{
#if defined (STDIN_BASEADDRESS) || defined(SDT)
  s32 i;
  s32 numbytes = 0;
  char8* LocalBuf = buf;

  (void)fd;
  if(LocalBuf != NULL) {
	for (i = 0; i < nbytes; i++) {
		numbytes++;
		LocalBuf[i] = inbyte();
		if ((LocalBuf[i] == '\n' )|| (LocalBuf[i] == '\r')) {
			break;
		}
	}
  }

  return numbytes;
#else
  (void)fd;
  (void)buf;
  (void)nbytes;
  return 0;
#endif
}

/**
 * @brief Stub for _read(); reads from STDIN if configured, otherwise returns 0.
 * @param[in] fd File descriptor (unused; reads from STDIN).
 * @param[out] buf Buffer to store the bytes read.
 * @param[in] nbytes Maximum number of bytes to read.
 * @return Number of bytes read.
 */
__attribute__((weak)) s32
_read (s32 fd, char8* buf, s32 nbytes)
{
#if defined(STDIN_BASEADDRESS) || defined(SDT)
  s32 i;
  s32 numbytes = 0;
  char8* LocalBuf = buf;

  (void)fd;
  if(LocalBuf != NULL) {
	for (i = 0; i < nbytes; i++) {
		numbytes++;
		LocalBuf[i] = inbyte();
		if ((LocalBuf[i] == '\n' )|| (LocalBuf[i] == '\r')) {
			break;
		}
	}
  }

  return numbytes;
#else
  (void)fd;
  (void)buf;
  (void)nbytes;
  return 0;
#endif
}
#endif
