/******************************************************************************
* Copyright (c) 2009 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file write.c
 * @brief File write stub implementation for standalone BSP.
 */


/*
 * The "__write" function should output "size" number of bytes from
 * "buffer" in some application-specific way.  It should return the
 * number of characters written, or _LLIO_ERROR on failure.
 *
 * If "buffer" is zero then __write should perform flushing of
 * internal buffers, if any.  In this case "handle" can be -1 to
 * indicate that all handles should be flushed.
 *
 * The template implementation below assumes that the application
 * provides the function "MyLowLevelPutchar".  It should return the
 * character written, or -1 on failure.
 *
 */

#include <yfuns.h>
#include "xil_types.h"


#if 0
/*
 * If the __write implementation uses internal buffering, uncomment
 * the following line to ensure that we are called with "buffer" as 0
 * (that is flush) when the application terminates.
 */

/** @brief IAR __write syscall; outputs to STDOUT if configured. */
size_t __write(sint32 handle, const u8 * buffer, size_t size)
{
  u32 volatile *uart_base = (u32 *)0xE0001000U;
  s32 i;

  for (i =0; i < size;i++) {
    /* wait if TNFUL */
    while (*(uart_base + 11U) & (1U << 14U)) ;
    *(uart_base + 12U) = buffer[i];
  }
  return 0;
}

#endif

#include "xparameters.h"
#include "xil_printf.h"

#ifdef __cplusplus
extern "C" {
	sint32 _write (sint32 fd, char8* buf, sint32 nbytes);
}
#endif

/*
 * write -- write bytes to the serial port. Ignore fd, since
 *          stdout and stderr are the same. Since we have no filesystem,
 *          open will only return an error.
 */
/**
 * @brief Stub for write(); outputs to STDOUT if configured.
 * @param[in] fd File descriptor (unused; writes to STDOUT).
 * @param[in] buf Buffer of bytes to write.
 * @param[in] nbytes Number of bytes to write.
 * @return Number of bytes written.
 */
sint32
write (sint32 fd, char8* buf, sint32 nbytes)

{
#ifdef STDOUT_BASEADDRESS
  s32 i;

  (void)fd;
  for (i = 0; i < nbytes; i++) {
    if (*(buf + i) == '\n') {
      outbyte ('\r');
    }
    outbyte (*(buf + i));
  }
  return (nbytes);
#else
  (void)fd;
  (void)buf;
  (void)nbytes;
  return 0;
#endif
}

/**
 * @brief Alternate __write entry point; outputs to STDOUT if configured.
 * @param[in] fd File descriptor (unused; writes to STDOUT).
 * @param[in] buf Buffer of bytes to write.
 * @param[in] nbytes Number of bytes to write.
 * @return Number of bytes written.
 */
size_t
__write (sint32 fd, const u8* buf, size_t nbytes)
{
#ifdef STDOUT_BASEADDRESS
  s32 i;

  (void)fd;
  for (i = 0; i < nbytes; i++) {
    if (*(buf + i) == '\n') {
      outbyte ('\r');
    }
    outbyte (*(buf + i));
  }
  return (nbytes);
#else
  (void)fd;
  (void)buf;
  (void)nbytes;
  return 0;
#endif
}
