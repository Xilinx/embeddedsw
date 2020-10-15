/******************************************************************************
* Copyright (c) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_util.c
*
* This file which contains the code which interfaces with the CRP
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
* 1.01  bsv  04/18/2019 Added support for NPI readback and CFI readback
*       kc   04/26/2019 Updated Delay and Poll timeout based on timers
*       rm   06/27/2019 Added APIs for safety register writes
*       vnsl 07/19/2019 Added XPlmi_MemCmp API to check for PPK and SPK integrity
* 1.02  bsv  02/17/2020 Added 64-bit / 128-bit safety write APIs for xilsem
*       bsv  04/04/2020 Code clean up
* 1.03  kc   06/22/2020 Minor updates to PrintArray for better display
*       kc   08/17/2020 Added redundancy checks to XPlmi_MemCmp
*       bsv  09/04/2020 Added checks to validate input params for XPlmi_Strcat
*                       and XPlmi_Strcpy
*       bm   10/14/2020 Code clean up
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_util.h"
#include "xplmi_hw.h"
#include "xplmi_debug.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a register.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	Value is the value to be written to the register
 *
 * @return	None
 *
 *****************************************************************************/
void XPlmi_UtilRMW(u32 RegAddr, u32 Mask, u32 Value)
{
	u32 Val;

	Val = XPlmi_In32(RegAddr);
	Val = (Val & (~Mask)) | (Mask & Value);
	XPlmi_Out32(RegAddr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a register and then
 * read back to validate if the value read is the same as the expected value.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	Value is the value to be written to the register
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_UtilSafetyWrite(u32 RegAddr, u32 Mask, u32 Value)
{
	int Status = XST_FAILURE;
	u32 Val;

	Val = XPlmi_In32(RegAddr);
	Val = (Val & (~Mask)) | (Mask & Value);

	XPlmi_Out32(RegAddr, Val);
	if (XPlmi_In32(RegAddr) == Val) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr is the address of the register
 * @param	Mask denotes the bits to be modified
 * @param	ExpectedValue is the value for which the register is polled
 * @param	TimeOutInUs is the max time in microseconds for which the register
 *			would be polled for the expected value
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 *****************************************************************************/
int XPlmi_UtilPoll(u32 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * If timeout value is zero, max time out value is taken
	 */
	if (TimeOut == 0U) {
		TimeOut = XPLMI_TIME_OUT_DEFAULT;
	}
	/*
	 * Read the Register value
	 */
	RegValue = XPlmi_In32(RegAddr);
	/*
	 * Loop while the MAsk is not set or we timeout
	 */
	while (((RegValue & Mask) != ExpectedValue) && (TimeOut > 0U)) {
		usleep(1U);
		/*
		 * Latch up the Register value again
		 */
		RegValue = XPlmi_In32(RegAddr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}
	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit address till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr 64 bit address
 * @param	Mask is the bit field to be polled
 * @param	Expected Value is value to be polled
 * @param   TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPoll64(u64 RegAddr, u32 Mask, u32 ExpectedValue, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 ReadValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * If timeout value is zero, max time out value is taken
	 */
	if (TimeOut == 0U) {
		TimeOut = XPLMI_TIME_OUT_DEFAULT;
	}
	/*
	 * Read the Register value
	 */
	ReadValue = XPlmi_In64(RegAddr);
	/*
	 * Loop while the Mask is not set or we timeout
	 */
	while (((ReadValue & Mask) != ExpectedValue) && (TimeOut > 0U)) {
		usleep(1U);
		/*
		 * Latch up the value again
		 */
		ReadValue = XPlmi_In64(RegAddr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}
	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	RegAddr is the register address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask(u32 RegAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u32 RegValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * Read the Register value
	 */
	RegValue = XPlmi_In32(RegAddr);

	/*
	 * Loop while the MAsk is not set or we timeout
	 */
	while (((RegValue & Mask) != Mask) && (TimeOut > 0U)) {
		/*
		 * Latch up the Register value again
		 */
		RegValue = XPlmi_In32(RegAddr);

		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}

	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function polls a 64 bit register till the masked bits are set to
 * expected value or till timeout occurs.
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	TimeOutInUs is delay time in micro sec
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilPollForMask64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 TimeOutInUs)
{
	int Status = XST_FAILURE;
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadValue;
	u32 TimeOut = TimeOutInUs;

	/*
	 * Read the Register value
	 */
	ReadValue = lwea(Addr);
	 /*
	 * Loop while the Mask is not set or we timeout
	 */
	while (((ReadValue & Mask) != Mask) && (TimeOut > 0U)) {
		usleep(1U);
		/*
		 * Latch up the value again
		 */
		ReadValue = lwea(Addr);
		/*
		 * Decrement the TimeOut Count
		 */
		TimeOut--;
	}
	if (TimeOut > 0U) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function will Read, Modify and Write to a 64 bit register.
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAaddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	Value is value to be updated
 *
 * @return	None
 *
 ******************************************************************************/
void XPlmi_UtilRMW64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadVal;

	ReadVal = lwea(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);
	swea(Addr, ReadVal);
}

/*****************************************************************************/
/**
 * @brief	The function writes data to 64 bit address and validates the
 * operation by reading back the contents of the address.
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAaddr is lower 32-bits of 64-bit address
 * @param	Mask is the bit field to be updated
 * @param	Value is value to be updated
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_UtilSafetyRMW64(u32 HighAddr, u32 LowAddr, u32 Mask, u32 Value)
{
	int Status = XST_FAILURE;
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	u32 ReadVal;

	ReadVal = XPlmi_In64(Addr);
	ReadVal = (ReadVal & (~Mask)) | (Mask & Value);
	XPlmi_Out64(Addr, ReadVal);
	if (XPlmi_In64(Addr) == ReadVal) {
		Status = XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
 * @brief 	This function writes to a 64 bit address
 *
 * @param	HighAddr is higher 32-bits of 64-bit address
 * @param	LowAddr is lower 32-bits of 64-bit address
 * @param	Value is value to be updated
 *
 * @return	None
 *
 ******************************************************************************/
void XPlmi_UtilWrite64(u32 HighAddr, u32 LowAddr, u32 Value)
{
	u64 Addr = (((u64)HighAddr << 32U) | LowAddr);
	swea(Addr, Value);
}

/****************************************************************************/
/**
* @brief	This function is used to print the entire array in bytes as specified by the
* debug type.
*
* @param	DebugType printing of the array will happen as defined by the debug type
* @param	Buf pointer to the  buffer to be printed
* @param	Len length of the bytes to be printed
* @param	Str pointer to the data that is printed along the data
*
* @return	None
*
*****************************************************************************/
void XPlmi_PrintArray (u32 DebugType, const u64 BufAddr, u32 Len, const char *Str)
{
	u32 Index;
	u64 Addr = BufAddr;

	if ((DebugType & XPlmiDbgCurrentTypes) != 0U) {
		XPlmi_Printf(DebugType, "%s START, Len:0x%08x\r\n 0x%08x%08x: ",
			     Str, Len, (u32)(Addr >> 32U), (u32)Addr);
		for (Index = 0U; Index < Len; Index++) {
			XPlmi_Printf_WoTimeStamp(DebugType, "0x%08x ",
				XPlmi_In64(Addr));
			if (((Index + 1U) % XPLMI_WORD_LEN) == 0U) {
				XPlmi_Printf_WoTimeStamp(DebugType,
				"\r\n 0x%08x%08x: ", (u32)(Addr >> 32U), (u32)Addr);
			}
			Addr += XPLMI_WORD_LEN;
		}
		XPlmi_Printf_WoTimeStamp(DebugType, "\r\n");
		XPlmi_Printf(DebugType, "%s END\r\n", Str);
	}
	return;
}

/*****************************************************************************/
/**
 * @brief	This functions copies source string to destination string.
 *
 * @param	DestPtr is pointer to destination string
 * @param	SrcPtr is pointer to source string
 * @param	Size is the maximum number of bytes of the source string
 *		to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_Strcpy(char *DestPtr, const char *SrcPtr, const u32 Size)
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

/*****************************************************************************/
/**
 * @brief	This function appends string2 to string1.
 *
 * @param	Str1Ptr is pointer to string1
 * @param	Str2Ptr is pointer to string2
 * @param	Size is the maximum number of bytes Str1 can hold
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_Strcat(char* Str1Ptr, const char* Str2Ptr, const u32 Size)
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
 *			filled with 0s till DestPtrLen bytes and is considered as a failure
 *
 * @param	DestPtr is pointer to destination address
 * @param	DestPtrLen is the memory alloted to the destination buffer
 * @param	SrcPtr is pointer to source address
 * @param	Len is number of bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
int XPlmi_MemCpy(void *DestPtr, u32 DestPtrLen, const void *SrcPtr, u32 Len)
{
	int Status = XST_FAILURE;
	u8 *Dest = (u8 *)DestPtr;
	const u8 *Src = (const u8 *)SrcPtr;
	u32 DestLen = DestPtrLen;
	u32 Size = Len;

	if ((DestPtr == NULL) || (SrcPtr == NULL)) {
		goto END;
	}

	if (Len > DestLen) {
		while (DestLen != 0U) {
			*Dest = 0U;
			Dest++;
			DestLen--;
		}
		goto END;
	}

	/* Loop and copy.  */
	while (Size != 0U) {
		*Dest = *Src;
		Dest++;
		Src++;
		Size--;
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function compares Len bytes from memory1 and memory2.
 *
 * @param	Buf1Ptr is pointer to memory1
 * @param	Buf2Ptr is pointer to memory2
 * @param	Len is number of byets to be compared
 *
 * @return	0 if contents of both the memory regions are same,
 *			-1/1 if first non matching character has
 *			lower/greater value in Buf1Ptr
 *
 ******************************************************************************/
int XPlmi_MemCmp(const void *Buf1Ptr, const void *Buf2Ptr, u32 Len)
{
	volatile int RetVal = 1;
	const u8 *Buf1 = Buf1Ptr;
	const u8 *Buf2 = Buf2Ptr;
	u32 Size = Len;

	/* Assert validates the input arguments */
	Xil_AssertNonvoid(Buf1 != NULL);
	Xil_AssertNonvoid(Buf2 != NULL);
	Xil_AssertNonvoid(Len != 0x0U);

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

/****************************************************************************/
/**
* @brief	This function calculates the length the input string
*
* @param	String is the input string
* @param	MaxStrLen is the maximum size possible for the string
*
* @return	The length of the input String on success, zero if String is
* 			a null pointer, MaxStrLen if the null character is not found
* 			within MaxStrLen bytes of String
*
*****************************************************************************/
u32 XPlmi_StrLen(const char *String, const u32 MaxStrLen)
{
	u32 Index = 0U;

	if (String == NULL) {
		goto END;
	}

	for (; Index < MaxStrLen; Index++) {
		if (String[Index] == '\0') {
			break;
		}
	}

END:
	return Index;
}
