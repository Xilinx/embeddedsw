/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
#define XSECURE_WORD_IN_BITS		(32U)/**< WORD size in BITS */

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Backward compatibility */
#define XSecure_MemCpy			Xil_MemCpy

#ifdef XSECDEBUG
/* All asserts are under XSECDEBUG macro now */
#define XSecure_AssertVoid		(Xil_AssertVoid)
#define XSecure_AssertVoidAlways	(Xil_AssertVoidAlways)
#define XSecure_AssertNonvoid		(Xil_AssertNonvoid)
#define XSecure_AssertNonvoidAlways	(Xil_AssertNonvoidAlways)
#else
#define XSecure_AssertVoid(Expression)
#define XSecure_AssertVoidAlways()
#define XSecure_AssertNonvoid(Expression)
#define XSecure_AssertNonvoidAlways()
#endif

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
	u32 Status = (u32)XST_FAILURE;

	Status = Xil_In32(BaseAddress + RegOffset);

	return Status;
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
 * @return	None
 *
 *
 ******************************************************************************/
static inline void XSecure_WriteReg(u32 BaseAddress,
					u32 RegOffset, u32 RegisterValue)
{
	Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue));
}

#define XSecure_In32		(Xil_In32)
#define XSecure_In64		(Xil_In64)
#define XSecure_Out32		(Xil_Out32)
#define XSecure_Out64		(Xil_Out64)
#define XSecure_SecureOut32	(Xil_SecureOut32)

/************************** Function Prototypes ******************************/
void XSecure_SetReset(u32 BaseAddress, u32 Offset);
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_UTILS_H_ */