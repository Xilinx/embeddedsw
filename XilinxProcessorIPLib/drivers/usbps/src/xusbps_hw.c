/******************************************************************************
* Copyright (C) 2010 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
 *
 * @file xusbps_hw.c
* @addtogroup usbps Overview
* @{
 *
 * The implementation of the XUsbPs interface reset functionality
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.05a kpc  10/10/10 first version
 * 2.11  ka   01/29/26 Fix codespell errors.
 * 2.12  ml   09/22/26 Fixed doxygen 1.15.0 and compliance checker warnings.
 * </pre>
 *
 *****************************************************************************/

/***************************** Include Files ********************************/

#include "xstatus.h"
#include "xusbps.h"
#include "xparameters.h"


/************************** Constant Definitions ****************************/
/** @brief Maximum polling timeout for USB reset completion. */
#define XUSBPS_RESET_TIMEOUT 0xFFFFF
/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/*****************************************************************************/
/**
* This function perform the reset sequence to the given usbps interface by
* configuring the appropriate control bits in the usbps specific registers.
* the usbps reset sequence involves the below steps
* 	Disable the interrupts
*	Clear the status registers
*	Apply the reset command and wait for reset complete status
*	Update the relevant control registers with reset values
* @param[in] BaseAddress of the interface
*
******************************************************************************/
void XUsbPs_ResetHw(u32 BaseAddress)
{
	u32 RegVal;
	u32 Timeout = 0;

	/* Host and device mode */
	/* Disable the interrupts */
	XUsbPs_WriteReg(BaseAddress, XUSBPS_IER_OFFSET, 0x0);
	/* Clear the interrupt status */
	RegVal = XUsbPs_ReadReg(BaseAddress, XUSBPS_ISR_OFFSET);
	XUsbPs_WriteReg(BaseAddress, XUSBPS_ISR_OFFSET, RegVal);

	/* Perform the reset operation using USB CMD register */
	RegVal = XUsbPs_ReadReg(BaseAddress, XUSBPS_CMD_OFFSET);
	RegVal = RegVal | XUSBPS_CMD_RST_MASK;
	XUsbPs_WriteReg(BaseAddress, XUSBPS_CMD_OFFSET, RegVal);
	RegVal = XUsbPs_ReadReg(BaseAddress, XUSBPS_CMD_OFFSET);
	/* Wait till the reset operation returns success */
	/*
	 * FIX ME: right now no indication to the caller or user about
	 * timeout overflow
	 */
	while ((RegVal & XUSBPS_CMD_RST_MASK) && (Timeout < XUSBPS_RESET_TIMEOUT)) {
		RegVal = XUsbPs_ReadReg(BaseAddress, XUSBPS_CMD_OFFSET);
		Timeout++;
	}
	/* Update periodic list base address register with reset value */
	XUsbPs_WriteReg(BaseAddress, XUSBPS_LISTBASE_OFFSET, 0x0);
	/* Update async/endpoint list base address register with reset value */
	XUsbPs_WriteReg(BaseAddress, XUSBPS_ASYNCLISTADDR_OFFSET, 0x0);

}



/** @} */
