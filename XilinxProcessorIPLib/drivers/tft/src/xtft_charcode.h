/******************************************************************************
*
* Copyright (C) 2008 - 2015 Xilinx, Inc.  All rights reserved.
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
/***************************************************************************/
/**
*
* @file xtft_charcode.h
* @addtogroup tft_v6_1
* @{
*
* This file contains the macro and variable definitions of character
* dimensions and character array of bitmaps of ASCII characters.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver    Who   Date      Changes
* -----  ----  --------  -----------------------------------------------
* 1.00a  sg    03/24/08  First release
*</pre>
*
****************************************************************************/
#ifndef XTFT_CHARCODE_H /* prevent circular inclusions */
#define XTFT_CHARCODE_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *******************************/
#include "xil_types.h"
#include "xil_assert.h"

/************************** Constant Definitions ***************************/

/**************************** Type Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *******************/

/**
 *
 * Compression method for the character bitmaps.
 *
 * @param	a7 is bit value at 8th position.
 * @param	a6 is bit value at 7th position.
 * @param	a5 is bit value at 6th position.
 * @param	a4 is bit value at 5th position.
 * @param	a3 is bit value at 4th position.
 * @param	a2 is bit value at 3rd position.
 * @param	a1 is bit value at 2nd position.
 * @param	a0 is bit value at 1st position.
 *
 * @return	Bit Value - either 0 or 1.
 *
 * @note	C-style signature:
 *		int GenPixels(int a7, int a6, int a5, int a4, int a3, int a2,
 * 				int a1, int a0).
 */
#define GenPixels(a7, a6, a5, a4, a3, a2, a1, a0) (   ((a7) << 7) | 	\
		((a6) << 6) | ((a5) << 5) | ((a4) << 4) | ((a3) << 3) | \
		((a2) << 2) | ((a1) << 1) | (a0)   )

/************************** Function Prototypes ***************************/

/************************** Variable Definitions ***************************/

/**
 * Character array of 96 characters with each character having dimensions
 * of width 8 and height 12 pixels. It starts with space as first character.
 */
u8 XTft_VidChars[96][12] =
	{
	/* ASCII 0x20 0d032 ' ' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x21 0d033 '!' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x22 0d034 '"' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x23 0d035 '#' */
		{
		/* Line  0 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x24 0d036 '$' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x25 0d037 '%' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 1, 0, 1, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 1, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x26 0d038 '&' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 1, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 1, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 1, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 1, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x27 0d039 ''' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x28 0d040 '(' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x29 0d041 ')' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2a 0d042 '*' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 1, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 1, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2b 0d043 '+' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2c 0d044 ',' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2d 0d045 '-' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2e 0d046 '.' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x2f 0d047 '/' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x30 0d048 '0' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 1, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 1, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 1, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 1, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x31 0d049 '1' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 1, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x32 0d050 '2' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x33 0d051 '3' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x34 0d052 '4' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 1, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 1, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x35 0d053 '5' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 1, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 1, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x36 0d054 '6' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 1, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x37 0d055 '7' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x38 0d056 '8' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x39 0d057 '9' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3a 0d058 ':' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3b 0d059 ';' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3c 0d060 '<' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3d 0d061 '=' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3e 0d062 '>' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x3f 0d063 '?' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x40 0d064 '@' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 1, 0, 1, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 1, 0, 1, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 1, 1, 1, 1, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x41 0d065 'A' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x42 0d066 'B' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x43 0d067 'C' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x44 0d068 'D' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x45 0d069 'E' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x46 0d070 'F' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x47 0d071 'G' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x48 0d072 'H' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x49 0d073 'I' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4a 0d074 'J' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4b 0d075 'K' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 1, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4c 0d076 'L' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4d 0d077 'M' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 1, 0, 0, 0, 1, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 1, 0, 1, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4e 0d078 'N' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 1, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 1, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 1, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 1, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 1, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 1, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x4f 0d079 'O' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x50 0d080 'P' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x51 0d081 'Q' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 1, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x52 0d082 'R' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x53 0d083 'S' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x54 0d084 'T' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x55 0d085 'U' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x56 0d086 'V' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x57 0d087 'W' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 1, 0, 1, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 1, 0, 0, 0, 1, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x58 0d088 'X' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x59 0d089 'Y' */
		{
		/* Line  0 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  1 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5a 0d090 'Z' */
		{
		/* Line  0 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5b 0d091 '[' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5c 0d092 '\' */
		{
		/* Line  0 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5d 0d093 ']' */
		{
		/* Line  0 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5e 0d094 '^' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x5f 0d095 '_' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 1, 1, 1, 1, 1, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x60 0d096 '`' */
		{
		/* Line  0 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x61 0d097 'a' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 1, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x62 0d098 'b' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x63 0d099 'c' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x64 0d100 'd' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 1, 1, 1, 1, 1, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x65 0d101 'e' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 1, 1, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x66 0d102 'f' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 1, 0, 0, 1, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 1, 1, 1, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x67 0d103 'g' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 1, 1, 1, 1, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x68 0d104 'h' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x69 0d105 'i' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6a 0d106 'j' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6b 0d107 'k' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6c 0d108 'l' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6d 0d109 'm' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 1, 0, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6e 0d110 'n' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x6f 0d111 'o' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x70 0d112 'p' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 1, 1, 1, 1, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 1, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x71 0d113 'q' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x72 0d114 'r' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 1, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x73 0d115 's' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x74 0d116 't' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 1, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x75 0d117 'u' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 1, 1, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x76 0d118 'v' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x77 0d119 'w' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  6 */  GenPixels( 1, 0, 0, 0, 0, 0, 1, 0),
		/* Line  7 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 0, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x78 0d120 'x' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 1, 0, 1, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 0, 0, 0, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x79 0d121 'y' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 1, 0, 0, 0, 0, 1, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 1, 1, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 1, 1, 0, 1, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 1, 0),
		/* Line  9 */  GenPixels( 0, 0, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7a 0d122 'z' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 1, 1, 1, 1, 1, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7b 0d123 '{' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 1, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 1, 1, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7c 0d124 '|' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7d 0d125 '}' */
		{
		/* Line  0 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 1, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 1, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 1, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 1, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7e 0d126 '~' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 1, 1, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 1, 0, 0, 1, 0, 0, 1, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 1, 1, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		},
	/* ASCII 0x7f 0d127 ' ' */
		{
		/* Line  0 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  1 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  2 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  3 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  4 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  5 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  6 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  7 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  8 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line  9 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 10 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0),
		/* Line 11 */  GenPixels( 0, 0, 0, 0, 0, 0, 0, 0)
		}
	};

/************************** Function Definitions ***************************/

#ifdef __cplusplus
}
#endif

#endif /* XTFT_CHARCODE_H */
/** @} */
