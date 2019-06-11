/******************************************************************************/
/**
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
* @file xil_util.c
*
* This file contains xil utility functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.4   mmd      04/21/19 First release.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/
#define MAX_NIBBLES			8U

/************************** Function Prototypes *****************************/
/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param	InChar is input character. It has to be between 0-9,a-f,A-F
 * @param	Num is the output nibble.
 *
 * @return
 *		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *
 * @note	None.
 *
 *****************************************************************************/

u32 Xil_ConvertCharToNibble(u8 InChar, u8 *Num)
{
	u32 Status;

	/* Convert the char to nibble */
	if ((InChar >= (u8)'0') && (InChar <= (u8)'9')) {
		*Num = InChar - (u8)'0';
		Status = XST_SUCCESS;
	}
	else if ((InChar >= (u8)'a') && (InChar <= (u8)'f')) {
		*Num = InChar - (u8)'a' + 10U;
		Status = XST_SUCCESS;
	}
	else if ((InChar >= (u8)'A') && (InChar <= (u8)'F')) {
		*Num = InChar - (u8)'A' + 10U;
		Status = XST_SUCCESS;
	}
	else {
		Status = XST_FAILURE;
	}

	return Status;
}

/****************************************************************************/
/*
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper
 *		case values. Value should be between 0-9, a-f and A-F
 *
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 *		- XST_SUCCESS no errors occured.
 *		- ERROR when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 * @note	None.
 *
 *****************************************************************************/
u32 Xil_ConvertStringToHex(const char *Str, u32 *buf, u8 Len)
{
	u32 Status = XST_FAILURE;
	u8 ConvertedLen = 0U, index = 0U;
	u8 Nibble[MAX_NIBBLES] = {0U};
	u8 i;

	while (ConvertedLen < Len) {
		/* Convert char to nibble */
		for (i = 0U; i < MAX_NIBBLES; i++) {
			Status = Xil_ConvertCharToNibble(
					Str[ConvertedLen], &Nibble[i]);
			ConvertedLen = ConvertedLen +1U;
			if (Status != XST_SUCCESS) {
				/* Error converting char to nibble */
				goto END;
			}
		}

		buf[index] = ((Nibble[0] << (u8)28U )| (Nibble[1] << (u8)24U) |
				(Nibble[2] << (u8)20U) | (Nibble[3] << (u8)16U) |
				(Nibble[4] << (u8)12U) | (Nibble[5] << (u8)8U)|
				(Nibble[6] << (u8)4U) | (u32)Nibble[7]);
		index++;
	}
END:
	return Status;
}

/****************************************************************************/
/*
 * Waits for the event
 *
 * @param	RegAddr    Address of register to be checked for event(s) occurance
 * @param	EventMask  Mask indicating event(s) to be checked
 * @param	Event      Specific event(s) value to be checked
 * @param	Timeout    Free counter decremented on each event(s) check and
 *       	           declared timeout when reaches 0
 *
 * @return
 *		- XST_SUCCESS  On occurance of the event(s).
 *		- XST_FAILURE  Event did not occur before counter reaches 0
 *
 * @note	None.
 *
 *****************************************************************************/
u32 Xil_WaitForEvent(u32 RegAddr, u32 EventMask, u32 Event, u32 Timeout)
{
	u32 RegVal;
	u32 PollCount = Timeout;
	u32 Status = XST_FAILURE;

	while(PollCount > 0) {
		RegVal = Xil_In32(RegAddr) & EventMask;
		if (RegVal == Event) {
			Status = XST_SUCCESS;
			break;
		}
		PollCount--;
	}

	return Status;
}
