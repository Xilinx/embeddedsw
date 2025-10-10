/******************************************************************************/
/**
* Copyright (C) 2024 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_sutil.h
* @addtogroup common_utilities Common Utility APIs
* @{
* @details
*
* xil_sutil.h file contains xil utility functions declarations
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
* 9.3   ml       02/19/25 Fix Type Mismatch in Xil_UtilRMW32
* 9.4   ml       07/24/25 Fixed GCC warnings
*       har      10/10/25 Updated datatype of Len in Xil_ConvertStringToHex
*
* </pre>
*
*****************************************************************************/

#ifndef XIL_SUTIL_H_
#define XIL_SUTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************* Include Files ********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/

/****************** Macros (Inline Functions) Definitions *********************/

/******************************************************************************/
/**
 *
 * Builds version number by concatenates 16-bit Major version and Minor version.
 *
 * @param   Major is the 16-bit major version number
 * @param   Minor is the 16-bit minor version number
 *
 * @return	32-bit version number
 *
 ******************************************************************************/
#define XIL_BUILD_VERSION(Major, Minor)		((((u32)Major) << 16U) | (Minor))

#ifdef __GNUC__
/******************************************************************************/
/**
 *
 * Updates the return value of the called function into Var and VarTmp variables
 * for redundancy. This is to avoid glitches from altering the return values of
 * security critical functions.
 *
 * @param   Var is the variable which holds the return value of function
 *          executed
 * @param   VarTmp is the variable which holds the value stored in Var
 * @param	Function is the function to be executed
 * @param	Other params are arguments to the called function
 *
 * @return	None
 *
 ******************************************************************************/
#define XSECURE_TEMPORAL_IMPL(Var, VarTmp, Function, ...) \
	{ \
		Var = XST_FAILURE; \
		VarTmp = XST_FAILURE; \
		Var = Function(__VA_ARGS__); \
		VarTmp = Var; \
	}

/******************************************************************************/
/**
 *
 * Adds redundancy while checking the status of the called function.
 * This is to avoid glitches from altering the return values of security
 * critical functions. The macro requires a label to be passed to "go to"
 * in case of error.
 *
 * @param   Label is the label defined in function and the control
 *          will jump to the label in case of XST_FAILURE
 * @param   Status is the variable which holds the return value of
 *          function executed
 * @param	Function is the function to be executed
 * @param	Other params are arguments to the called function
 *
 * @return	None
 *
 ******************************************************************************/
#define XSECURE_TEMPORAL_CHECK(Label, Status, Function, ...)   \
	{ \
		volatile int StatusTmpVal; \
		XSECURE_TEMPORAL_IMPL(Status, StatusTmpVal, Function, __VA_ARGS__); \
		if ((Status != XST_SUCCESS) || \
		    (StatusTmpVal != XST_SUCCESS)) { \
			if (((Status) != (StatusTmpVal)) || \
			    (Status == XST_SUCCESS)) { \
				Status = XST_GLITCH_ERROR; \
			}\
			goto Label; \
		} \
	}

/******************************************************************************/
/**
 *
 * Adds redundancy to the function call. This is to avoid glitches which can skip
 * a function call and cause altering of the code flow in security critical
 * functions.
 *
 * @param	Status is the variable which holds the return value of
 *		function executed
 * @param	StatusTmp is the variable which holds the return value of
 *		redundant function call executed
 * @param	Function is the function to be executed
 * @param	Other params are arguments to the called function
 *
 * @return	None
 *
 ******************************************************************************/
#define XSECURE_REDUNDANT_CALL(Status, StatusTmp, Function, ...)   \
	{ \
		Status = Function(__VA_ARGS__); \
		StatusTmp = Function(__VA_ARGS__); \
	}

/******************************************************************************/
/**
 *
 * Adds redundancy to the function call. This is to avoid glitches which can skip
 * a function call and cause altering of the code flow in security critical
 * functions.
 *
 * @param	Function is the function to be executed
 * @param	Other params are arguments to the called function
 *
 * @return	None
 *
 ******************************************************************************/
#define XSECURE_REDUNDANT_IMPL(Function, ...)   \
	{ \
		Function(__VA_ARGS__); \
		Function(__VA_ARGS__); \
	}

/******************************************************************************/
/**
 *
 * This Macro helps to detect glitches skipping the Status check
 * in case of error.
 *
 * @param 	None
 *
 * @return	None
 *
 ******************************************************************************/
#define XSECURE_STATUS_CHK_GLITCH_DETECT(Status)   \
	{ \
		if (Status == XST_SUCCESS) { \
			Status = (int)XST_GLITCH_ERROR; \
		} \
	}

#endif
/*************************** Function Prototypes ******************************/

/**< Converts input character to nibble */
u32 Xil_ConvertCharToNibble(u8 InChar, u8 *Num);

/**< Convert input hex string to array of 32-bits integers */
u32 Xil_ConvertStringToHex(const char *Str, u32 *buf, u32 Len);

#ifdef VERSAL_PLM
/**< Register PLM handler */
void Xil_RegisterPlmHandler(void (*PlmAlive) (void));

/**< Call PLM handler */
void Xil_PlmStubHandler(void);
#endif

/**< Waits for specified event */
u32 Xil_WaitForEvent(UINTPTR RegAddr, u32 EventMask, u32 Event, u32 Timeout);

/**< Waits for specified events */
u32 Xil_WaitForEvents(UINTPTR EventsRegAddr, u32 EventsMask, u32 WaitEvents,
		      u32 Timeout, u32 *Events);

/**< Read, Modify and Write to an address*/
void Xil_UtilRMW32(UINTPTR Addr, u32 Mask, u32 Value);

/**< Copies Len bytes from source memory to destination memory */
s32 Xil_SecureMemCpy(void *DestPtr, u32 DestPtrLen, const void *SrcPtr, u32 Len);

/**< Zeroizes the memory of given length */
s32 Xil_SecureZeroize(u8 *DataPtr, const u32 Length);

/**< Copies Len bytes from source memory to destination memory */
s32 Xil_SMemCpy (void *Dest, const u32 DestSize,
		 const void *Src, const u32 SrcSize, const u32 CopyLen);

/**< Copies Len bytes from source memory to destination memory, allows
   overlapped memory between source and destination */
s32 Xil_SMemMove(void *Dest, const u32 DestSize,
		 const void *Src, const u32 SrcSize, const u32 CopyLen);

/**< Compares Len bytes between source and destination memory */
s32 Xil_SMemCmp (const void *Src1, const u32 Src1Size,
		 const void *Src2, const u32 Src2Size, const u32 CmpLen);

/**< Compares Len bytes between source and destination memory with constant time */
s32 Xil_SMemCmp_CT (const void *Src1, const u32 Src1Size,
		    const void *Src2, const u32 Src2Size, const u32 CmpLen);

/**< Sets the destination memory of given length with given data */
s32 Xil_SMemSet (void *Dest, const u32 DestSize,
		 const u8 Data, const u32 Len);

/**< Copies source string to destination string */
s32 Xil_SStrCpy (u8 *DestStr, const u32 DestSize,
		 const u8 *SrcStr, const u32 SrcSize);

/**< Compares source string with destination string */
s32 Xil_SStrCmp (const u8 *Str1, const u32 Str1Size,
		 const u8 *Str2, const u32 Str2Size);

/**< Compares source string with destination string with constant time */
s32 Xil_SStrCmp_CT (const u8 *Str1, const u32 Str1Size,
		    const u8 *Str2, const u32 Str2Size);

/**< Concatenates source string to destination string */
s32 Xil_SStrCat (u8 *DestStr, const u32 DestSize,
		 const u8 *SrcStr, const u32 SrcSize);

/**< Waits for event timeout */
u32 Xil_WaitForEventSet(u32 Timeout, u32 NumOfEvents, volatile u32 *EventAddr, ...);

/**< Implements Read Modify Writes securely */
s32 Xil_SecureRMW32(UINTPTR Addr, u32 Mask, u32 Value);

/**< Changes byte endianness of source buffer and copies it into destination */
s32 Xil_SChangeEndiannessAndCpy(void *Dest, const u32 DestSize,
		const void *Src, const u32 SrcSize, const u32 CopyLen);

/**< Changes byte endianness of buffer and copies it into the same buffer */
s32 Xil_SReverseData(void *Buf, u32 Size);

/**< Copies data from 64 bit address Src to 64 bit address Dst */
void Xil_MemCpy64(u64 DstAddr, u64 SrcAddr, u32 Cnt);

#ifdef __cplusplus
}
#endif

#endif	/* XIL_SUTIL_H_ */
/**
* @} End of "addtogroup common_utilities".
*/
