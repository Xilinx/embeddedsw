/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xdebug.h
*
* This file contains functions to configure Video Pattern Generator core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  KI    07/13/17 Initial release.
* </pre>
*
******************************************************************************/

#ifndef XDEBUG  /* prevent circular inclusions */
#define XDEBUG  /* by using protection macros */

//#define DEBUG 1

#if defined(DEBUG) && !defined(NDEBUG)

#ifndef XDEBUG_WARNING
#define XDEBUG_WARNING
//#warning DEBUG is enabled
#endif

int printf(const char *format, ...);

#define XDBG_DEBUG_ERROR             0x00000001U /* error  condition messages */
#define XDBG_DEBUG_GENERAL           0x00000002U /* general debug  messages */
#define XDBG_DEBUG_ALL               0xFFFFFFFFU /* all debugging data */

#define xdbg_current_types (XDBG_DEBUG_GENERAL)

#define xdbg_stmnt(x)  x

#define xdbg_printf(type, ...) \
			(((type) & xdbg_current_types) ? printf (__VA_ARGS__) : 0)


#else /* defined(DEBUG) && !defined(NDEBUG) */

#define xdbg_stmnt(x)

#define xdbg_printf(...)

#endif /* defined(DEBUG) && !defined(NDEBUG) */

#endif /* XDEBUG */
