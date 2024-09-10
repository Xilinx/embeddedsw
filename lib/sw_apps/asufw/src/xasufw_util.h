/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_util.h
 * @addtogroup Overview
 * @{
 *
 * This is the header file for ASUFW utilities code.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   03/29/24 Initial release
 *       ma   04/18/24 Added macro for array size calculation
 *       ma   05/20/24 Rename XASUFW_WORD_LEN macro and add XASUFW_BYTE_LEN_IN_BITS macro
 *       am   06/26/24 Added Reset set, unset and XASUFW_CONVERT_BYTES_TO_WORDS macros
 *       ss   07/11/24 Added XAsufw_ChangeEndiannessAndCpy function
 *
 * </pre>
 *
 *************************************************************************************************/

#ifndef XASUFW_UTIL_H_
#define XASUFW_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_io.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASUFW_WORD_LEN_IN_BYTES		4U	/**< Word length in bytes */
#define XASUFW_BYTE_LEN_IN_BITS			8U	/** < Byte length in bits */
#define XASUFW_ARRAY_SIZE(x)	(u32)(sizeof(x) / sizeof(x[0U]))	/**< Size of array */
#define XASUFW_CONVERT_BYTES_TO_WORDS(x)	((x) / (sizeof(int)))	/**< Converts bytes to
									     integer words */

#define XASUFW_RESET_SET			(1U)	/**< To set the core into reset */
#define XASUFW_RESET_UNSET			(0U)	/**< To take the core out of reset */

#define XASUFW_EVEN_MODULUS			(2U)	/**< Modulus to determine evenness */

/*************************************************************************************************/
/**
 * @brief	This function writes 32-bit value to 32-bit register
 *
 * @param	Addr	Address of the register
 * @param	Data	Value to store in register
 *
 *************************************************************************************************/
static inline void XAsufw_WriteReg(u32 Addr, u32 Value)
{
	Xil_Out32(Addr, Value);
}

/*************************************************************************************************/
/**
 * @brief	This function reads a 32 bit value from a 32-bit register
 *
 * @param	Addr	Address of the register
 *
 * @return
 * 			- Returns 32-bit value from 32-bit register
 *
 *************************************************************************************************/
static inline u32 XAsufw_ReadReg(u32 Addr)
{
	return Xil_In32(Addr);
}

/************************************ Function Prototypes ****************************************/
void XAsufw_RMW(u32 Addr, u32 Mask, u32 Value);
void XAsufw_RCMW(u32 Addr, u32 Mask, u32 Value);
void XAsufw_CryptoCoreReleaseReset(u32 BaseAddress, u32 Offset);
void XAsufw_CryptoCoreSetReset(u32 BaseAddress, u32 Offset);
s32 XAsufw_ChangeEndiannessAndCpy(void *Dest, const u32 DestSize, const void *Src,
				  const u32 SrcSize,
				  const u32 CopyLen);
s32 XAsufw_ChangeEndianness(u8 *Buffer, u32 Length);

/************************************ Variable Definitions ***************************************/

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_UTIL_H_ */
