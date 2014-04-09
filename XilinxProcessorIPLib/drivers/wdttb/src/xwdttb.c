/* $Id: xwdttb.c,v 1.1.2.1 2009/12/04 05:52:37 svemula Exp $ */
/******************************************************************************
*
* (c) Copyright 2001-2009 Xilinx, Inc. All rights reserved.
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
*
* @file xwdttb.c
*
* Contains the required functions of the XWdtTb driver. See xwdttb.h for a
* description of the driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a ecm  08/16/01 First release
* 1.00b jhl  02/21/02 Repartitioned the driver for smaller files
* 1.00b rpm  04/26/02 Made LookupConfig public
* 1.10b mta  03/23/07 Updated to new coding style
* 2.00a ktn  10/22/09 Updated to use the HAL processor APIs/macros.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_types.h"
#include "xil_assert.h"
#include "xparameters.h"
#include "xil_io.h"
#include "xwdttb.h"


/************************** Constant Definitions *****************************/


/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

extern XWdtTb_Config XWdtTb_ConfigTable[];

/****************************************************************************/
/**
*
* Initialize a specific watchdog timer/timebase instance/driver. This function
* must be called before other functions of the driver are called.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
* @param	DeviceId is the unique id of the device controlled by this
*		XWdtTb instance. Passing in a device id associates the generic
*		XWdtTb instance to a specific device, as chosen by the caller or
*		application developer.
*
* @return
* 		- XST_SUCCESS if initialization was successful
* 		- XST_DEVICE_IS_STARTED if the device has already been started
* 		- XST_DEVICE_NOT_FOUND if the configuration for device ID was
*		not found
*
* @note		None.
*
******************************************************************************/
int XWdtTb_Initialize(XWdtTb *InstancePtr, u16 DeviceId)
{
	XWdtTb_Config *ConfigPtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * If the device is started, disallow the initialize and return a status
	 * indicating it is started.  This allows the user to stop the device
	 * and reinitialize, but prevents a user from inadvertently initializing
	 */
	if (InstancePtr->IsStarted == XIL_COMPONENT_IS_STARTED) {
		return XST_DEVICE_IS_STARTED;
	}

	/*
	 * Lookup the device configuration. Use this info when initializing
	 * the driver
	 */
	ConfigPtr = XWdtTb_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_DEVICE_NOT_FOUND;
	}

	/*
	 * Save the base address pointer such that the registers of the block
	 * can be accessed and indicate it has not been started yet
	 */
	InstancePtr->RegBaseAddress = ConfigPtr->BaseAddr;
	InstancePtr->IsStarted = 0;

	/*
	 * Indicate the instance is ready to use, successfully initialized
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Start the watchdog timer of the device.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return	None.
*
* @note		The Timebase is reset to 0 when the Watchdog Timer is started.
*		The Timebase is always incrementing
*
******************************************************************************/
void XWdtTb_Start(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the current contents of TCSR0 so that subsequent writes to the
	 * register won't destroy any other bits
	 */
	ControlStatusRegister0 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress,
					  XWT_TWCSR0_OFFSET);
	/*
	 * Clear the bit that indicates the reason for the last
	 * system reset, WRS and the WDS bit, if set, by writing
	 * 1's to TCSR0
	 */
	ControlStatusRegister0 |= (XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK);

	/*
	 * Indicate that the device is started before we enable it
	 */
	InstancePtr->IsStarted = XIL_COMPONENT_IS_STARTED;

	/*
	 * Set the registers to enable the watchdog timer, both enable bits
	 * in TCSR0 and TCSR1 need to be set to enable it
	 */
	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR0_OFFSET,
		  (ControlStatusRegister0 | XWT_CSR0_EWDT1_MASK));

	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR1_OFFSET,
		  XWT_CSRX_EWDT2_MASK);

}

/****************************************************************************/
/**
*
* Disable the watchdog timer.
*
* It is the caller's responsibility to disconnect the interrupt handler
* of the watchdog timer from the interrupt source, typically an interrupt
* controller, and disable the interrupt in the interrupt controller.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return
* 		- XST_SUCCESS if the watchdog was stopped successfully
* 		- XST_NO_FEATURE if the watchdog cannot be stopped
*
* @note
*
* The hardware configuration controls this functionality. If it is not
* allowed by the hardware the failure will be returned and the timer
* will continue without interruption.
*
******************************************************************************/
int XWdtTb_Stop(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Check if the disable of the watchdog timer is possible by writing a 0
	 * to TCSR1 to clear the 2nd enable. If the Enable does not clear in
	 * TCSR0, the watchdog cannot be disabled. Return a NO_FEATURE to
	 * indicate this.
	 */
	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR1_OFFSET, 0);

	/*
	 * Read the contents of TCSR0 so that the writes to the register
	 * that follow are not destructive to other bits and to check if
	 * the second enable was set to zero
	 */
	ControlStatusRegister0 =
		XWdtTb_ReadReg(InstancePtr->RegBaseAddress, XWT_TWCSR0_OFFSET);

	/*
	 * If the second enable was not set to zero, the feature is not
	 * allowed in the hardware. Return with NO_FEATURE status
	 */
	if ((ControlStatusRegister0 & XWT_CSRX_EWDT2_MASK) != 0) {
		return XST_NO_FEATURE;
	}

	/*
	 * Disable the watchdog timer by performing 2 writes, 1st to
	 * TCSR0 to clear the enable 1 and then to TCSR1 to clear the 2nd enable
	 */
	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR0_OFFSET,
		  (ControlStatusRegister0 & ~XWT_CSR0_EWDT1_MASK));

	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR1_OFFSET, 0);

	InstancePtr->IsStarted = 0;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Check if the watchdog timer has expired. This function is used for polled
* mode and it is also used to check if the last reset was caused by the
* watchdog timer.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return	TRUE if the watchdog has expired, and FALSE otherwise.
*
* @note		None.
*
******************************************************************************/
int XWdtTb_IsWdtExpired(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;
	u32 Mask;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the current contents
	 */
	ControlStatusRegister0 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress,
					  XWT_TWCSR0_OFFSET);
	/*
	 * The watchdog has expired if either of the bits are set
	 */
	Mask = XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK;

	return ((ControlStatusRegister0 & Mask) != 0);
}

/****************************************************************************/
/**
*
* Restart the watchdog timer. An application needs to call this function
* periodically to keep the timer from asserting the reset output.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XWdtTb_RestartWdt(XWdtTb *InstancePtr)
{
	u32 ControlStatusRegister0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read the current contents of TCSR0 so that subsequent writes won't
	 * destroy any other bits
	 */
	ControlStatusRegister0 = XWdtTb_ReadReg(InstancePtr->RegBaseAddress,
					  XWT_TWCSR0_OFFSET);
	/*
	 * Clear the bit that indicates the reason for the last
	 * system reset, WRS and the WDS bit, if set, by writing
	 * 1's to TCSR0
	 */
	ControlStatusRegister0 |= (XWT_CSR0_WRS_MASK | XWT_CSR0_WDS_MASK);

	XWdtTb_WriteReg(InstancePtr->RegBaseAddress, XWT_TWCSR0_OFFSET,
		  ControlStatusRegister0);
}

/****************************************************************************/
/**
*
* Returns the current contents of the timebase.
*
* @param	InstancePtr is a pointer to the XWdtTb instance to be worked on.
*
* @return	The contents of the timebase.
*
* @note		None.
*
******************************************************************************/
u32 XWdtTb_GetTbValue(XWdtTb *InstancePtr)
{
	u32 CurrentTBR;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Return the contents of the timebase register
	 */
	CurrentTBR = XWdtTb_ReadReg(InstancePtr->RegBaseAddress,
					XWT_TBR_OFFSET);

	return CurrentTBR;
}

/****************************************************************************
*
* Lookup the device configuration based on the unique device ID.
*
* @param	DeviceId The unique device ID to search on in the configuration
*		table.
*
* @return	A pointer to the configuration data for the given device, or
*		NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
XWdtTb_Config *XWdtTb_LookupConfig(u16 DeviceId)
{
	XWdtTb_Config *CfgPtr = NULL;
	int i;

	for (i = 0; i < XPAR_XWDTTB_NUM_INSTANCES; i++) {
		if (XWdtTb_ConfigTable[i].DeviceId == DeviceId) {
			CfgPtr = &XWdtTb_ConfigTable[i];
			break;
		}
	}

	return CfgPtr;
}
