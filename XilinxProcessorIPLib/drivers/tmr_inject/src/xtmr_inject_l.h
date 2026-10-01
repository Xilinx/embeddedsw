/******************************************************************************
* Copyright (C) 2017 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xtmr_inject_l.h
* @addtogroup Overview
* @{
*
* This header file contains identifiers and low-level driver functions (or
* macros) that can be used to access the device.  High-level driver functions
* are defined in xtmr_inject.h.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sa   04/05/17 First release
* 1.9   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
*****************************************************************************/

#ifndef XTMR_INJECT_L_H
#define XTMR_INJECT_L_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/**< TMR Inject register offsets */

#define XTI_CR_OFFSET		0	/**< Control register offset, write only */
#define XTI_AIR_OFFSET		4	/**< Address inject register offset, write only */
#define XTI_IIR_OFFSET		8	/**< Instruction inject register offset, write only */
#define XTI_EAIR_OFFSET		0x10	/**< Extended address inject register offset, write only */

/**< Control Register bit positions and masks */

#define XTI_CR_MAGIC		0	/**< Magic byte bit position in control register */
#define XTI_CR_CPU		8	/**< CPU ID bit position in control register */
#define XTI_CR_INJ		10	/**< Fault inject enable bit position in control register */

#define XTI_CR_MAGIC_MASK	0x0ff	/**< Magic byte field mask in control register */
#define XTI_CR_CPU_MASK		0x300	/**< CPU ID field mask in control register */

/** @name Instruction Inject Register masks */
/** @{ */
#define	XTI_IIR_MASK		0xffff0000 /**< Instruction inject address mask */
/** @} */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/*
 * Define the appropriate I/O access method to memory mapped I/O.
 */

#define XTMR_Inject_Out32 Xil_Out32 /**< Write a 32-bit value to a TMR Inject register */
#define XTMR_Inject_Out64 Xil_Out64 /**< Write a 64-bit value to a TMR Inject register */


/****************************************************************************/
/**
*
* Write a value to a TMRInject register. A 32 bit write is performed.
*
* @param[in]	BaseAddress is the base address of the TMRInject device.
* @param[in]	RegOffset is the register offset from the base to write to.
* @param[in]	Data is the data written to the register.
*
* @note		C-style signature:
*		void XTMR_Inject_WriteReg(u32 BaseAddress, u32 RegOffset,
*					u32 Data)
*
****************************************************************************/
#define XTMR_Inject_WriteReg(BaseAddress, RegOffset, Data) \
	XTMR_Inject_Out32((BaseAddress) + (RegOffset), (u32)(Data))


/****************************************************************************/
/**
*
* Write a value to a TMRInject register. A 64 bit write is performed.
*
* @param[in]	BaseAddress is the base address of the TMRInject device.
* @param[in]	RegOffset is the register offset from the base to write to.
* @param[in]	Data is the data written to the register.
*
* @note		C-style signature:
*		void XTMR_Inject_WriteReg64(UINTPTR BaseAddress, u32 RegOffset,
*					u64 Data)
*
****************************************************************************/
#define XTMR_Inject_WriteReg64(BaseAddress, RegOffset, Data) \
	XTMR_Inject_Out64((BaseAddress) + (RegOffset), (UINTPTR)(Data))


/****************************************************************************/
/**
*
* Set the contents of the control register. Use the XUL_CR_* constants defined
* above to create the bit-mask to be written to the register.
*
* @param[in]	BaseAddress is the base address of the device
* @param[in]	Mask is the 32-bit value to write to the control register
*
* @note		C-style Signature:
*		void XTMR_Inject_SetControlReg(u32 BaseAddress, u32 Mask);
*
*****************************************************************************/
#define XTMR_Inject_SetControlReg(BaseAddress, Mask) \
	XTMR_Inject_WriteReg((BaseAddress), XTI_CONTROL_REG_OFFSET, (Mask))


/************************** Function Prototypes *****************************/

#ifdef __cplusplus
}
#endif

#endif /**< end of protection macro */


/** @} */
