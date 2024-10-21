/******************************************************************************/
/**
* Copyright (C) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_util.c
*
* xil_util.c file contains xil utility functions
* Except few functions, most of these functions are wrappers to standard functions.
* The standard string functions do not validate the input and that results into
* buffer overflows. To avoid it, the wrapper function validates the input and
* then passed to standard function. There are few constant time functions
* ( xxx_CT() ) which are used to compare the data in constant time.
* The constant time functions should be used while comparing secure data
* like password, keys which prevent disclosing of the data using
* timing analysis.
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
* 7.7   kpt      11/09/21 Added Xil_SMemCmp, Xil_SMemCmp_CT, Xil_SMemCpy,
*                         Xil_SMemSet, Xil_SStrCat, Xil_SStrCmp, Xil_SStrCmp_CT
*                         Xil_SStrCpy functions
*       kpt      11/25/21 Added strnlen function to fix ARMCC compilation
*                         failure
* 7.7	sk	 01/10/22 Update functions return type and update RetVal variable
* 			  data type to fix misra_c_2012_directive_4_6 misrac
* 			  violations.
* 7.7	sk	 01/10/22 Update values from signed to unsigned to fix
* 			  misra_c_2012_rule_10_4 violation.
* 7.7	sk	 01/10/22 Add explicit parentheses to fix misra_c_2012_rule_12_1
* 			  violation.
* 7.7	sk	 01/10/22 Typecast character strings to u8 to fix misra_c_2012_rule_
* 			  10_3 violation.
* 7.7	sk	 01/10/22 Modify the code to reduce multiple break statements
* 			  and fix misra_c_2012_rule_15_4 violation.
* 7.7	sk	 01/10/22 Modify Xil_SMemCmp_CT and Xil_SMemCmp function argument
* 			  type to fix misra_c_2012_rule_8_3 violation.
* 7.7	sk	 01/10/22 Update conditional expression to fix misra_c_2012_rule_14_4
* 			  violation.
*       bm       01/20/22 Fix compilation warnings in Xil_SMemCpy
*       mmd      02/28/22 Added Xil_SMemMove function
* 8.0	sk	 03/02/22 Add explicit parentheses to fix misra_c_2012_rule_12_1
* 			  violation.
* 8.0	sk	 03/02/22 Add const qualifier to variables to fix misra_c_2012_rule_
* 			  11_8 violation.
* 8.0	sk	 03/02/22 Typecast expression with unsigned int to fix
* 			  misra_c_2012_rule_10_7 violation.
* 8.0	sk	 03/02/22 Typecast variables with unsigned or signed to fix misra_c
* 			  _2012_rule_10_3 violation.
* 8.0	sk	 03/02/22 Add const to unmodified variable to fix misra_c_2012
* 			  _rule_8_13 violation.
* 8.0	sk	 03/02/22 Typecast the function with void as return type is
* 			  not used and fix misra_c_2012_rule_17_7 violation.
* 8.0	sk	 03/02/22 Remove increment operations during comparison to
* 			  fix misra_c_2012_rule_13_3 violation.
* 8.0	sk	 03/02/22 Update values from signed to unsigned to fix
* 			  misra_c_2012_rule_10_4 violation.
* 8.0	sk	 03/17/22 Add const to unmodified pointer variable to fix misra_c
*			  _2012_rule_8_13 violation.
* 8.0   adk      04/18/22 Added Xil_WaitForEventSet function.
*       adk	 07/15/22 Updated the Xil_WaitForEventSet() API to
*			  support variable number of events.
*	ssc	 08/25/22 Added Xil_SecureRMW32 API
* 8.1	sa       09/29/22 Change the type of first argument passed to Xil_WaitForEvent
*			  API from u32 to UINTPTR for supporting 64 bit addressing.
* 8.1   sa       10/20/22 Change the type of first argument passed to Xil_WaitForEvents
*                         API from u32 to UINTPTR for supporting 64 bit addressing.
* 8.1   akm      01/02/23 Added Xil_RegisterPlmHandler() & Xil_PlmStubHandler() APIs.
* 9.0   ml       03/03/23 Add description to fix doxygen warnings.
* 9.0   ml       04/26/23 Updated code to fix DC.STRING BUFFER and VARARGS coverity warnings.
* 9.0   ml       09/13/23 Replaced numerical types (int) with proper typedefs(s32) to
*                         fix MISRA-C violations for Rule 4.6
* 9.1   kpt      02/21/24 Added Xil_SChangeEndiannessAndCpy function
* 9.2   kpt      06/24/24 Added Xil_SReverseData function
*       pre      08/16/24 Added Xil_MemCpy64 function
*       pre      08/29/24 Fixed compilation warning
*       kpt      10/17/24 Move API's used in secure libs to xil_sutil.c
*
* </pre>
*
*****************************************************************************/

/****************************** Include Files *********************************/
#include "xil_util.h"
#include "sleep.h"
#ifdef SDT
#include "bspconfig.h"
#endif

/************************** Constant Definitions ****************************/

/************************** Function Prototypes *****************************/

#ifdef __ARMCC_VERSION
/******************************************************************************/
/**
 *
 * This API returns the length of the input string
 *
 * @param  StartPtr is the pointer to the input string
 * @param  StrSize is the maximum length of the input string
 *
 * @return Returns the length of the input string
 *
 ******************************************************************************/
static size_t strnlen (const char *StartPtr, size_t StrSize)
{
	const char *EndPtr = StartPtr;
	size_t StrLen = 0U;

	EndPtr = memchr(StartPtr, '\0', StrSize);
	if (EndPtr == NULL) {
		StrLen = StrSize;
	} else {
		StrLen = (size_t) (EndPtr - StartPtr);
	}

	return StrLen;
}
#endif

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
s32 Xil_Ceil(float Value)
{
	s32 Result = Value;

	if (Value > Result) {
		Result = Result + 1;
	}

	return Result;
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

	if (NULL == Ch) {
		goto END;
	}
	if (((*Ch >= '0') && (*Ch <= '9')) ||
	    ((*Ch >= 'a') && (*Ch <= 'f')) ||
	    ((*Ch >= 'A') && (*Ch <= 'F'))) {

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

	if (NULL == HexStr) {
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

	if (Len != (strlen(Str) * XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		Status = (u32)XST_INVALID_PARAM;
		goto END;
	}

	ConvertedLen = 0U;
	while (ConvertedLen < (Len / XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		if ((Xil_ConvertCharToNibble(((u8)Str[ConvertedLen]), &UpperNibble)
		     == (u32)XST_SUCCESS) && (Xil_ConvertCharToNibble(((u8)Str[ConvertedLen + 1U]),
					      &LowerNibble) == (u32)XST_SUCCESS)) {
			Buf[ConvertedLen / 2U] =
				(UpperNibble << XIL_SIZE_OF_NIBBLE_IN_BITS) |
				LowerNibble;
		} else {
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

	if (Len != (strlen(Str) * XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	StrIndex = (Len / XIL_SIZE_OF_BYTE_IN_BITS) - 1U;
	ConvertedLen = 0U;
	while (ConvertedLen < (Len / XIL_SIZE_OF_NIBBLE_IN_BITS)) {
		if ((Xil_ConvertCharToNibble(((u8)Str[ConvertedLen]),
					     &UpperNibble) == XST_SUCCESS) &&
		    (Xil_ConvertCharToNibble(((u8)Str[ConvertedLen + 1U]),
					     &LowerNibble) == XST_SUCCESS)) {
			Buf[StrIndex] =
				(UpperNibble << XIL_SIZE_OF_NIBBLE_IN_BITS) |
				LowerNibble;
			StrIndex = StrIndex - 1U;
		} else {
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

	while (StrLen < MaxLen) {
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
s32 Xil_Strcpy(char *DestPtr, const char *SrcPtr, const u32 Size)
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
 * @param	Dest  is a pointer to destination string
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
s32 Xil_StrCpyRange(const u8 *Src, u8 *Dest, u32 From, u32 To, u32 MaxSrcLen,
		    u32 MaxDstLen)
{
	int Status = XST_FAILURE;
	u32 SrcLength;
	u32 Index;

	if ((Src == NULL) || (Dest == NULL)) {
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

	for (Index = From; (Index <= To) && (Src[Index] != (u8)'\0'); Index++) {
		Dest[Index - From] = Src[Index];
	}

	Dest[Index - From] = (u8)'\0';
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
s32 Xil_Strcat(char *Str1Ptr, const char *Str2Ptr, const u32 Size)
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
		Str1Ptr[Count] = Str2Ptr[CountTmp];
		Count++;
		CountTmp++;
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
s32 Xil_MemCmp(const void *Buf1Ptr, const void *Buf2Ptr, u32 Len)
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
