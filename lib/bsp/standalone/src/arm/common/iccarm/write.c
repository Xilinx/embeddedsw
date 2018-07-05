/******************************************************************************
*
* Copyright (C) 2009 - 2015 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

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
 * (i.e. flush) when the application terminates.
 */

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
