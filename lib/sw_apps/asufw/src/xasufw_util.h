/**************************************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_util.h
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
 *       yog  09/26/24 Added doxygen groupings and fixed doxygen comments.
 *       ss   10/05/24 Added XAsufw_IsBufferNonZero function.
 * 1.1   vns  02/06/25 Removed XAsufw_ChangeEndiannessAndCpy() function which is not in use
 *       am   02/21/25 Added performance measurement macros
 * 1.2   am   05/18/25 Fixed implicit conversion of operands
 *       rmv  07/09/25 Added XAsufw_AsciiToInt() function prototype and added macro for integer
 *                     value.
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
#ifndef XASUFW_UTIL_H_
#define XASUFW_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xasufw_config.h"
#include "xasufw_init.h"
#include "xasufw_debug.h"
#include "xfih.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/
#define XASU_TRUE		(TRUE) /**< Define for true */
#define XASU_FALSE		(FALSE) /**< Define for false */

/** This define is for enabling/disabling FIH variable creation */
#define FIH_VOLATILE 0
#if FIH_VOLATILE
#define CREATE_VOLATILE(x,y)		s32 x = XFih_VolatileAssign(y) /**< Create FIH variable */
#define ASSIGN_VOLATILE(x,y)		x = XFih_VolatileAssign(y) /**< Assign FIH variable */
#else
#define CREATE_VOLATILE(x,y)		volatile s32 (x) = (y) /**< Create volatile s32 variable */
#define ASSIGN_VOLATILE(x,y)		(x) = (y) /**< Assign volatile variable */
#endif

#define XASUFW_WORD_LEN_IN_BYTES		4U	/**< Word length in bytes */
#define XASUFW_BYTE_LEN_IN_BITS			8U	/**< Byte length in bits */
#define XASUFW_ARRAY_SIZE(x)	(u32)(sizeof(x) / sizeof(x[0U]))	/**< Size of array */
#define XASUFW_CONVERT_BYTES_TO_WORDS(x)	((x) / (sizeof(u32)))	/**< Converts bytes to
									     integer words */
#define XASUFW_DOUBLE_VALUE(x)			((x) << 1U)	/**< multiply the value by twice */

#define XASUFW_RESET_SET			(1U)	/**< To set the core into reset */
#define XASUFW_RESET_UNSET			(0U)	/**< To take the core out of reset */

#define XASUFW_EVEN_MODULUS			(2U)	/**< Modulus to determine evenness */

#define XASUFW_BUFFER_INDEX_ONE			(1U) /**< First index of buffer */
#define XASUFW_BUFFER_INDEX_TWO			(2U) /**< Second index of buffer */
#define XASUFW_BUFFER_INDEX_THREE		(3U) /**< Third index of buffer */
#define XASUFW_BUFFER_INDEX_FOUR		(4U) /**< Fourth index of buffer */

#define XASUFW_ONE_BYTE_SHIFT_VALUE		(8U) /**< One byte shift value for an integer */
#define XASUFW_TWO_BYTE_SHIFT_VALUE		(16U) /**< Two byte shift value for an integer */
#define XASUFW_THREE_BYTE_SHIFT_VALUE		(24U) /**< Three byte shift value for an integer */

#define XASUFW_LSB_MASK_VALUE			(255U) /**< Integer to octet stream primitive limit */
#define XASUFW_MAX_32BIT_ADDRESS		(0xFFFFFFFFU) /**< Max supported 32-bit address */

#define XASUFW_VALUE_ONE			(1U)	/**< Integer value 1U */
#define XASUFW_VALUE_TWO			(2U)	/**< Integer value 2U */

#if XASUFW_ENABLE_PERF_MEASUREMENT
#define XASUFW_MEASURE_PERF_START() \
	StartTime = XAsufw_GetTimerValue()
		/**< Capture the start time for performance measurement */
#define XASUFW_MEASURE_PERF_STOP(func_name) \
	XAsufw_MeasurePerfTime(StartTime, &PerfTime); \
	XAsufw_Printf(DEBUG_PRINT_ALWAYS, "%s execution time: %llu.%06llu us\n\r", func_name, \
		(u64)PerfTime.TPerfUs, (u64)PerfTime.TPerfUsFrac)
		/**< Measure and print execution time with the function name */
#else
#define XASUFW_MEASURE_PERF_START()
		/**< No operation when XASUFW_ENABLE_PERF_MEASUREMENT is not enabled */
#define XASUFW_MEASURE_PERF_STOP(func_name)
		/**< No operation when XASUFW_ENABLE_PERF_MEASUREMENT is not enabled */
#endif

/*************************************************************************************************/
/**
 * @brief	This function writes 32-bit value to 32-bit register.
 *
 * @param	Addr	Address of the register.
 * @param	Value	Value to store in register.
 *
 *************************************************************************************************/
static inline void XAsufw_WriteReg(u32 Addr, u32 Value)
{
	Xil_Out32(Addr, Value);
}

/*************************************************************************************************/
/**
 * @brief	This function reads a 32 bit value from a 32-bit register.
 *
 * @param	Addr	Address of the register.
 *
 * @return
 * 			- Returns 32-bit value from 32-bit register.
 *
 *************************************************************************************************/
static inline u32 XAsufw_ReadReg(u32 Addr)
{
	return Xil_In32(Addr);
}

/*************************************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a
 * 		four byte length
 *
 * @param	Integer	Variable in which input should be provided.
 * @param	Size	Holds the required size in bytes.
 * @param	Convert	Pointer in which output will be updated.
 *
 *************************************************************************************************/
static inline void XAsufw_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	Convert[Size - XASUFW_BUFFER_INDEX_FOUR] = (u8)(Integer >> XASUFW_THREE_BYTE_SHIFT_VALUE)
							& XASUFW_LSB_MASK_VALUE;
	Convert[Size - XASUFW_BUFFER_INDEX_THREE] = (u8)(Integer >> XASUFW_TWO_BYTE_SHIFT_VALUE)
							& XASUFW_LSB_MASK_VALUE;
	Convert[Size - XASUFW_BUFFER_INDEX_TWO] = (u8)(Integer >> XASUFW_ONE_BYTE_SHIFT_VALUE)
							& XASUFW_LSB_MASK_VALUE;
	Convert[Size - XASUFW_BUFFER_INDEX_ONE] = (u8)(Integer & XASUFW_LSB_MASK_VALUE);
}

#define XAsufw_SecureOut32			(Xil_SecureOut32) /**< Writes data to 32-bit address and checks
									for blind writes */

/************************************ Function Prototypes ****************************************/
void XAsufw_RMW(u32 Addr, u32 Mask, u32 Value);
void XAsufw_CryptoCoreReleaseReset(u32 BaseAddress, u32 Offset);
void XAsufw_CryptoCoreSetReset(u32 BaseAddress, u32 Offset);
s32 XAsufw_ChangeEndianness(u8 *Buffer, u32 Length);
s32 XAsufw_IsBufferNonZero(u8 *Buffer, u32 Length);
s32 XAsufw_NvmEfuseWriteOffChipRevokeId(const u32 OffChipRevokeIdNum);
s32 XAsufw_SMemSet(void *Dest, const u32 DestSize);
s32 XAsufw_WriteDataToRegsWithEndianSwap(u32 BaseAddress, u32 RegOffset, const u32 *DataArray,
        u32 NumOfWords);
u32 XAsufw_AsciiToInt(const u8 *Buf, u32 Len);

/************************************ Variable Definitions ***************************************/
extern s32 ReturnStatus;  /**< Redundant variable holds non-zero success value helps to	detect any
		glitch attacks */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_UTIL_H_ */
/** @} */
