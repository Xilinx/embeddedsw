/******************************************************************************/
/**
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_sutil.c
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
* 9.2   kpt      04/21/19 First release.
* 9.3   sk       02/05/25 Added overlap check in Xil_MemCpy64
* 9.3   ml       02/19/25 Fix Type Mismatch in Xil_UtilRMW32
*       ng       03/25/25 Prevent compiler optimization by using volatile for status variable,
*                         add checks for RISC-V MB proc and zeroize memory before return
*       ng       04/07/25 Prevent overwriting of the status variable in Xil_SReverseData
* 9.4   ml       09/01/25 Fix MISRA-C violation for Rule 17.7
* 9.4   vmt      24/09/25 Added extended address support for RISC-V
*       har      10/10/25 Updated datatype of Len in Xil_ConvertStringToHex
*       hj       14/10/25 Remove zero address check in Xil_SMemCpy
*
* </pre>
*
*****************************************************************************/

/****************************** Include Files *********************************/
#include "xil_sutil.h"
#include "sleep.h"
#ifdef SDT
#include "bspconfig.h"
#endif

/************************** Constant Definitions ****************************/
#define MAX_NIBBLES	8U /**< maximum nibbles */
#define XIL_WORD_SIZE		    (4U) /**< WORD size in BYTES */
#define XIL_WORD_ALIGN_MASK		(XIL_WORD_SIZE - 1U)/**< WORD alignment */
#define XIL_ONE_BYTE	(1U) /**< One byte length */

/************************** Function Prototypes *****************************/

void (*fptr)(void) = NULL;

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
	} else if ((InChar >= (u8)'a') && (InChar <= (u8)'f')) {
		*Num = InChar - (u8)'a' + 10U;
		Status = XST_SUCCESS;
	} else if ((InChar >= (u8)'A') && (InChar <= (u8)'F')) {
		*Num = InChar - (u8)'A' + 10U;
		Status = XST_SUCCESS;
	} else {
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
 *          XST_FAILURE - Invalid character in input string
 *
 * @note    None.
 *
 *****************************************************************************/
u32 Xil_ConvertStringToHex(const char *Str, u32 *buf, u32 Len)
{
	u32 Status = XST_FAILURE;
	u32 ConvertedLen = 0U;
	u32 index = 0U;
	u8 Nibble[MAX_NIBBLES] = {0U};
	u8 i;

	while (ConvertedLen < Len) {
		for (i = 0U; i < MAX_NIBBLES; i++) {
			Status = Xil_ConvertCharToNibble((u8)Str[ConvertedLen],
							 &Nibble[i]);
			ConvertedLen = ConvertedLen + 1U;
			if (Status != XST_SUCCESS) {
				/* Error converting char to nibble */
				goto END;
			}
		}

		buf[index] = (((u32)Nibble[0] << (u8)28U) | ((u32)Nibble[1] << (u8)24U) |
			      ((u32)Nibble[2] << (u8)20U) | ((u32)Nibble[3] << (u8)16U) |
			      ((u32)Nibble[4] << (u8)12U) | ((u32)Nibble[5] << (u8)8U)  |
			      ((u32)Nibble[6] << (u8)4U)  | (u32)Nibble[7]);
		index++;
	}
END:
	return Status;
}

#ifdef VERSAL_PLM
/****************************************************************************/
/*
 * Register PLM Alive Handler.
 *
 * @param   PlmAlive	- Pointer to PlmAlive API.
 *
 * @return    None.
 *
 *****************************************************************************/
void Xil_RegisterPlmHandler(void (*PlmAlive) (void))
{
	fptr = PlmAlive;
}

/****************************************************************************/
/*
 * PLM Stub Handler called while waiting for event.
 *
 * @param   None.
 *
 * @return    None.
 *
 *****************************************************************************/
void Xil_PlmStubHandler(void)
{
	if (fptr != NULL) {
		fptr();
	}
}
#endif

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
u32 Xil_WaitForEvent(UINTPTR RegAddr, u32 EventMask, u32 Event, u32 Timeout)
{
	u32 EventStatus;
	u32 PollCount = Timeout;
	u32 Status = XST_FAILURE;

	while (PollCount > 0U) {
		EventStatus = Xil_In32(RegAddr) & EventMask;
		if (EventStatus == Event) {
			Status = XST_SUCCESS;
			break;
		}
		PollCount--;
#ifdef VERSAL_PLM
		Xil_PlmStubHandler();
#endif
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
u32 Xil_WaitForEvents(UINTPTR EventsRegAddr, u32 EventsMask, u32 WaitEvents,
		      u32 Timeout, u32 *Events)
{
	u32 EventStatus;
	u32 PollCount = Timeout;
	u32 Status = XST_TIMEOUT;

	*Events = 0x00;
	do {
		EventStatus = Xil_In32(EventsRegAddr);
		EventStatus &= EventsMask;
		if ((EventStatus & WaitEvents) != 0U) {
			Status = XST_SUCCESS;
			*Events = EventStatus;
			break;
		}
		PollCount--;
#ifdef VERSAL_PLM
		Xil_PlmStubHandler();
#endif
		usleep(1U);
	} while (PollCount > 0U);

	return Status;
}

/****************************************************************************/
/*
 * Waits for the event to be set with in timeout and returns error in case
 * event was not set in specified time out.
 *
 * @param   Timeout     - Max number of microseconds to wait for an event,
 *			  It should be maximum timeout needed among the
 *			  events specified.
 * @param   NumOfEvents - Number of event(s) to be checked.
 * @param   EventAddr   - Pointer to address of event(s) to be set.
 *
 * @return
 *          XST_SUCCESS - On occurrence of the event(s).
 *          XST_FAILURE - Event(s) did not occur before counter reaches 0
 *
 * @note    None.
 *
 *****************************************************************************/
u32 Xil_WaitForEventSet(u32 Timeout, u32 NumOfEvents, volatile u32 *EventAddr, ...)
{
	u32 PollCount = Timeout;
	u32 Status = XST_FAILURE;
	u32 LoopCnt = 0, i;
	va_list Event;

	va_start(Event, EventAddr);
	/* wait for all events to complete */
	for (i = 0; i < NumOfEvents; i++) {
		while (PollCount > 0U) {
			if (Xil_In32((UINTPTR)EventAddr)) {
				LoopCnt++;
				break;
			}
			PollCount--;
#ifdef VERSAL_PLM
			Xil_PlmStubHandler();
#endif
			usleep(1U);
		}
		if (PollCount == 0U) {
			goto END;
		}
		EventAddr = va_arg(Event, volatile u32 *);
		PollCount = Timeout;
	}

END:
	if (LoopCnt == NumOfEvents) {
		Status = XST_SUCCESS;
	}

	va_end(Event);
	return Status;
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
void Xil_UtilRMW32(UINTPTR Addr, u32 Mask, u32 Value)
{
	u32 Val;

	Val = Xil_In32(Addr);
	Val = (Val & (~Mask)) | (Mask & Value);
	Xil_Out32(Addr, Val);
}

/*****************************************************************************/
/**
 * @brief	This function copies Len bytes from source memory to destination
 *			memory. If Len is greater than DestPtrLen, then DestPtr is also
 *			filled with 0s till DestPtrLen bytes and is considered as a failure.
 *			This function is a secure implementation of memcpy
 *
 * @param	DestPtr is pointer to destination address
 * @param	DestPtrLen is the memory allotted to the destination buffer
 * @param	SrcPtr is pointer to source address
 * @param	Len is number of bytes to be copied
 *
 * @return	XST_SUCCESS on success and error code on failure
 *
 ******************************************************************************/
s32 Xil_SecureMemCpy(void *DestPtr, u32 DestPtrLen, const void *SrcPtr, u32 Len)
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
 * @brief	This function is used to zeroize the memory
 *
 * @param	DataPtr Pointer to the memory which need to be zeroized.
 * @param	Length	Length of the data in bytes.
 *
 * @return
 *		- XST_SUCCESS: If Zeroization is successful.
 *		- XST_FAILURE: If Zeroization is not successful.
 ********************************************************************************/
s32 Xil_SecureZeroize(u8 *DataPtr, const u32 Length)
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

/*****************************************************************************/
/**
 * @brief	This function compares two memory regions for specified number
 *		of bytes. This function takes size of two memory regions to
 *		make sure not to read from or write to out of bound memory
 *		region.
 *
 * @param	Src1     - Pointer to first memory range
 * @param	Src1Size - Maximum size of first memory range
 * @param	Src2     - Pointer to second memory range
 * @param	Src2Size - Maximum size of second memory range
 * @param	CmpLen   - Number of bytes to be compared
 *
 * @return
 *		XST_SUCCESS - If specified number of bytes matches
 * 		XST_FAILURE - If there is a mistmatch found during comparison
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SMemCmp(const void *Src1, const u32 Src1Size,
		const void *Src2, const u32 Src2Size, const u32 CmpLen)
{
	int Status = XST_FAILURE;

	if ((Src1 == NULL) || (Src2 == NULL)) {
		Status =  XST_INVALID_PARAM;
	} else if ((CmpLen == 0U) || (Src1Size < CmpLen) || (Src2Size < CmpLen)) {
		Status =  XST_INVALID_PARAM;
	} else {
		Status = memcmp (Src1, Src2, CmpLen);
		if (Status != 0) {
			Status = XST_FAILURE;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function compares two memory regions for specified number
 *		of bytes. This function takes size of two memory regions to
 *		make sure not to read from or write to out of bound memory
 *		region.
 *		Note that this function compares till end to make it execute
 *		in constant time irrespective of the content of input memory.
 *
 * @param	Src1     - Pointer to first memory range
 * @param	Src1Size - Maximum size of first memory range
 * @param	Src2     - Pointer to second memory range
 * @param	Src2Size - Maximum size of second memory range
 * @param	CmpLen   - Number of bytes to be compared
 *
 * @return
 *		XST_SUCCESS - If specified number of bytes matches
 * 		XST_FAILURE - If mistmatch
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SMemCmp_CT(const void *Src1, const u32 Src1Size,
		   const void *Src2, const u32 Src2Size, const u32 CmpLen)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusRedundant = XST_FAILURE;
	volatile u32 Data = 0U;
	volatile u32 DataRedundant = 0xFFFFFFFFU;
	u32 Cnt = CmpLen;
	const u8 *Src_1 = (const u8 *)Src1;
	const u8 *Src_2 = (const u8 *)Src2;


	if ((Src1 == NULL) || (Src2 == NULL)) {
		Status =  XST_INVALID_PARAM;
	} else if ((CmpLen == 0U) || (Src1Size < CmpLen) || (Src2Size < CmpLen)) {
		Status =  XST_INVALID_PARAM;
	} else {
		while (Cnt >= sizeof(u32)) {
			Data |= (*(const u32 *)Src_1 ^ * (const u32 *)Src_2);
			DataRedundant &= ~Data;
			Src_1 += sizeof(u32);
			Src_2 += sizeof(u32);
			Cnt -= sizeof(u32);
		}

		while (Cnt > 0U) {
			Data |= (u32)(*Src_1 ^ *Src_2);
			DataRedundant &= ~Data;
			Src_1++;
			Src_2++;
			Cnt--;
		}

		if ((Data == 0U) && (DataRedundant == 0xFFFFFFFFU)) {
			Status = XST_SUCCESS;
			StatusRedundant = XST_SUCCESS;
		}
	}

	return (Status | StatusRedundant);
}

/*****************************************************************************/
/**
 * @brief	This is wrapper function to memcpy function. This function
 *		takes size of two memory regions to make sure not read from
 *		or write to out of bound memory region. Since 0 is valid address,
*		Src and Dest paramemter are not checked for NULL
 *
 * @param	Dest      - Pointer to destination memory
 * @param	DestSize  - Memory available at destination
 * @param	Src       - Pointer to source memory
 * @param	SrcSize   - Maximum data that can be copied from source
 * @param	CopyLen   - Number of bytes to be copied
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SMemCpy(void *Dest, const u32 DestSize,
		const void *Src, const u32 SrcSize, const u32 CopyLen)
{
	int Status = XST_FAILURE;
	const u8 *Src8 = (const u8 *) Src;
	const u8 *Dst8 = (u8 *) Dest;
	void *volatile DestTemp = Dest;
	const void *volatile SrcTemp = Src;

	if ((CopyLen == 0U) || (DestSize < CopyLen) || (SrcSize < CopyLen)) {
		Status =  XST_INVALID_PARAM;
	}
	/* Return error for overlap string */
	else if ((Src8 < Dst8) && (&Src8[CopyLen - 1U] >= Dst8)) {
		Status =  XST_INVALID_PARAM;
	} else if ((Dst8 < Src8) && (&Dst8[CopyLen - 1U] >= Src8)) {
		Status =  XST_INVALID_PARAM;
	} else {
		(void)memcpy(DestTemp, SrcTemp, CopyLen);
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is wrapper function to memset function. This function
 *		writes specified byte to destination specified number of times.
 *		This function also takes maximum string size that destination
 *		holds to make sure not to write out of bound area.
 *
 * @param	Dest     - Pointer to destination memory
 * @param	DestSize - Memory available at destination
 * @param	Data     - Any value from 0 to 255
 * @param	Len      - Number of bytes to be copied
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SMemSet(void *Dest, const u32 DestSize,
		const u8 Data, const u32 Len)
{
	int Status = XST_FAILURE;

	if ((Dest == NULL) || (DestSize < Len) || (Len == 0U)) {
		Status =  XST_INVALID_PARAM;
	} else {
		(void)memset(Dest, (s32)Data, Len);
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function concatenates two strings. This function
 *		takes size of both strings to make sure not to read from /
 *		write to out of bound area.
 *
 * @param	DestStr  - Pointer to destination string
 * @param	DestSize - Maximum string size that destination can hold
 * @param	SrcStr   - Pointer to source string
 * @param	SrcSize  - Maximum string size that source can hold
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SStrCat (u8 *DestStr, const u32 DestSize,
		 const u8 *SrcStr, const u32 SrcSize)
{
	int Status = XST_FAILURE;
	u32 SrcLen;
	u32 DstLen;
	u32 Length;

	if ((DestStr == NULL) || (SrcStr == NULL)) {
		Status =  XST_INVALID_PARAM;
		goto END;
	}

	SrcLen = strnlen((const char *)SrcStr, SrcSize);
	DstLen = strnlen((const char *)DestStr, DestSize);
	Length = SrcLen + DstLen;

	if ((DestSize <= DstLen) || (SrcSize <= SrcLen)) {
		Status =  XST_INVALID_PARAM;
	} else if (DestSize <= Length) {
		Status =  XST_INVALID_PARAM;
	} else {
		(void)strncat((char *)DestStr, (const char *)SrcStr, Length);
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function compares two strings. It also takes maximum string
 *		size that Src1 and Src2 can hold to make sure not to read out of
 *		bound data for comparison.
 *
 * @param	Str1     - Pointer to first string
 * @param	Str1Size - Maximum string size that Str1 can hold
 * @param	Str2     - Pointer to second string
 * @param	Str2Size - Maximum string size that Str2 can hold
 *
 * @return
 *		XST_SUCCESS - If both strings are same
 *		XST_FAILURE - If there is difference between two strings
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SStrCmp(const u8 *Str1, const u32 Str1Size,
		const u8 *Str2, const u32 Str2Size)
{
	int Status = XST_FAILURE;
	u32 Str1Len = 0U;
	u32 Str2Len = 0U;

	if ((Str1 == NULL) || (Str2 == NULL)) {
		Status =  XST_INVALID_PARAM;
		goto END;
	}

	Str1Len = strnlen((const char *)Str1, Str1Size);
	Str2Len = strnlen((const char *)Str2, Str2Size);

	if ((Str1Size <= Str1Len) || (Str2Size <= Str2Len)) {
		Status =  XST_INVALID_PARAM;
	} else if ((Str1Len < Str2Len) || (Str1Len > Str2Len)) {
		Status = XST_FAILURE;
	} else {
		Status = memcmp(Str1, Str2, Str1Len);
		if (Status != 0) {
			Status = XST_FAILURE;
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function compares two strings. It also takes maximum string
 *		size that Src1 and Src2 can hold to make sure not to read out of
 *		bound data for comparison. This function compares each character
 *		of the strings so that execution time of the function is not
 *		dependent on the content of two input strings. The execution
 *		time is constant when string size are same.
 *
 * @param	Str1     - Pointer to first string
 * @param	Str1Size - Maximum string size that Str1 can hold
 * @param	Str2     - Pointer to second string
 * @param	Str2Size - Maximum string size that Str2 can hold
 *
 * @return
 *		XST_SUCCESS - If both strings are same
 *		XST_FAILURE - If there is difference between two strings
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SStrCmp_CT (const u8 *Str1, const u32 Str1Size,
		    const u8 *Str2, const u32 Str2Size)
{
	int Status = XST_FAILURE;
	u32 Str1Len = 0U;
	u32 Str2Len = 0U;

	if ((Str1 == NULL) || (Str2 == NULL)) {
		Status =  XST_INVALID_PARAM;
		goto END;
	}

	Str1Len = strnlen((const char *)Str1, Str1Size);
	Str2Len = strnlen((const char *)Str2, Str2Size);

	if ((Str1Size <= Str1Len) || (Str2Size <= Str2Len)) {
		Status =  XST_INVALID_PARAM;
	} else if (Str1Len != Str2Len) {
		Status = XST_FAILURE;
	} else {
		Status = Xil_SMemCmp_CT (Str1, Str1Size, Str2, Str2Size, Str1Len);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function copies one string to other. This function
 *		takes size of both strings to make sure not to read from /
 *		write to out of bound area.
 *
 * @param	DestStr  - Pointer to destination string
 * @param	DestSize - Maximum string size that destination can hold
 * @param	SrcStr   - Pointer to source string
 * @param	SrcSize  - Maximum string size that source can hold
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SStrCpy(u8 *DestStr, const u32 DestSize,
		const u8 *SrcStr, const u32 SrcSize)
{
	int Status = XST_FAILURE;
	u32 SrcLen = 0U;

	if ((DestStr == NULL) || (SrcStr == NULL)) {
		Status =  XST_INVALID_PARAM;
		goto END;
	}

	SrcLen = strnlen((const char *)SrcStr, SrcSize);

	if ((DestSize <= SrcLen) || (SrcSize <= SrcLen)) {
		Status =  XST_INVALID_PARAM;
	} else {
		(void)memcpy(DestStr, SrcStr, SrcLen + 1U);
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This is wrapper function to memmove function. This function
 *		takes size of two memory regions to avoid out of bound memory region.
 *
 * @param	Dest      - Pointer to destination memory
 * @param	DestSize  - Memory available at destination
 * @param	Src       - Pointer to source memory
 * @param	SrcSize   - Maximum data that can be copied from source
 * @param	CopyLen   - Number of bytes to be copied
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *
 *****************************************************************************/
s32 Xil_SMemMove(void *Dest, const u32 DestSize,
		 const void *Src, const u32 SrcSize, const u32 CopyLen)
{
	volatile int Status = XST_FAILURE;
	const void *Output = NULL;

	if ((Dest == NULL) || (Src == NULL)) {
		Status =  XST_INVALID_PARAM;
	} else if ((CopyLen == 0U) || (DestSize < CopyLen) || (SrcSize < CopyLen)) {
		Status =  XST_INVALID_PARAM;
	} else {
		Output = memmove(Dest, Src, CopyLen);
		if (Output != NULL) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief
 * Performs a Read Modify Write operation for a memory location by writing the
 * 32 bit value to the the specified address and then reading it back to
 * verify the value written in the register.
 *
 * @param	Addr contains the address to perform the output operation
 * @param	Mask indicates the bits to be modified
 * @param	Value contains 32 bit Value to be written at the specified address
 *
 * @return
 *         XST_SUCCESS on success
 *         XST_FAILURE on failure
 *
 *****************************************************************************/
s32 Xil_SecureRMW32(UINTPTR Addr, u32 Mask, u32 Value)
{
	s32 Status = XST_FAILURE;
	u32 ReadReg;
	u32 Val;

	Val = Xil_In32(Addr);
	Val = (Val & (~Mask)) | (Mask & Value);
	Xil_Out32(Addr, Val);

	/* verify value written to specified address */
	ReadReg = Xil_In32(Addr) & Mask;

	if (ReadReg == (Mask & Value)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function changes the endianness of source data and copies it
 *          into destination buffer.
 *
 * @param	Dest      - Pointer to destination memory
 * @param	DestSize  - Memory available at destination
 * @param	Src       - Pointer to source memory
 * @param	SrcSize   - Maximum data that can be copied from source
 * @param	CopyLen   - Number of bytes to be copied
 *
 * @return
 *		XST_SUCCESS - Copy is successful
 * 		XST_INVALID_PARAM - Invalid inputs
 *      XST_FAILURE       - On failure
 *
 *****************************************************************************/
s32 Xil_SChangeEndiannessAndCpy(void *Dest, const u32 DestSize,
		const void *Src, const u32 SrcSize, const u32 CopyLen)
{
	s32 Status = XST_FAILURE;
	volatile u32 Index;
	const u8 *Src8 = (const u8 *) Src;
	const u8 *Dst8 = (u8 *) Dest;
	u8 *DestTemp = Dest;
	const u8 *SrcTemp = Src;

	if ((Dest == NULL) || (Src == NULL)) {
		Status =  XST_INVALID_PARAM;
	} else if ((CopyLen == 0U) || (DestSize < CopyLen) || (SrcSize < CopyLen)) {
		Status =  XST_INVALID_PARAM;
	}
	/* Return error for overlap string */
	else if ((Src8 < Dst8) && (&Src8[CopyLen - 1U] >= Dst8)) {
		Status =  XST_INVALID_PARAM;
	} else if ((Dst8 < Src8) && (&Dst8[CopyLen - 1U] >= Src8)) {
		Status =  XST_INVALID_PARAM;
	} else {
		for (Index = 0U; Index < CopyLen; Index++) {
			DestTemp[Index] = SrcTemp[CopyLen - Index - 1U];
		}
		if (Index == CopyLen) {
			Status = XST_SUCCESS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function changes the endianness of given buffer by reversing the byte order.
 *
 * @param	Buf - Pointer to the buffer whose data needs to be reversed.
 * @param	Size - is the size of the buffer in bytes to be reversed.
 *
 * @return
 *		XST_SUCCESS - Data successfully reversed
 *		XST_INVALID_PARAM - Invalid parameters (NULL buffer or zero size)
 *		XST_FAILURE - Failed to reverse data or zeroize temporary variables
 *
 ******************************************************************************/
s32 Xil_SReverseData(void *Buf, u32 Size)
{
	volatile s32 Status = XST_FAILURE;
	volatile s32 SStatus = XST_FAILURE;
	volatile u32 Index = 0U;
	u8 *Buffer = (u8 *)Buf;
	u8 Data= 0U;
	u32 LoopCnt = 0U;

	/* Input validation */
	if ((Buf == NULL) || (Size == 0U)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	LoopCnt =  Size/2U;
	for (Index = 0U; Index < LoopCnt; Index++) {
		Data = Buffer[Size - Index - 1U];
		Buffer[Size - Index - 1U] = Buffer[Index];
		Buffer[Index] = Data;
	}

	if (Index == LoopCnt) {
		Status = XST_SUCCESS;
	}

	SStatus = Xil_SecureZeroize(&Data, XIL_ONE_BYTE);
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function copies data from 64 bit address Src to 64 bit
 * address Dst
 *
 * @param	DstAddr is the 64 bit destination address
 * @param	SrcAddr is the 64 bit source address
 * @param	Cnt is the number of bytes of data to be copied
 *
 *************************************************************************************************/
void Xil_MemCpy64(u64 DstAddr, u64 SrcAddr, u32 Cnt)
{
	/* Checking for overlap */
	if (((SrcAddr < DstAddr) && ((SrcAddr + Cnt) <= DstAddr)) ||
	    ((DstAddr < SrcAddr) && ((DstAddr + Cnt) <= SrcAddr))) {
#if (defined(__riscv) && (__riscv_xlen == 32) && (XPAR_MICROBLAZE_RISCV_ADDR_SIZE > 32)) || \
    (defined(__MICROBLAZE__) && (XPAR_MICROBLAZE_ADDR_SIZE > 32) && (XPAR_MICROBLAZE_DATA_SIZE == 32)) || \
    defined(VERSAL_PLM)
			u64 Dst = DstAddr;
			u64 Src = SrcAddr;
			u32 Count = Cnt;
			if (((Dst & XIL_WORD_ALIGN_MASK) == 0U) &&
			    ((Src & XIL_WORD_ALIGN_MASK) == 0U)) {
					while (Count >= sizeof (int)) {
						swea(Dst, lwea(Src));
						Dst += sizeof(int);
						Src += sizeof(int);
						Count -= (u32)sizeof(int);
					}
			}
			while (Count > 0U) {
				sbea(Dst, lbuea(Src));
				Dst += 1U;
				Src += 1U;
				Count -= 1U;
			}
		}
#else
		(void)memcpy((void *)(UINTPTR)DstAddr, (void *)(UINTPTR)SrcAddr, Cnt);
	}
#endif
}
