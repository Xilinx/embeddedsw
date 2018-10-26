/******************************************************************************
*
* Copyright (C) 2017 - 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xtmr_inject_l.h
* @addtogroup tmr_inject_v1_1
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
* </pre>
*
*****************************************************************************/

#ifndef XTMR_INJECT_L_H /* prevent circular inclusions */
#define XTMR_INJECT_L_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"

/************************** Constant Definitions ****************************/

/* TMR Inject register offsets */

#define XTI_CR_OFFSET		0	/* control register, write only */
#define XTI_AIR_OFFSET		4	/* address register, write only */
#define XTI_IIR_OFFSET		8	/* instruction register, write only */
#define XTI_EAIR_OFFSET		0x10	/* address register, write only */

/* Control Register bit positions and masks */

#define XTI_CR_MAGIC		0	/* magic byte */
#define XTI_CR_CPU		8	/* cpu id */
#define XTI_CR_INJ		10	/* fault inject enabled */

#define XTI_CR_MAGIC_MASK	0x0ff	/* magic byte mask */
#define XTI_CR_CPU_MASK		0x300	/* cpu id mask */

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/*
 * Define the appropriate I/O access method to memory mapped I/O.
 */

#define XTMR_Inject_Out32 Xil_Out32
#define XTMR_Inject_Out64 Xil_Out64


/****************************************************************************/
/**
*
* Write a value to a TMRInject register. A 32 bit write is performed.
*
* @param	BaseAddress is the base address of the TMRInject device.
* @param	RegOffset is the register offset from the base to write to.
* @param	Data is the data written to the register.
*
* @return	None.
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
* @param	BaseAddress is the base address of the TMRInject device.
* @param	RegOffset is the register offset from the base to write to.
* @param	Data is the data written to the register.
*
* @return	None.
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
* @param	BaseAddress is the base address of the device
* @param	Mask is the 32-bit value to write to the control register
*
* @return	None.
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

#endif /* end of protection macro */


/** @} */
