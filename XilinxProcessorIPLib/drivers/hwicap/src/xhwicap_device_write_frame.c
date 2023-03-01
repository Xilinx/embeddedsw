/******************************************************************************
* Copyright (C) 2003 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xhwicap_device_write_frame.c
* @addtogroup hwicap Overview
* @{
*
* This file contains the function that writes the frame stored in the
* memory to the device (ICAP).
*
* @note none.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/20/03 First release
* 1.01a bjb  04/10/06 V4 Support
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*		      modifications
* 5.00a hvm  2/25/10  Added support for S6
* 5.01a hvm  07/06/10 Removed the code that adds wrong data byte before the
*		      CRC bytes in the XHwIcap_DeviceWriteFrame function for S6
*		      (CR560534)
* 6.00a hvm  08/01/11 Added support for K7
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.5  Nava 09/30/22 Added new IDCODE's as mentioned in the ug570 Doc.
*
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xhwicap.h"
#include <xil_types.h>
#include <xil_assert.h>

/************************** Constant Definitions ****************************/

#define READ_FRAME_SIZE 30

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/

/****************************************************************************/
/**
*
* Writes one frame from the specified buffer and puts it in the device
* (ICAP).
*
* @param	InstancePtr is a pointer to the XHwIcap instance.
* @param	Top - top (0) or bottom (1) half of device
* @param	Block - Block Address (XHI_FAR_CLB_BLOCK,
* 		XHI_FAR_BRAM_BLOCK, XHI_FAR_BRAM_INT_BLOCK)
* @param	HClkRow - selects the HClk Row
* @param	MajorFrame - selects the column
* @param	MinorFrame - selects frame inside column
* @param	FrameData is a pointer to the frame that is to be written
*		to the device.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		This is a blocking function.
*		This function is used in conjunction with the function
*		XHwIcap_DeviceReadFrame. This function is used to write back
*		the frame of data read using the XHwIcap_DeviceReadFrame.
*
*****************************************************************************/
int XHwIcap_DeviceWriteFrame(XHwIcap *InstancePtr, long Top, long Block,
				long HClkRow, long MajorFrame, long MinorFrame,
				u32 *FrameData)
{

	u32 Packet;
	u32 Data;
	u32 TotalWords;
	int Status;
	u32 WriteBuffer[READ_FRAME_SIZE];
	u32 Index =0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(FrameData != NULL);


	/*
	 * DUMMY and SYNC
	 */
	WriteBuffer[Index++] = XHI_DUMMY_PACKET;
	WriteBuffer[Index++] = XHI_SYNC_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Reset CRC
	 */
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	Data = XHI_CMD_RCRC;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Bypass CRC
	 */

	/*
	 * ID register
	 */
	Packet = XHwIcap_Type1Write(XHI_IDCODE) | 1;
	Data = InstancePtr->DeviceIdCode;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;

	/*
	 * Setup FAR
	 */
	Packet = XHwIcap_Type1Write(XHI_FAR) | 1;

	Data = XHwIcap_SetupFar(Top, Block, HClkRow,  MajorFrame, MinorFrame);

	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;

	/*
	 * Setup CMD register - write configuration
	 */
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	Data = XHI_CMD_WCFG;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Setup Packet header.
	 */
	TotalWords = InstancePtr->WordsPerFrame << 1;
	if (TotalWords < XHI_TYPE_1_PACKET_MAX_WORDS)  {
		/*
		 * Create Type 1 Packet.
		 */
		Packet = XHwIcap_Type1Write(XHI_FDRI) | TotalWords;
		WriteBuffer[Index++] = Packet;
	}
	else {

		/*
		 * Create Type 2 Packet.
		 */
		Packet = XHwIcap_Type1Write(XHI_FDRI);
		WriteBuffer[Index++] = Packet;

		Packet = XHI_TYPE_2_WRITE | TotalWords;
		WriteBuffer[Index++] = Packet;
	}

	/*
	 * Write the Header data into the FIFO and intiate the transfer of
	 * data present in the FIFO to the ICAP device
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, (u32 *)&WriteBuffer[0], Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	/*
	 * Write the modified frame data.
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr,
				(u32 *) &FrameData[InstancePtr->WordsPerFrame],
				InstancePtr->WordsPerFrame);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Write out the pad frame. The pad frame was read from the device
	 * before the data frame.
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, (u32 *) &FrameData[0],
				    InstancePtr->WordsPerFrame);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Add CRC */
	Index = 0;
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	Data = XHI_CMD_RCRC;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/* Park the FAR */
	Packet = XHwIcap_Type1Write(XHI_FAR) | 1;
	Data = XHwIcap_SetupFar(0, 0, 3, 33, 0);

	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] =  Data;

	/* Add CRC */
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	Data = XHI_CMD_RCRC;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;


	/*
	 * Intiate the transfer of data present in the FIFO to
	 * the ICAP device
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, &WriteBuffer[0], Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	/*
	 * Send DESYNC command
	 */
	Status = XHwIcap_CommandDesync(InstancePtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
};


/** @} */
