/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_utils.h
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 5.1   kpt     08/18/24 Initial Release
*
* </pre>
*
******************************************************************************/

#ifndef XSECURE_UTILS_H_
#define XSECURE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/
#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"
#include "xstatus.h"
#include "xil_assert.h"
#include "xil_mem.h"

/************************** Constant Definitions ****************************/
#define XSECURE_RESET_SET			(1U) /**< To set the core into reset */
#define XSECURE_RESET_UNSET			(0U) /**< To take the core out of reset */
#define XSECURE_WORD_SIZE			(4U) /**< WORD size in BYTES */
#define XSECURE_QWORD_SIZE			(16U) /**< QWORD size in BYTES */
#define XSECURE_WORD_IN_BITS		(32U) /**< WORD size in BITS */
#define XSECURE_BYTE_IN_BITS		(8U)  /**< Byte size in BITS */
#define XSECURE_WORD_ALIGN_MASK			(XSECURE_WORD_SIZE - 1U)/**< WORD alignment */
#define XSECURE_SET_BIT				(0x0U) /**< To set bit */
#define XSECURE_CLEAR_BIT			(0xFFFFFFFFU) /**< To clear bit */

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XSecure_MemCpy			Xil_MemCpy /**< Backward compatibility */

/**
 * @name  Definition of asserts if macro is defined
 * @{
 */
/**< All asserts are under XSECDEBUG macro now */
#ifdef XSECDEBUG
#define XSecure_AssertVoid		(Xil_AssertVoid)
#define XSecure_AssertNonvoid		(Xil_AssertNonvoid)
/** @} */
/**
 * @name  Definition of asserts if macro is undefined
 * @{
 */
/**< Asserts if XSECDEBUG macro is undefined */
#else
#define XSecure_AssertVoid(Expression)
#define XSecure_AssertNonvoid(Expression)
#endif
/** @} */

/*****************************************************************************/
/**
 * @brief	Read from the register
 *
 * @param	BaseAddress - Contains the base address of the device
 * @param	RegOffset   - Contains the offset from the base address of the
 *			      device
 *
 * @return	The value read from the register
 *
 ******************************************************************************/
static inline u32 XSecure_ReadReg(UINTPTR BaseAddress, u16 RegOffset)
{
	return Xil_In32((BaseAddress + RegOffset));
}

/***************************************************************************/
/**
 * @brief	Write to the register
 *
 * @param	BaseAddress   - Contains the base address of the device
 * @param	RegOffset     - Contains the offset from the base address of the
 *				device
 * @param	RegisterValue - Is the value to be written to the register
 *
 ******************************************************************************/
static inline void XSecure_WriteReg(UINTPTR BaseAddress,
					u32 RegOffset, u32 RegisterValue)
{
	Xil_Out32((BaseAddress + RegOffset), RegisterValue);
}

/*****************************************************************************/
/**
 * @brief        This function reads data from 64-bit addresss
 *
 * @param        Addr is the address
 *
 ******************************************************************************/
static inline u32 XSecure_In64(u64 Addr)
{
	u32 ReadVal;

	ReadVal = (u32)Xil_In64((UINTPTR)Addr);

	return ReadVal;
}

/*****************************************************************************/
/**
 * @brief        This function reads a byte from 64-bit address
 *
 * @param        Addr is the address
 *
 ******************************************************************************/
static inline u8 XSecure_InByte64(u64 Addr)
{
	u8 ReadVal;

	ReadVal = Xil_In8((UINTPTR)Addr);

	return ReadVal;
}

/*****************************************************************************/
/**
 * @brief        This function writes data to 64-bit address
 *
 * @param        Addr is the address
 * @param        Data is the value to be written
 *
 ******************************************************************************/
static inline void XSecure_Out64(u64 Addr, u32 Data)
{
	Xil_Out64((UINTPTR)Addr, Data);
}

/*****************************************************************************/
/**
 * @brief        This function writes 32 bit data to 64-bit address
 *
 * @param        Addr is the address
 * @param        Data is the value to be written
 *
 ******************************************************************************/
static inline void XSecure_OutWord64(u64 Addr, u32 Data)
{
	Xil_Out32((UINTPTR)Addr, Data);
}

/*****************************************************************************/
/**
 * @brief        This function writes a byte to a 64-bit address
 *
 * @param        Addr is the address
 * @param        Data is the value to be written
 *
 ******************************************************************************/
static inline void XSecure_OutByte64(u64 Addr, u8 Data)
{
	Xil_Out8((UINTPTR)Addr, Data);
}

#define XSecure_In32		(Xil_In32)		/**< Reads data from 32-bit address */
#define XSecure_Out32		(Xil_Out32)		/**< Writes data to 32-bit address */
#define XSecure_SecureOut32	(Xil_SecureOut32)/**< Writes data to 32-bit address
												and checks for blind writes*/

/************************** Function Prototypes ******************************/
void XSecure_SetReset(UINTPTR BaseAddress, u32 Offset);
void XSecure_ReleaseReset(UINTPTR BaseAddress, u32 Offset);
void XSecure_MemCpy64(u64 DstAddr, u64 SrcAddr, u32 Cnt);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_UTILS_H_ */
