/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
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
* 4.0   vns     03/12/19 Initial Release
* 4.1   kal     05/20/19 Updated doxygen tags
*       psl     08/05/19 Fixed MISRA-C violation
* 4.2   har     01/06/20 Added macro XSecure_Out32
*       kpt     01/07/20 Added Macro XSECURE_WORD_SIZE common for
*                        both AES and RSA
*       rpo     04/02/20 Replaced function like macros with inline functions
*                        Redefined macros for reading and writing into registers
*       har     04/13/20 Removed code for SSS configuration
* 4.3   rpo     09/01/20 Asserts are not compiled by default for
*                        secure libraries
*       am      09/24/20 Resolved MISRA C violations
*       har     10/12/20 Addressed security review comments
*       am      10/10/20 Resolved Coverity warning
* 4.5   am      11/24/20 Resolved MISRA C violations
*       bm      01/13/21 Added 64 bit In and Out apis
*       har     02/01/21 Removed Status variable from XSecure_ReadReg
*       bm      05/19/21 Added macro for word aligned mask
* 4.6   har     07/14/21 Fixed doxygen warnings
*       gm      07/16/21 Support added to read 32 bit data from 64bit address
*       har     08/14/21 Added macro for QWord
*       am      09/17/21 Resolved compiler warnings
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
#define XSECURE_RESET_SET		(1U) /**< To set the core into reset */
#define XSECURE_RESET_UNSET		(0U) /**< To take the core out of reset */
#define XSECURE_WORD_SIZE		(4U) /**< WORD size in BYTES */
#define XSECURE_QWORD_SIZE		(16U) /**< QWORD size in BYTES */
#define XSECURE_WORD_IN_BITS		(32U)/**< WORD size in BITS */
#define XSECURE_WORD_ALIGN_MASK		(XSECURE_WORD_SIZE - 1U)/**< WORD alignment */

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
#define XSecure_AssertVoidAlways	(Xil_AssertVoidAlways)
#define XSecure_AssertNonvoid		(Xil_AssertNonvoid)
#define XSecure_AssertNonvoidAlways	(Xil_AssertNonvoidAlways)
/** @} */
/**
 * @name  Definition of asserts if macro is undefined
 * @{
 */
/**< Asserts if XSECDEBUG macro is undefined */
#else
#define XSecure_AssertVoid(Expression)
#define XSecure_AssertVoidAlways()
#define XSecure_AssertNonvoid(Expression)
#define XSecure_AssertNonvoidAlways()
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
static inline u32 XSecure_ReadReg(u32 BaseAddress, u16 RegOffset)
{
	return Xil_In32((UINTPTR)(BaseAddress + RegOffset));
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
static inline void XSecure_WriteReg(u32 BaseAddress,
					u32 RegOffset, u32 RegisterValue)
{
	Xil_Out32((UINTPTR)(BaseAddress + RegOffset), RegisterValue);
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
#ifdef VERSAL_PLM
	ReadVal = lwea(Addr);
#else
	ReadVal = (u32)Xil_In64((UINTPTR)Addr);
#endif
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
#ifdef VERSAL_PLM
	ReadVal = (u8)lbuea(Addr);
#else
	ReadVal = Xil_In8((UINTPTR)Addr);
#endif
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
#ifdef VERSAL_PLM
	swea(Addr, Data);
#else
	Xil_Out64((UINTPTR)Addr, Data);
#endif
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
#ifdef VERSAL_PLM
	swea(Addr, Data);
#else
	Xil_Out32((UINTPTR)Addr, Data);
#endif
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
#ifdef VERSAL_PLM
	sbea(Addr, Data);
#else
	Xil_Out8((UINTPTR)Addr, Data);
#endif
}

#define XSecure_In32		(Xil_In32)		/**< Reads data from 32-bit address */
#define XSecure_Out32		(Xil_Out32)		/**< Writes data to 32-bit address */
#define XSecure_SecureOut32	(Xil_SecureOut32)/**< Writes data to 32-bit address
												and checks for blind writes*/

/************************** Function Prototypes ******************************/
void XSecure_SetReset(u32 BaseAddress, u32 Offset);
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset);
void XSecure_MemCpy64(u64 DstAddr, u64 SrcAddr, u32 Cnt);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_UTILS_H_ */
