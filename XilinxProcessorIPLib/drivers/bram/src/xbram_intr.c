/******************************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xbram_intr.c
* @brief Implements XBram interrupt processing functions.
*
* Implements BRAM interrupt processing functions for the
* XBram driver. See xbram.h for more information
* about the driver.
*
* The functions in this file require the hardware device to be built with
* interrupt capabilities. The functions assert if called using hardware
* that does not have interrupt capabilities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   05/11/10 Initial release
* 4.15  vn   07/30/26 Fix Doxygen warnings
* 4.15  ss   08/24/26 Fix interrupt status register read
* 4.15  tb  09/15/26 Align Doxygen comments with AMD standards.
* </pre>
*
*****************************************************************************/

/**
* @addtogroup bram_api BRAM APIs
* @{
*/

/***************************** Include Files ********************************/
#include "xbram.h"


/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* @brief Enables selected BRAM interrupts.
*
* This function asserts if the hardware device has not been built with
* interrupt capabilities.
*
* @param[in] InstancePtr BRAM instance to operate on.
* @param[in] Mask Mask to enable. Bit positions of 1 are enabled.
*   This mask is formed by OR'ing bits from XBRAM_IR* bits which
*   are contained in xbram_hw.h.
*
*****************************************************************************/
void XBram_InterruptEnable(XBram *InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.CtrlBaseAddress != 0);

	/*
	 * Read the interrupt enable register and only enable the specified
	 * interrupts without disabling or enabling any others.
	 */
	Register = XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
				 XBRAM_ECC_EN_IRQ_OFFSET);
	XBram_WriteReg(InstancePtr->Config.CtrlBaseAddress,
		       XBRAM_ECC_EN_IRQ_OFFSET,
		       Register | Mask);
}


/****************************************************************************/
/**
* @brief Disables selected BRAM interrupts.
*
* This function allows each specific interrupt to be disabled. This function
* asserts if the hardware device has not been built with interrupt
* capabilities.
*
* @param[in] InstancePtr BRAM instance to operate on.
* @param[in] Mask Mask to disable. Bits set to 1 are disabled. This
*   mask is formed by OR'ing bits from XBRAM_IR* bits which are
*   contained in xbram_hw.h.
*
*****************************************************************************/
void XBram_InterruptDisable(XBram *InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.CtrlBaseAddress != 0);

	/*
	 * Read the interrupt enable register and only disable the specified
	 * interrupts without enabling or disabling any others.
	 */
	Register = XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
				 XBRAM_ECC_EN_IRQ_OFFSET);
	XBram_WriteReg(InstancePtr->Config.CtrlBaseAddress,
		       XBRAM_ECC_EN_IRQ_OFFSET,
		       Register & (~Mask));
}

/****************************************************************************/
/**
* @brief Clears pending BRAM interrupts for the provided mask.
*
* Call this function after the software has serviced the pending interrupts.
* This function asserts if the hardware device has not been built with
* interrupt capabilities.
*
* @param[in] InstancePtr BRAM instance to operate on.
* @param[in] Mask Mask to clear pending interrupts for. Bit
*   positions of 1 are cleared. This mask is formed by OR'ing bits
*   from XBRAM_IR* bits which are contained in xbram_hw.h.
*
*****************************************************************************/
void XBram_InterruptClear(XBram *InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->Config.CtrlBaseAddress != 0);

	/*
	 * Read the interrupt status register and only clear the interrupts
	 * that are specified without affecting any others.  Since the register
	 * is a toggle on write, make sure any bits to be written are already
	 * set.
	 */
	Register = XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
				 XBRAM_ECC_STATUS_OFFSET);
	XBram_WriteReg(InstancePtr->Config.CtrlBaseAddress,
		       XBRAM_ECC_STATUS_OFFSET,
		       Register & Mask);


}


/****************************************************************************/
/**
* @brief Returns the BRAM interrupt enable mask.
*
* This function asserts if the hardware device has not been built with
* interrupt capabilities.
*
* @param[in] InstancePtr BRAM instance to operate on.
*
* @return A mask of bits made from XBRAM_IR* bits which are contained in
*         xbram_hw.h.
*
*****************************************************************************/
u32 XBram_InterruptGetEnabled(XBram *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.CtrlBaseAddress != 0);

	return XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
			     XBRAM_ECC_EN_IRQ_OFFSET);
}


/****************************************************************************/
/**
* @brief Returns the status of BRAM interrupt signals.
*
* Any bit in the mask set to 1 indicates that the channel associated with
* the bit has asserted an interrupt condition. This function asserts if the
* hardware device has not been built with interrupt capabilities.
*
* @param[in] InstancePtr BRAM instance to operate on.
*
* @return A mask of bits made from XBRAM_IR* bits which are contained in
*         xbram_hw.h.
*
* @note The interrupt status indicates the status of the device regardless
*       of whether interrupts have been enabled through
*       XBram_InterruptEnable().
*
*****************************************************************************/
u32 XBram_InterruptGetStatus(XBram *InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.CtrlBaseAddress != 0);

	return XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
			     XBRAM_ECC_STATUS_OFFSET);
}
/** @} */
