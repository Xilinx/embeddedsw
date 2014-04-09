/******************************************************************************
*
* (c) Copyright 2010-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xbram_intr.c
*
* Implements BRAM interrupt processing functions for the
* XBram driver. See xbram.h for more information
* about the driver.
*
* The functions in this file require the hardware device to be built with
* interrupt capabilities. The functions will assert if called using hardware
* that does not have interrupt capabilities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   05/11/10 Initial release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xbram.h"


/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* Enable interrupts. This function will assert if the hardware device has not
* been built with interrupt capabilities.
*
* @param	InstancePtr is the BRAM instance to operate on.
* @param	Mask is the mask to enable. Bit positions of 1 are enabled.
*		This mask is formed by OR'ing bits from XBRAM_IR*
*		bits which are contained in xbram_hw.h.
*
* @return	None.
*
* @note		None.
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
* Disable interrupts. This function allows each specific interrupt to be
* disabled. This function will assert if the hardware device has not been
* built with interrupt capabilities.
*
* @param	InstancePtr is the BRAM instance to operate on.
* @param 	Mask is the mask to disable. Bits set to 1 are disabled. This
*		mask is formed by OR'ing bits from XBRAM_IR* bits
*		which are contained in xbram_hw.h.
*
* @return	None.
*
* @note		None.
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
* Clear pending interrupts with the provided mask. This function should be
* called after the software has serviced the interrupts that are pending.
* This function will assert if the hardware device has not been built with
* interrupt capabilities.
*
* @param 	InstancePtr is the BRAM instance to operate on.
* @param 	Mask is the mask to clear pending interrupts for. Bit positions
*		of 1 are cleared. This mask is formed by OR'ing bits from
*		XBRAM_IR* bits which are contained in
*		xbram_hw.h.
*
* @return	None.
*
* @note		None.
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
* Returns the interrupt enable mask. This function will assert if the
* hardware device has not been built with interrupt capabilities.
*
* @param	InstancePtr is the BRAM instance to operate on.
*
* @return	A mask of bits made from XBRAM_IR* bits which
*		are contained in xbram_hw.h.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
u32 XBram_InterruptGetEnabled(XBram * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.CtrlBaseAddress != 0);

	return XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
				XBRAM_ECC_EN_IRQ_OFFSET);
}


/****************************************************************************/
/**
* Returns the status of interrupt signals. Any bit in the mask set to 1
* indicates that the channel associated with the bit has asserted an interrupt
* condition. This function will assert if the hardware device has not been
* built with interrupt capabilities.
*
* @param	InstancePtr is the BRAM instance to operate on.
*
* @return	A pointer to a mask of bits made from XBRAM_IR*
*		bits which are contained in xbram_hw.h.
*
* @note
*
* The interrupt status indicates the status of the device irregardless if
* the interrupts from the devices have been enabled or not through
* XBram_InterruptEnable().
*
*****************************************************************************/
u32 XBram_InterruptGetStatus(XBram * InstancePtr)
{
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(InstancePtr->Config.CtrlBaseAddress != 0);

	return XBram_ReadReg(InstancePtr->Config.CtrlBaseAddress,
				XBRAM_ECC_EN_IRQ_OFFSET);
}
