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
 *
 * This is a template implementation of the "__lseek" function used by
 * the standard library.  Replace it with a system-specific
 * implementation.
 *
 * The "__lseek" function makes the next file operation (__read or
 * __write) act on a new location.  The parameter "whence" specifies
 * how the "offset" parameter should be interpreted according to the
 * following table:
 *
 *  0 (=SEEK_SET) - Goto location "offset".
 *  1 (=SEEK_CUR) - Go "offset" bytes from the current location.
 *  2 (=SEEK_END) - Go to "offset" bytes from the end.
 *
 * This function should return the current file position, or -1 on
 * failure.
 */

#include <stdio.h>
#include <yfuns.h>
#include "xil_types.h"
LONG __lseek(sint32 handle, LONG offset, sint32 whence);

LONG __lseek(sint32 handle, LONG offset, sint32 whence)
{
  return (-1);
}
