/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_srp.c
* @addtogroup hwicap Overview
* @{
*
* This file contains the functions of the XHwIcap driver used to access the
* configuration memory of the Xilinx FPGAs through the ICAP port.
*
* These APIs provide methods for reading and writing data, frames, and partial
* bitstreams to the ICAP port. See xhwicap.h for a detailed description of the
* driver.
*
* @note
*
* Only 7 series, Zynq, Ultrascale and ZynqMP Ultrascale devices are supported.
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/17/03 First release
* 1.01a sv   03/03/07 V4 Updates.
* 2.00a ecm  10/20/07 V5 Support
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  2/25/10  Added support for S6
* 5.00a hvm  5/21/10  Modified XHwIcap_GetConfigReg function for V4/V5/V6/S6
*			command sequence. Added one extra NOP before the
*			Type 1 read config register command and removed an
*			extra NOP after the config register command.
* 6.00a hvm  8/12/11  Added support for K7
* 7.00a bss  03/14/12 ReadId API is added to desync after lock up during
*			configuration CR 637538
*
* 8.00a bss  06/20/12 Deleted ReadId API as per CR 656162
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.5  Nava 09/30/22 Added new IDCODE's as mentioned in the ug570 Doc.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include <xil_types.h>
#include <xil_assert.h>
#include "xhwicap.h"

/************************** Constant Definitions *****************************/

#define DESYNC_COMMAND_SIZE	7 /* Number of words in the Desync command */
#define CAPTURE_COMMAND_SIZE	7 /* Number of words in the Capture command */
#define READ_CFG_REG_COMMAND_SIZE 7 /* Num of words in Read Config command */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
*
* Sends a DESYNC command to the ICAP port.
*
* @param	InstancePtr - a pointer to the XHwIcap instance to be worked on
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
******************************************************************************/
int XHwIcap_CommandDesync(XHwIcap *InstancePtr)
{
	int Status;
	u32 FrameBuffer[DESYNC_COMMAND_SIZE];
	u32 Index =0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Create the data to be written to the ICAP.
	 */
	FrameBuffer[Index++] = (XHwIcap_Type1Write(XHI_CMD) | 1);
	FrameBuffer[Index++] = XHI_CMD_DESYNCH;
	FrameBuffer[Index++] = XHI_DUMMY_PACKET;
	FrameBuffer[Index++] = XHI_DUMMY_PACKET;


	/*
	 * Write the data to the FIFO and intiate the transfer of data present
	 * in the FIFO to the ICAP device.
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, &FrameBuffer[0], Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Sends a CAPTURE command to the ICAP port.  This command captures all
* of the flip flop states so they will be available during readback.
* One can use this command instead of enabling the CAPTURE block in the
* design.
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
*
* @return	XST_SUCCESS or XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
int XHwIcap_CommandCapture(XHwIcap *InstancePtr)
{
	int Status;
	u32 FrameBuffer[CAPTURE_COMMAND_SIZE];
	u32 Index =0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Create the data to be written to the ICAP.
	 */
	FrameBuffer[Index++] = XHI_DUMMY_PACKET;
	FrameBuffer[Index++] = XHI_SYNC_PACKET;
	FrameBuffer[Index++] = XHI_NOOP_PACKET;
	FrameBuffer[Index++] = (XHwIcap_Type1Write(XHI_CMD) | 1);
	FrameBuffer[Index++] = XHI_CMD_GCAPTURE;
	FrameBuffer[Index++] =  XHI_DUMMY_PACKET;
	FrameBuffer[Index++] =  XHI_DUMMY_PACKET;

	/*
	 * Write the data to the FIFO and intiate the transfer of data present
	 * in the FIFO to the ICAP device.
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, &FrameBuffer[0], Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
 *
 * This function returns the value of the specified configuration register.
 *
 * @param	InstancePtr is a pointer to the XHwIcap instance.
 * @param	ConfigReg  is a constant which represents the configuration
 *		register value to be returned. Constants specified in
 *		xhwicap_i.h.
 * 		Examples:  XHI_IDCODE, XHI_FLR.
 * @param	RegData is the value of the specified configuration
 *		register.
 *
 * @return	XST_SUCCESS or XST_FAILURE
 *
 * @note	This is a blocking call.
 *
 *****************************************************************************/
u32 XHwIcap_GetConfigReg(XHwIcap *InstancePtr, u32 ConfigReg, u32 *RegData)
{
	int Status;
	int EosRetries =0; /* Counter for checking EOS to become high */
	u32 FrameBuffer[READ_CFG_REG_COMMAND_SIZE];
	u32 Index =0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Create the data to be written to the ICAP.
	 */
	FrameBuffer[Index++] = XHI_DUMMY_PACKET;
	FrameBuffer[Index++] = XHI_SYNC_PACKET;
	FrameBuffer[Index++] = XHI_NOOP_PACKET;
	FrameBuffer[Index++] = XHI_NOOP_PACKET;
	FrameBuffer[Index++] = XHwIcap_Type1Read(ConfigReg) | 0x1;
	FrameBuffer[Index++] = XHI_NOOP_PACKET;
	FrameBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Check for EOS bit of Status Register. EOS bit becomes high after
	 * ICAP completes Start up sequence. Access to ICAP should start
	 * only after EOS bit becomes high.
	 */

	while((!(XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
			XHI_SR_OFFSET)& XHI_SR_EOS_MASK))) {

		if(EosRetries < XHI_MAX_RETRIES) {
			EosRetries++;
		}
		else {
	   		return XST_FAILURE;
		}

	}

	/*
	 * Write the data to the FIFO and intiate the transfer of data present
	 * in the FIFO to the ICAP device.
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, &FrameBuffer[0], Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	while (XHwIcap_IsDeviceBusy(InstancePtr) != FALSE);
	while ((XHwIcap_ReadReg(InstancePtr->HwIcapConfig.BaseAddress,
			XHI_CR_OFFSET)) & XHI_CR_WRITE_MASK);

	/*
	 * Read the Config Register using DeviceRead since
	 * DeviceRead reads depending on ICAP Width for V6
	 * and 7 series devices
  	 */
	XHwIcap_DeviceRead(InstancePtr, RegData, 1);

	return XST_SUCCESS;
}

/** @} */
