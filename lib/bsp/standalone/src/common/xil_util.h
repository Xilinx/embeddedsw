/******************************************************************************/
/**
* Copyright (c) 2019 - 2020  Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_util.h
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

/******************************************************************************/
/**
* This API ceils the provided float value.
*
* @param	Value is a float variable which has to ceiled to nearest
*		integer.
*
* @return	Returns ceiled value.
*
* @note		one.
*
*******************************************************************************/
#define Xil_Ceil(Value) \
	(((Value > (u32)Value) || ((u32)Value == 0U)) ? \
					(u32)((u32)Value + 1U) : (u32)Value)


/*************************** Function Prototypes ******************************/

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
u32 Xil_IsValidHexChar(const char Ch);

/* Validate the input string contains only hexadecimal characters */
u32 Xil_ValidateHexStr(const char *HexStr);

/* Convert string to hex numbers in little enidian format */
u32 Xil_ConvertStringToHexLE(const char *Str, u8 *Buf, u32 Len);

/* Returns length of the input string */
u32 Xil_Strnlen(const char *Str, u32 MaxLen);

/* Convert string to hex numbers in big endian format */
u32 Xil_ConvertStringToHexBE(const char * Str, u8 * Buf, u32 Len);

#ifdef __cplusplus
}
#endif

#endif	/* XIL_UTIL_H_ */
