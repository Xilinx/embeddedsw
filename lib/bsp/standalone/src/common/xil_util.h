/******************************************************************************/
/**
* Copyright (c) 2019 - 2020  Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_util.h
* @addtogroup common_utilities Common Utility APIs
* @{
* @details
*
* This file contains xil utility functions declaration
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.4   mmd      04/21/19 First release.
* 6.5   kal      02/29/20 Added Xil_ConvertStringToHexBE API
* 7.3   kal      06/30/20 Converted Xil_Ceil macro to API
*		rpo      08/19/20 Added function for read, modify and write
*       bsv      08/21/20 Added XSECURE_TEMPORAL_CHECK macro to add
*                         redundancy in security critical functions, to avoid
*                         glitches from altering the return values of security
*                         critical functions. The macro requires a label to be
*                         passed to "go to" in case of error.
*      kpt       09/03/20 Added XSECURE_TEMPORAL_IMPL macro for redundancy
*      kal       09/22/20 Changed the param type from const char to const char*
*				  to avoid copying key onto stack
*      td	 10/16/20 Added Xil_Strcpy, Xil_Strcat, Xil_SecureMemCpy and
* 				  Xil_MemCmp functions
*      am        10/13/20 Resolved Coverity warning
*
* </pre>
*
*****************************************************************************/

#ifndef XIL_UTIL_H_
#define XIL_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"
#include "xstatus.h"

/*************************** Constant Definitions *****************************/
#define XIL_SIZE_OF_NIBBLE_IN_BITS	4U
#define XIL_SIZE_OF_BYTE_IN_BITS	8U

/* Maximum string length handled by Xil_ValidateHexStr function */
#define XIL_MAX_HEX_STR_LEN	512U


/****************** Macros (Inline Functions) Definitions *********************/
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
		({ \
			Var = XST_FAILURE; \
			VarTmp = XST_FAILURE; \
			Var = Function(__VA_ARGS__); \
			VarTmp = Var; \
		})

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
	({ \
		volatile int StatusTmp = XST_FAILURE; \
		XSECURE_TEMPORAL_IMPL(Status, StatusTmp, Function, __VA_ARGS__); \
		if ((Status != XST_SUCCESS) || \
			(StatusTmp != XST_SUCCESS)) { \
			Status |= StatusTmp;\
			goto Label; \
		} \
	 })
#endif

/*************************** Function Prototypes ******************************/
/* Ceils the provided float value */
int Xil_Ceil(float Value);

/* Converts input character to nibble */
u32 Xil_ConvertCharToNibble(u8 InChar, u8 *Num);

/* Convert input hex string to array of 32-bits integers */
u32 Xil_ConvertStringToHex(const char *Str, u32 *buf, u8 Len);

/* Waits for specified event */
u32 Xil_WaitForEvent(u32 RegAddr, u32 EventMask, u32 Event, u32 Timeout);

/* Waits for specified events */
u32 Xil_WaitForEvents(u32 EventsRegAddr, u32 EventsMask, u32 WaitEvents,
			 u32 Timeout, u32* Events);

/* Validate input hex character */
u32 Xil_IsValidHexChar(const char *Ch);

/* Validate the input string contains only hexadecimal characters */
u32 Xil_ValidateHexStr(const char *HexStr);

/* Convert string to hex numbers in little enidian format */
u32 Xil_ConvertStringToHexLE(const char *Str, u8 *Buf, u32 Len);

/* Returns length of the input string */
u32 Xil_Strnlen(const char *Str, u32 MaxLen);

/* Convert string to hex numbers in big endian format */
u32 Xil_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);

/*Read, Modify and Write to an address*/
void Xil_UtilRMW32(u32 Addr, u32 Mask, u32 Value);

/* Copies source string to destination string */
int Xil_Strcpy(char *DestPtr, const char *SrcPtr, const u32 Size);

/* Appends string2 to string1 */
int Xil_Strcat(char* Str1Ptr, const char* Str2Ptr, const u32 Size);

/* Copies Len bytes from source memory to destination memory */
int Xil_SecureMemCpy(void * DestPtr, u32 DestPtrLen, const void * SrcPtr, u32 Len);

/* Compares Len bytes from memory1 and memory2 */
int Xil_MemCmp(const void * Buf1Ptr, const void * Buf2Ptr, u32 Len);

#ifdef __cplusplus
}
#endif

#endif	/* XIL_UTIL_H_ */
/**
* @} End of "addtogroup common_utilities".
*/
