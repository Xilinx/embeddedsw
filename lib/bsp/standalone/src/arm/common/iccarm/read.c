/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file read.c
 * @brief File read stub implementation for standalone BSP.
 */

/*
 *
 * The "__read" function reads a number of bytes, at most "size" into
 * the memory area pointed to by "buffer".  It returns the number of
 * bytes read, 0 at the end of the file, or _LLIO_ERROR if failure
 * occurs.
 *
 * The template implementation below should return a
 * character value, or -1 on failure.
 *
 */

#include <yfuns.h>
#include "xparameters.h"
#include "xil_types.h"


/**
 * @brief Read a character from console input.
 * @return Character received from the console.
 */
extern char inbyte(void);

size_t __read(sint32 handle,unsigned char * buffer, size_t size);

/**
 * @brief IAR __read syscall; reads from STDIN if configured.
 * @param[in] handle File handle (unused; reads from STDIN).
 * @param[out] buffer Buffer to store the bytes read.
 * @param[in] size Maximum number of bytes to read.
 * @return Number of bytes read, or 0 on error.
 */
size_t __read(sint32 handle,unsigned char * buffer, size_t size)
{

#ifdef STDIN_BASEADDRESS
  s32 i;
  s32 numbytes = 0;
  unsigned char *LocalBuf = buffer;

  (void)handle;
  if(LocalBuf != NULL) {
	for (i = 0; i < size; i++) {
		numbytes++;
		*(LocalBuf + i) = inbyte();
		if ((*(LocalBuf + i) == '\n' )|| (*(LocalBuf + i) == '\r')) {
			break;
		}
	}
  }

  return numbytes;
#else
  (void)handle;
  (void)buffer;
  (void)size;
  return 0;
#endif
}
