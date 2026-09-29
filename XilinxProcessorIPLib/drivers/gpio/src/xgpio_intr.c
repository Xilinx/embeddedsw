/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_intr.c
* @brief Implements GPIO interrupt processing functions for the XGpio driver.
*
* See xgpio.h for more information about the driver.
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
* 2.00a jhl  11/26/03 Initial release
* 2.11a mta  03/21/07 Updated to new coding style
* 2.12a sv   06/05/08 Updated driver to fix the XGpio_InterruptDisable function
*		      to properly update the Interrupt Enable register
* 3.00a sv   11/21/09 Updated to use HAL Processor APIs. Renamed the macros
*		      XGpio_mWriteReg to XGpio_WriteReg, and XGpio_mReadReg
*		      to XGpio_ReadReg.
* 4.14  vn   07/30/26 Fix Doxygen warnings
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/** @addtogroup gpio_api GPIO APIs
 * @{
 */

/****************************** Include Files ********************************/
#include "xgpio.h"


/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/


/*****************************************************************************/
/**
* @brief Enables the interrupt output signal.
*
* Interrupts enabled through XGpio_InterruptEnable() are not passed
* through until the global enable bit is set by this function. This
* function is designed to allow all interrupts (both channels) to be
* enabled easily for exiting a critical section. This function asserts if
* the hardware device has not been built with interrupt capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
*
******************************************************************************/
void XGpio_InterruptGlobalEnable(XGpio *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	XGpio_WriteReg(InstancePtr->BaseAddress, XGPIO_GIE_OFFSET,
			XGPIO_GIE_GINTR_ENABLE_MASK);
}


/*****************************************************************************/
/**
* @brief Disables the interrupt output signal.
*
* Interrupts enabled through XGpio_InterruptEnable() are no longer passed
* through until the global enable bit is set by XGpio_InterruptGlobalEnable().
* This function is designed to allow all interrupts (both channels) to be
* disabled easily for entering a critical section. This function asserts
* if the hardware device has not been built with interrupt capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
*
*****************************************************************************/
void XGpio_InterruptGlobalDisable(XGpio *InstancePtr)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);


	XGpio_WriteReg(InstancePtr->BaseAddress, XGPIO_GIE_OFFSET, 0x0);

}


/****************************************************************************/
/**
* @brief Enables interrupts for the specified channels.
*
* Also call XGpio_InterruptGlobalEnable() to enable the global interrupt;
* otherwise, no interrupts occur. This function
* asserts if the hardware device has not been built with interrupt
* capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   Mask Mask to enable. Bit positions of 1 are enabled.
*              This mask is a bitwise OR of XGPIO_IR* bits, which are
*              contained in xgpio_l.h.
*
*****************************************************************************/
void XGpio_InterruptEnable(XGpio *InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	/*
	 * Read the interrupt enable register and only enable the specified
	 * interrupts without disabling or enabling any others.
	 */

	Register = XGpio_ReadReg(InstancePtr->BaseAddress, XGPIO_IER_OFFSET);
	XGpio_WriteReg(InstancePtr->BaseAddress, XGPIO_IER_OFFSET,
			Register | Mask);

}


/****************************************************************************/
/**
* @brief Disables interrupts for the specified channels.
*
* This function disables specific interrupts for each channel. This function
* asserts if the hardware device has not been built with interrupt
* capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   Mask Mask to disable. Bits set to 1 are disabled. This
*              mask is a bitwise OR of XGPIO_IR* bits, which are contained
*              in xgpio_l.h.
*
*****************************************************************************/
void XGpio_InterruptDisable(XGpio *InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	/*
	 * Read the interrupt enable register and only disable the specified
	 * interrupts without enabling or disabling any others.
	 */
	Register = XGpio_ReadReg(InstancePtr->BaseAddress, XGPIO_IER_OFFSET);
	XGpio_WriteReg(InstancePtr->BaseAddress, XGPIO_IER_OFFSET,
			Register & (~Mask));

}

/****************************************************************************/
/**
* @brief Clears pending interrupts with the provided mask.
*
* Call this function after the software has serviced the
* interrupts that are pending. This function asserts if the hardware
* device has not been built with interrupt capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   Mask Mask to clear pending interrupts. Bit positions
*              of 1 are cleared. This mask is a bitwise OR of XGPIO_IR*
*              bits, which are contained in xgpio_l.h.
*
******************************************************************************/
void XGpio_InterruptClear(XGpio * InstancePtr, u32 Mask)
{
	u32 Register;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(InstancePtr->InterruptPresent == TRUE);

	/*
	 * Read the interrupt status register and only clear the interrupts
	 * that are specified without affecting any others.  Since the register
	 * is a toggle on write, make sure any bits to be written are already
	 * set.
	 */
	Register = XGpio_ReadReg(InstancePtr->BaseAddress, XGPIO_ISR_OFFSET);
	XGpio_WriteReg(InstancePtr->BaseAddress, XGPIO_ISR_OFFSET,
			Register & Mask);


}


/*****************************************************************************/
/**
* @brief Returns the interrupt enable mask.
*
* This function asserts if the hardware device has not been built with
* interrupt capabilities.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
*
* @return      A mask of bits made from XGPIO_IR* bits which are contained in
*              xgpio_l.h.
*
******************************************************************************/
u32 XGpio_InterruptGetEnabled(XGpio * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);

	return XGpio_ReadReg(InstancePtr->BaseAddress, XGPIO_IER_OFFSET);
}


/*****************************************************************************/
/**
* @brief Returns the status of interrupt signals.
*
* Any bit in the mask set to 1 indicates that the channel associated with
* the bit has asserted an interrupt condition. This function asserts if
* the hardware device has not been built with interrupt capabilities.
*
* The interrupt status indicates the status of the device regardless of
* whether the interrupts from the devices have been enabled or not through
* XGpio_InterruptEnable().
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
*
* @return      Mask of bits made from XGPIO_IR* bits which are contained in
*              xgpio_l.h.
*
******************************************************************************/
u32 XGpio_InterruptGetStatus(XGpio * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->InterruptPresent == TRUE);


	return XGpio_ReadReg(InstancePtr->BaseAddress, XGPIO_ISR_OFFSET);
}
/** @} */
