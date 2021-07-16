/******************************************************************************/
/**
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* 7.2   nava     08/01/20 Updated Xil_WaitForEvent() and Xil_WaitForEvents(()
*                         API to use microsecond timeout instead of a free
*                         counter.
* 7.3   kal      06/30/20 Converted Xil_Ceil macro to API.
*       rpo      08/19/20 Added function for read,modify,write
*       kal      09/22/20 Changed the param type from const char to const char*
*                         to avoid copying key onto stack
*       td       10/16/20 Added Xil_Strcpy, Xil_Strcat, Xil_SecureMemCpy and
*                         Xil_MemCmp functions
* 7.4   am       11/26/20 Added Xil_StrCpyRange function
* 7.6   kpt      07/15/21 Added Xil_SecureZeroize function
*
* </pre>
*
*****************************************************************************/

/****************************** Include Files *********************************/
#include "xil_util.h"
#include "sleep.h"

/************************** Constant Definitions ****************************/
#define MAX_NIBBLES			8U

/************************** Function Prototypes *****************************/

/******************************************************************************/
/**
* This API ceils the provided float value.
*
* @param	Value is a float variable which has to ceiled to nearest
*		integer.
*
* @return	Returns ceiled value.
*
*******************************************************************************/
int Xil_Ceil(float Value)
{
    int Result = Value;

	if (Value > Result) {
        Result = Result + 1;
    }

    return Result;
}

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
 * @param   RegAddr   - Address of register to be checked for event(s) occurrence
 * @param   EventMask - Mask indicating event(s) to be checked
 * @param   Event     - Specific event(s) value to be checked
 * @param   Timeout   - Max number of microseconds to wait for an event(s).
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
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
		usleep(1U);
	}

	return Status;
}


/******************************************************************************/
/**
 * Waits for the events. Returns on occurrence of first event / timeout.
 *
 * @param   EventsRegAddr - Address of register to be checked for event(s)
 *                          occurrence
 * @param   EventsMask - Mask indicating event(s) to be checked
 * @param   WaitEvents - Specific event(s) to be checked
 * @param   Timeout    - Max number of microseconds to wait for an event(s).
 * @param   Events     - Mask of Events occurred returned in memory pointed by
 *                       this variable
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
 *          XST_FAILURE - Event did not occur before counter reaches 0
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
		usleep(1U);
	}
	while(PollCount > 0);

	return Status;
}

/******************************************************************************/
/**
 * Checks whether the passed character is a valid hex digit
 *
 * @param   Ch - Pointer to the input character
 *
 * @return
 *          XST_SUCCESS	- on valid hex digit
 *          XST_FAILURE - on invalid hex digit
 *
 * @note    None.
 *
 ******************************************************************************/
u32 Xil_IsValidHexChar(const char *Ch)
{
	u32 Status = XST_FAILURE;

	if(NULL == Ch) {
		goto END;
	}
	if ((*Ch >= '0' && *Ch <='9')||
		(*Ch >= 'a' && *Ch <='f')||
		(*Ch >= 'A' && *Ch <='F')) {

		Status = XST_SUCCESS;
	}
END:
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
		Status = Xil_IsValidHexChar(&HexStr[Idx]);
		if (Status != XST_SUCCESS) {
			break;
		}
	}

END:
	return Status;
}

/****************************************************************************/
/**
 * Converts the string into the equivalent Hex buffer.
 *	Ex: "abc123" -> {0xab, 0xc1, 0x23}
 *
 * @param	Str is a Input String. Will support the lower and upper case values.
 * 		Value should be between 0-9, a-f and A-F
 * @param	Buf is Output buffer.
 * @param	Len of the input string. Should have even values
 *
 * @return
 * 		- XST_SUCCESS no errors occurred.
 *		- XST_FAILURE an error when input parameters are not valid
 *		- an error when input buffer has invalid values
 *
 *	TDD Test Cases:
 *	---Initialization---
 *	Len is odd
 *	Len is zero
 *	Str is NULL
 *	Buf is NULL
 *	---Functionality---
 *	Str input with only numbers
 *	Str input with All values in A-F
 *	Str input with All values in a-f
 *	Str input with values in a-f, 0-9, A-F
 *	Str input with values in a-z, 0-9, A-Z
 *	Boundary Cases
 *	Memory Bounds of buffer checking
 * ****************************************************************************/
u32 Xil_ConvertStringToHexBE(const char *Str, u8 *Buf, u32 Len)
{
	u32 ConvertedLen;
	u8 LowerNibble = 0U;
	u8 UpperNibble = 0U;
	u32 Status = (u32)XST_FAILURE;

	if ((Str == NULL) || (Buf == NULL)) {
		Status = (u32)XST_INVALID_PARAM;
		goto END;
	}

	if ((Len == 0U) || ((Len % XIL_SIZE_OF_BYTE_IN_BITS) != 0U)) {
		Status = (u32)XST_INVALID_PARAM;
		goto END;
	}

	if(Len != (strlen(Str) * XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		Status = (u32)XST_INVALID_PARAM;
		goto END;
	}

	ConvertedLen = 0U;
	while (ConvertedLen < (Len / XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		if (Xil_ConvertCharToNibble(Str[ConvertedLen],&UpperNibble)
				== (u32)XST_SUCCESS) {
			if (Xil_ConvertCharToNibble(Str[ConvertedLen+1],
					&LowerNibble) == (u32)XST_SUCCESS) {
				Buf[ConvertedLen/2] =
				(UpperNibble << XIL_SIZE_OF_NIBBLE_IN_BITS) |
								LowerNibble;
			}
			else {
				Status = (u32)XST_INVALID_PARAM;
				goto END;
			}
		}
		else {
			Status = (u32)XST_INVALID_PARAM;
			goto END;
		}
		ConvertedLen += 2U;
	}
	Status = (u32)XST_SUCCESS;
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

/*****************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to an address.
 *
 * @param	Addr denotes Address
 * @param	Mask denotes the bits to be modified
 * @param	Value is the value to be written to the address
 *
 * @return	None
 *
 *****************************************************************************/
void Xil_UtilRMW32(u32 Addr, u32 Mask, u32 Value)
{
	u32 Val;

	Val = Xil_In32(Addr);
	Val = (Val & (~Mask)) | (Mask & Value);
	Xil_Out32(Addr, Val);
}

/*****************************************************************************/
/**
 * @brief	This functions copies source string to destination string. This
 * 			function is a safe version of strcpy
 *
 * @param	DestPtr is pointer to destination string
 * @param	SrcPtr is pointer to source string
 * @param	Size is the maximum number of bytes of the source string
 *			to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int Xil_Strcpy(char *DestPtr, const char *SrcPtr, const u32 Size)
{
	int Status = XST_FAILURE;
	u32 Count;

	if ((SrcPtr == NULL) || (DestPtr == NULL) || (Size == 0U)) {
		goto END;
	}

	for (Count = 0U; (SrcPtr[Count] != '\0') && (Count < Size); ++Count) {
		DestPtr[Count] = SrcPtr[Count];
	}
	if (Count == Size) {
		DestPtr[0U] = '\0';
		goto END;
	}
	DestPtr[Count] = '\0';
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Copies specified range from source string to destination string
 *
 * @param	Src  is a pointer to source string
 * @param	Dst  is a pointer to destination string
 * @param	From is 0 based index from where string copy starts
 * @param	To   is 0 based index till which string is copied
 * @param	MaxSrcLen is the maximum length of source string
 * @param	MaxDstLen is the maximum length of destination string
 *
 * @return	XST_SUCCESS on success
 * 		XST_FAILURE on failure
 *
 * @note	None
 *
 ****************************************************************************/
int Xil_StrCpyRange(const u8 *Src, u8 *Dst, u32 From, u32 To, u32 MaxSrcLen,
	u32 MaxDstLen)
{
	int Status = XST_FAILURE;
	u32 SrcLength;
	u32 Index;

	if ((Src == NULL) || (Dst == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((To >= MaxSrcLen) || (To < From)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	if ((To - From + 1U) >= MaxDstLen) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	SrcLength = Xil_Strnlen((const char *)Src, MaxSrcLen);
	if (To >= SrcLength) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	for (Index = From; Index <= To && Src[Index]!= '\0'; Index++) {
		Dst[Index - From] = Src[Index];
	}

	Dst[Index - From] = '\0';
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function appends string2 to string1. This function is a safe
 * 			version of strcat
 *
 * @param	Str1Ptr is pointer to string1
 * @param	Str2Ptr is pointer to string2
 * @param	Size is the maximum number of bytes Str1 can hold
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int Xil_Strcat(char* Str1Ptr, const char* Str2Ptr, const u32 Size)
{
	int Status = XST_FAILURE;
	u32 Count = 0U;
	u32 CountTmp = 0U;

	if ((Str1Ptr == NULL) || (Str2Ptr == NULL) || (Size == 0U)) {
		goto END;
	}

	while ((Count < Size) && (Str1Ptr[Count] != '\0')) {
		Count++;
	}

	while ((Str2Ptr[CountTmp] != '\0') && (Count < Size)) {
		Str1Ptr[Count++] = Str2Ptr[CountTmp++];
	}
	if (Count == Size) {
		Str1Ptr[0U] = '\0';
		goto END;
	}
	Str1Ptr[Count] = '\0';
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies Len bytes from source memory to destination
 *			memory. If Len is greater than DestPtrLen, then DestPtr is also
 *			filled with 0s till DestPtrLen bytes and is considered as a failure.
 *			This function is a secure implementation of memcpy
 *
 * @param	DestPtr is pointer to destination address
 * @param	DestPtrLen is the memory alloted to the destination buffer
 * @param	SrcPtr is pointer to source address
 * @param	Len is number of bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int Xil_SecureMemCpy(void * DestPtr, u32 DestPtrLen, const void * SrcPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u8 *Dest = (u8 *)DestPtr;
	const u8 *Src = (const u8 *)SrcPtr;

	if ((DestPtr == NULL) || (SrcPtr == NULL)) {
		goto END;
	}

	if (Len > DestPtrLen) {
		while (DestPtrLen != 0U) {
			*Dest = 0U;
			Dest++;
			DestPtrLen--;
		}
		goto END;
	}

	/* Loop and copy.  */
	while (Len != 0U) {
		*Dest = *Src;
		Dest++;
		Src++;
		Len--;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function compares Len bytes from memory1 and memory2. This
 * 			function is a secure implementation of memcmp
 *
 * @param	Buf1Ptr is pointer to memory1
 * @param	Buf2Ptr is pointer to memory2
 * @param	Len is number of byets to be compared
 *
 * @return	0 if contents of both the memory regions are same,
 * 			-1 if first non-matching character has lower value in Buf1Ptr
 * 			1 if first non-matching character is greater value in Buf1Ptr
 *
 ******************************************************************************/
int Xil_MemCmp(const void * Buf1Ptr, const void * Buf2Ptr, u32 Len)
{
	volatile int RetVal = 1;
	const u8 *Buf1 = Buf1Ptr;
	const u8 *Buf2 = Buf2Ptr;
	u32 Size = Len;

	/* Assert validates the input arguments */
	if ((Buf1 == NULL) || (Buf2 == NULL) || (Len == 0x0U)) {
		goto END;
	}

	/* Loop and compare */
	while (Size != 0U) {
		if (*Buf1 > *Buf2) {
			RetVal = 1;
			goto END;
		} else if (*Buf1 < *Buf2) {
			RetVal = -1;
			goto END;
		} else {
			Buf1++;
			Buf2++;
			Size--;
		}
	}

	/* Make sure size is zero to know the whole of data is compared */
	if (Size == 0U) {
		RetVal = 0;
	}

END:
	return RetVal;
}

/*****************************************************************************/
/**
 * @brief	This function is used to zeroize the memory
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data in bytes.
 *
 * @return
 *		- XST_SUCCESS: If Zeroization is successful.
 *		- XST_FAILURE: If Zeroization is not successful.
 ********************************************************************************/
int Xil_SecureZeroize(u8 *DataPtr, const u32 Length)
{
	u32 Index;
	int Status = XST_FAILURE;

	/* Clear the data */
	(void)memset(DataPtr, 0, Length);

	/* Read it back to verify */
	 for (Index = 0U; Index < Length; Index++) {
		if (DataPtr[Index] != 0x00U) {
			goto END;
		}
	}
	if (Index == Length) {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}
