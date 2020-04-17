/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_utils.h
* @addtogroup xsecure_common_apis XILSECURE_UTILITIES
* @{
* @cond xsecure_internal
* This file contains common APIs which are used across the library.
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
*       har     03/26/20 Removed code for SSS configuration
* </pre>
* @endcond
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
#define XSECURE_RESET_UNSET		(0U)
					/**< To take the core out of reset */

#define XSECURE_TIMEOUT_MAX		(0x1FFFFFU)
#define XSECURE_WORD_SIZE		(4U) /**< WORD size in BYTES */
#define XSECURE_WORD_IN_BITS	(32U)/**< WORD size in BITS	 */

/***************************** Type Definitions******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/* Backward compatibility */
#define XSecure_MemCpy		Xil_MemCpy

/*****************************************************************************/
/**
* Read from the register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSecure_ReadReg(u32 BaseAddress, u16 RegOffset)
*
******************************************************************************/
static inline u32 XSecure_ReadReg(u32 BaseAddress, u16 RegOffset)
{
	u32 Status;
	Status = Xil_In32(BaseAddress + RegOffset);
	return Status;
}
/***************************************************************************/
/**
* Write to the register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
* @param	RegisterValue is the value to be written to the register
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_WriteReg(u32 BaseAddress, u16 RegOffset,
*			u16 RegisterValue)
*
******************************************************************************/
static inline void XSecure_WriteReg(u32 BaseAddress,
									u32 RegOffset, u32 RegisterValue)
{
	Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue));
}

#define XSecure_In32			Xil_In32

#define XSecure_In64			Xil_In64

#define XSecure_Out32			Xil_Out32

#define XSecure_Out64			Xil_Out64

#define XSecure_SecureOut32		Xil_SecureOut32

/************************** Function Prototypes ******************************/

void XSecure_SetReset(u32 BaseAddress, u32 Offset);
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_UTILS_H_ */
/**@}*/
