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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
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

/****************************** Include Files *********************************/
#include "xil_util.h"

/************************** Constant Definitions ****************************/
#define MAX_NIBBLES			8U

/************************** Function Prototypes *****************************/
/****************************************************************************/
/**
 * Converts the char into the equivalent nibble.
 *	Ex: 'a' -> 0xa, 'A' -> 0xa, '9'->0x9
 *
 * @param   InChar - Input character to be converted to nibble.
 *                   Valid characters are between 0-9, a-f, A-F
 * @param   Num    - Memory location where nibble is to be stored
 *
 * @return
 *          XST_SUCCESS - Character converted to nibble
 *          XST_FAILURE - Invalid input character
 *
 * @note    None.
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
 * @param   Str - Pointer to string to be converted to Hex.
 *                Accepted characters in string are between 0-9, a-f and A-F
 * @param   Buf - Pointer to memory location where converted hex values are to
 *                be stored.
 * @param   Len - Length of input string
 *
 * @return
 *          XST_SUCCESS - Input string is converted to hex
 *          XST_FAILURE - Invalid character in inpit string
 *
 * @note    None.
 *
 *****************************************************************************/
u32 Xil_ConvertStringToHex(const char *Str, u32 *buf, u8 Len)
{
	u32 Status = XST_FAILURE;
	u8 ConvertedLen = 0U, index = 0U;
	u8 Nibble[MAX_NIBBLES] = {0U};
	u8 i;

	while (ConvertedLen < Len) {
		for (i = 0U; i < MAX_NIBBLES; i++) {
			Status = Xil_ConvertCharToNibble(Str[ConvertedLen],
			                                &Nibble[i]);
			ConvertedLen = ConvertedLen +1U;
			if (Status != XST_SUCCESS) {
				/* Error converting char to nibble */
				goto END;
			}
		}

		buf[index] = ((Nibble[0] << (u8)28U) | (Nibble[1] << (u8)24U) |
		              (Nibble[2] << (u8)20U) | (Nibble[3] << (u8)16U) |
		              (Nibble[4] << (u8)12U) | (Nibble[5] << (u8)8U)  |
		              (Nibble[6] << (u8)4U)  | (u32)Nibble[7]);
		index++;
	}
END:
	return Status;
}

/****************************************************************************/
/*
 * Waits for the event
 *
 * @param   RegAddr   - Address of register to be checked for event(s) occurance
 * @param   EventMask - Mask indicating event(s) to be checked
 * @param   Event     - Specific event(s) value to be checked
 * @param   Timeout   - Free counter decremented on each event(s) check and
 *                      declared timeout when reaches 0
 *
 * @return
 *          XST_SUCCESS - On occurance of the event(s).
 *          XST_FAILURE - Event did not occur before counter reaches 0
 *
 * @note    None.
 *
 *****************************************************************************/
u32 Xil_WaitForEvent(u32 RegAddr, u32 EventMask, u32 Event, u32 Timeout)
{
	u32 EventStatus;
	u32 PollCount = Timeout;
	u32 Status = XST_FAILURE;

	while(PollCount > 0) {
		EventStatus = Xil_In32(RegAddr) & EventMask;
		if (EventStatus == Event) {
			Status = XST_SUCCESS;
			break;
		}
		PollCount--;
	}

	return Status;
}


/******************************************************************************/
/**
 * Waits for the events. Returns on occurrence of first event / timeout.
 *
 * @param   RegAddr    - Address of register to be checked for event(s)
 *                       occurrence
 * @param   EventMask  - Mask indicating event(s) to be checked
 * @param   WaitEvents - Specific event(s) to be checked
 * @param   Timeout    - Free counter decremented on each event(s) check and
 *                       declared timeout when reaches 0
 * @param   Events     - Mask of Events occured returned in memory pointed by
 *                       this variable
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
 *          XST_FAILURE - Event did not occur before counter reaches 0
 *
 * @note    None.
 *
 ******************************************************************************/
u32 Xil_WaitForEvents(u32 EventsRegAddr, u32 EventsMask, u32 WaitEvents,
			 u32 Timeout, u32* Events)
{
	u32 EventStatus;
	u32 PollCount = Timeout;
	u32 Status = XST_TIMEOUT;

	*Events = 0x00;
	do {
		EventStatus = Xil_In32(EventsRegAddr);
		EventStatus &= EventsMask;
		if(EventStatus & WaitEvents) {
			Status = XST_SUCCESS;
			*Events = EventStatus;
			break;
		}
		PollCount--;
	}
	while(PollCount > 0);

	return Status;
}

/******************************************************************************/
/**
 * Checks whether the passed character is a valid hex digit
 *
 * @param   Ch - Input Character
 *
 * @return
 *          XST_SUCCESS	- on valid hex digit
 *          XST_FAILURE - on invalid hex digit
 *
 * @note    None.
 *
 ******************************************************************************/
u32 Xil_IsValidHexChar(const char Ch)
{
	char ValidChars[] = "0123456789abcdefABCDEF";
	char *RetVal;
	u32 Status = XST_FAILURE;

	RetVal = strchr(ValidChars, (int)Ch);
	if (RetVal != NULL) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/******************************************************************************/
/**
 * Validate the input string contains only hexadecimal characters
 *
 * @param   HexStr - Pointer to string to be validated
 *
 * @return
 *          XST_SUCCESS	- On valid input hex string
 *          XST_INVALID_PARAM - On invalid length of the input string
 *          XST_FAILURE	- On non hexadecimal character in string
 *
 * @note    None
 *
 ******************************************************************************/
u32 Xil_ValidateHexStr(const char *HexStr)
{
	u32 Idx;
	u32 Len;
	u32 Status = XST_INVALID_PARAM;

	if(NULL == HexStr) {
		goto END;
	}

	Len = Xil_Strnlen(HexStr, XIL_MAX_HEX_STR_LEN + 1U);
	if (Len > XIL_MAX_HEX_STR_LEN) {
		goto END;
	}

	for (Idx = 0U; Idx < Len; Idx++) {
		Status = Xil_IsValidHexChar(HexStr[Idx]);
		if (Status != XST_SUCCESS) {
			break;
		}
	}

END:
	return Status;
}

/******************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0x23, 0xc1, 0xab}
 *
 * @param   Str - Input String to be converted to hex number in little
 *                endian format. Valid characters of input strin are between
 *                0-9, a-f and A-F
 * @param   Buf - Pointer to memory location where converted hex numbers are to
 *                be stored.
 * @param   Len - Expected number of output bits
 *
 * @return
 *          XST_SUCCESS - Input string is converted to hex number(s)
 *          XST_FAILURE - Invalid input character detected in input string
 *
 * @note
 *
 ******************************************************************************/
u32 Xil_ConvertStringToHexLE(const char *Str, u8 *Buf, u32 Len)
{
	u32 ConvertedLen;
	u8 LowerNibble = 0U;
	u8 UpperNibble = 0U;
	u32 StrIndex;
	u32 Status = XST_FAILURE;

	if ((NULL == Str) || (NULL == Buf)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((Len == 0U) || ((Len % XIL_SIZE_OF_BYTE_IN_BITS) != 0U)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if(Len != (strlen(Str) * XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	StrIndex = (Len / XIL_SIZE_OF_BYTE_IN_BITS) - 1U;
	ConvertedLen = 0U;
	while (ConvertedLen < (Len / XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		Status = Xil_ConvertCharToNibble(Str[ConvertedLen],
		                                &UpperNibble);
		if (XST_SUCCESS == Status) {
			Status = Xil_ConvertCharToNibble(Str[ConvertedLen + 1],
			                                &LowerNibble);
			if (XST_SUCCESS == Status) {
				Buf[StrIndex] =
				   (UpperNibble << XIL_SIZE_OF_NIBBLE_IN_BITS) |
				   LowerNibble;
				StrIndex = StrIndex - 1U;
			}
			else {
				Status = XST_INVALID_PARAM;
				goto END;
			}
		}
		else {
			Status = XST_INVALID_PARAM;
			goto END;
		}
		ConvertedLen += 2U;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/******************************************************************************/
/**
 * Returns the length of input string.
 *
 * @param   Str - Input string
 * @param   MaxLen - Maximum expected length of the input string
 *
 * @return
 *          Returns length of the input string if length is less than MaxLen.
 *          Returns MaxLen if the length of the input string is >= MaxLen.
 *
 * @note
 *
 ******************************************************************************/
u32 Xil_Strnlen(const char *Str, u32 MaxLen)
{
	const char *InStr = Str;
	u32 StrLen = 0U;

	if (NULL == Str) {
		goto END;
	}

	while(StrLen < MaxLen) {
		if ('\0' == *InStr) {
			break;
		}
		StrLen++;
		InStr++;
	}

END:
	return StrLen;
}
