/**************************************************************************************************
* Copyright (c) 2024 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       rmv  07/16/25 Added macro for buffer index zero
 *       kp   08/19/26 Added shared PLM XilNVM IPI command macros
 *       rmv  08/20/26 Added XAsufw_IntToAscii() function prototype.
 *       rmv  08/21/26 Added macro for the maximum 64-bit address and default subsystem ID
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
#include "xasu_sharedmem.h"

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

#define XASUFW_BUFFER_INDEX_ZERO		(0U) /**< Zero index of buffer */
#define XASUFW_BUFFER_INDEX_ONE			(1U) /**< First index of buffer */
#define XASUFW_BUFFER_INDEX_TWO			(2U) /**< Second index of buffer */
#define XASUFW_BUFFER_INDEX_THREE		(3U) /**< Third index of buffer */
#define XASUFW_BUFFER_INDEX_FOUR		(4U) /**< Fourth index of buffer */

#define XASUFW_ONE_BYTE_SHIFT_VALUE		(8U) /**< One byte shift value for an integer */
#define XASUFW_TWO_BYTE_SHIFT_VALUE		(16U) /**< Two byte shift value for an integer */
#define XASUFW_THREE_BYTE_SHIFT_VALUE		(24U) /**< Three byte shift value for an integer */

#define XASUFW_LSB_MASK_VALUE			(255U) /**< Integer to octet stream primitive limit */
#define XASUFW_MAX_32BIT_ADDRESS		(0xFFFFFFFFU) /**< Max supported 32-bit address */
#define XASUFW_MAX_64BIT_ADDRESS		(0xFFFFFFFFFFFFFFFFULL)	/**< Max supported 64-bit
									address */

#define XASUFW_VALUE_ONE			(1U)	/**< Integer value 1U */
#define XASUFW_VALUE_TWO			(2U)	/**< Integer value 2U */
#define XASUFW_VALUE_THREE			(3U)	/**< Integer value 3U */

#define XASUFW_U64_IN_BYTES			(8U)	/**< U64 value in bytes */

#define XASUFW_ALLFS				(0xFFFFFFFFU)	/**< All 0xF value */
#define XASUFW_IS_ODD(x)			((x) & 1U)	/**< Check if odd */
#define XASUFW_BYTE_MASK			(0xFFU)		/**< Single byte mask */
#define XASUFW_TWO_BYTE_MASK			(0x00FFU)	/**< Lower byte mask (16-bit) */
#define XASUFW_BYTE_MULTIPLIER			(256U)		/**< Byte multiplier for endianness swap */
#define XASUFW_MAX_BIT_INDEX_IN_WORD		(31U)	/**< Maximum bit index in a 32-bit word */
#define XASUFW_BITS_IN_WORD			(32U)	/**< Number of bits in a 32-bit word */

#define XASUFW_DEFAULT_SUBSYSTEM_ID		(0x1C000000U) /**< Default subsystem ID */

/* Constants common to every PLM XilNVM IPI request raised from ASUFW. */
#define XASUFW_PLM_XILNVM_MODULE_ID		(11U)	/**< XilNVM module ID in PLM, must match
								XPLMI_MODULE_XILNVM_ID */
#define XASUFW_PLM_XILNVM_CMD_HDR_INDEX		(0U)	/**< PLM command payload index of header */
#define XASUFW_PLM_XILNVM_RESP_LEN		(1U)	/**< Response buffer length */
#define XASUFW_PLM_XILNVM_ENV_MON_CHECK		(0U)	/**< Perform environment checks */

/* PLM XilNVM off chip revoke ID write request. */
#define XASUFW_PLM_XILNVM_WRITE_OFFCHIP_REVOKE_API_ID	(16U) /**< XilNVM Write off chip
								*  revoke API ID */
#define XASUFW_PLM_XILNVM_MAX_PAYLOAD_LEN	(3U)	/**< Maximum payload length */
#define XASUFW_PLM_XILNVM_ENVCTRL_INDEX		(1U)	/**< PLM command payload index of
								environment control */
#define XASUFW_PLM_XILNVM_OFFCHID_INDEX		(2U)	/**< PLM command payload index of off chip
								ID */

/*
 * PLM XilNVM AES/USER key write request (per-key protocol).
 * The PLM expects a separate XNVM_API_ID_EFUSE_WRITE_AES_KEY IPI command per key, with a payload of
 * {KeyType, KeyAddr} pointing to the raw 256-bit key (NOT a bulk XNvm_EfuseDataAddr struct).
 */
#define XASUFW_PLM_XILNVM_WRITE_AES_KEY_API_ID	(7U)	/**< Must match
								XNVM_API_ID_EFUSE_WRITE_AES_KEY */
#define XASUFW_PLM_XILNVM_ENVDIS_KEYTYPE_INDEX	(1U)	/**< Payload index of EnvDis|KeyType */
#define XASUFW_PLM_XILNVM_KEY_ADDR_LOW_INDEX	(2U)	/**< Payload index of key addr low */
#define XASUFW_PLM_XILNVM_KEY_ADDR_HIGH_INDEX	(3U)	/**< Payload index of key addr high */
#define XASUFW_PLM_XILNVM_WRITE_PAYLOAD_LEN	(4U)	/**< AES key write payload length
								(hdr + 3) */
#define XASUFW_PLM_XILNVM_PLOAD_LEN		(3U)	/**< Payload length in words
								(hdr Len field) */
#define XASUFW_PLM_XILNVM_KEYTYPE_SHIFT		(16U)	/**< KeyType occupies bits [31:16] of arg */
#define XASUFW_NVM_EFUSE_AES_KEY_LEN_IN_WORDS	(8U)	/**< AES/USER key length in words */

/* PLM XilNVM AES key CRC check request. */
#define XASUFW_PLM_XILNVM_CHECK_CRC_API_ID	(39U)	/**< Must match the PLM command ID of
							XNVM_API_ID_EFUSE_CHECK_AES_KEY_CRC */
#define XASUFW_PLM_XILNVM_CRC_INDEX		(1U)	/**< Payload index of the key CRC */
#define XASUFW_PLM_XILNVM_CRC_KEYTYPE_INDEX	(2U)	/**< Payload index of KeyType */
#define XASUFW_PLM_XILNVM_CRC_PAYLOAD_LEN	(3U)	/**< CRC check payload length (hdr + 2) */
#define XASUFW_PLM_XILNVM_CRC_PLOAD_LEN		(2U)	/**< CRC payload length in words */
#define XASUFW_NVM_EFUSE_CRC_AES_ZEROS		(0x6858A3D5U)	/**< Must match
								XNVM_EFUSE_CRC_AES_ZEROS */

/**
 * Macro to verify command length from header matches expected parameter structure size.
 * Extracts command length from header and compares with sizeof(ParamStructType) / XASU_WORD_LEN_IN_BYTES.
 * Returns XASUFW_INVALID_PARAM error if length don't match.
 */
#define XASUFW_VERIFY_CMD_LEN(Label, Status, ReqBuf, ParamStructType) \
	if ((((ReqBuf)->Header & XASU_COMMAND_LENGTH_MASK) >> XASU_COMMAND_LENGTH_SHIFT) != \
		(u32)(sizeof(ParamStructType) / XASUFW_WORD_LEN_IN_BYTES)) { \
		Status = XASUFW_INVALID_PARAM; \
		goto Label; \
	}

/**
 * This macro provides temporal redundancy against single instruction skipping glitches
 * by calling the specified void function twice.
 */
#define XASUFW_TEMPORAL_REDUNDANT_CALL(Func, ...) \
	do { \
		Func(__VA_ARGS__); \
		Func(__VA_ARGS__); \
	} while (0)

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
s32 XAsufw_NvmEfuseWriteOffChipRevokeId(const u32 OffChipRevokeIdNum);
s32 XAsufw_SMemSet(void *Dest, const u32 DestSize);
s32 XAsufw_WriteDataToRegsWithEndianSwap(u32 BaseAddress, u32 RegOffset, const u32 *DataArray,
        u32 NumOfWords);
s32 XAsufw_AsciiToInt(const u8 *Buf, u32 Len, u32 *Value);
s32 XAsufw_IntToAscii(u8 *Buf, u32 Len, u32 Value);
u32 XAsufw_SwapBytes(const u8 *const Source, u32 Bytes);

/************************************ Variable Definitions ***************************************/
/** @cond xasufw_util_internal */
extern s32 ReturnStatus;  /**< Redundant variable holds non-zero success value helps to detect any
		glitch attacks */
/** @endcond */

#ifdef __cplusplus
}
#endif

#endif /* XASUFW_UTIL_H_ */
/** @} */
