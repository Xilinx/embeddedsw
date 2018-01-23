/******************************************************************************
*
* Copyright (C) 2003 - 2016 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
* @file xhwicap_device_read_frame.c
* @addtogroup hwicap_v11_1
* @{
*
* This file contains the function that reads a specified frame from the
* device (ICAP) and stores it in the memory specified by the user.
*
* @note none.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00a bjb  11/20/03 First release
* 1.01a nps  04/10/06 V4 Support
* 2.00a ecm  10/20/07 V5 Support
* 4.00a hvm  11/30/09 Added support for V6 and updated with HAL phase 1
*					  modifications
* 5.00a hvm  2/25/10  Added support for S6
* 6.00a hvm  08/01/11 Added support for K7
* 10.0  bss  6/24/14  Removed support for families older than 7 series
* 11.0  MNK  6/12/14  Added support for 8-series family devices.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xhwicap_i.h"
#include "xhwicap.h"
#include <xil_types.h>
#include <xil_assert.h>

/************************** Constant Definitions ****************************/

#define WRITE_FRAME_SIZE 91

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/


/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/
/****************************************************************************/
/**
*
* Reads one frame from the device and puts it in memory specified by the user.
*
* @param	InstancePtr - a pointer to the XHwIcap instance to be worked on.
* @param	Top - top (0) or bottom (1) half of device
* @param	Block - Block Address (XHI_FAR_CLB_BLOCK,
*		XHI_FAR_BRAM_BLOCK, XHI_FAR_BRAM_INT_BLOCK)
* @param	HClkRow - selects the HClk Row
* @param	MajorFrame - selects the column
* @param	MinorFrame - selects frame inside column
* @param	FrameBuffer is a pointer to the memory where the frame read
*		from the device is stored
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		This is a blocking call.
*
*****************************************************************************/
int XHwIcap_DeviceReadFrame(XHwIcap *InstancePtr, long Top, long Block,
				long HClkRow, long MajorFrame, long MinorFrame,
				u32 *FrameBuffer)
{

	u32 Packet;
	u32 Data;
	u32 TotalWords;
	int Status;
	u32 WriteBuffer[WRITE_FRAME_SIZE];
	u32 Index = 0;
	u32 NumNoops;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(FrameBuffer != NULL);

	/*
	 * DUMMY and SYNC
	 */
	WriteBuffer[Index++] = XHI_DUMMY_PACKET;
	WriteBuffer[Index++] = XHI_BUS_WTH_PACKET;
	WriteBuffer[Index++] = XHI_BUS_DET_PACKET;
	WriteBuffer[Index++] = XHI_DUMMY_PACKET;
	WriteBuffer[Index++] = XHI_SYNC_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Reset CRC
	 */
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = XHI_CMD_RCRC;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Setup CMD register to read configuration
	 */
	Packet = XHwIcap_Type1Write(XHI_CMD) | 1;
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = XHI_CMD_RCFG;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	/*
	 * Setup FAR register.
	 */
	Packet = XHwIcap_Type1Write(XHI_FAR) | 1;
	Data = XHwIcap_SetupFar(Top, Block, HClkRow,  MajorFrame, MinorFrame);
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = Data;

	/*
	 * Setup read data packet header.
	 * The frame will be preceeded by a dummy frame, and we need to read one
	 * extra word for V4 and V5 devices.
	 */
	switch (InstancePtr->DeviceFamily) {
		case DEVICE_TYPE_7SERIES :
				TotalWords = InstancePtr->WordsPerFrame << 1;
				NumNoops = 32;
				break;
		case DEVICE_TYPE_ULTRA :
			TotalWords = (InstancePtr->WordsPerFrame << 1) + 10;
			NumNoops = 64;
				break;
		case DEVICE_TYPE_ULTRA_PLUS :
			TotalWords = (InstancePtr->WordsPerFrame << 1) + 25;
			NumNoops = 64;
				break;
		default:
			return XST_FAILURE;
	}

	/*
	 * Create Type one packet
	 */
	Packet = XHwIcap_Type1Read(XHI_FDRO);
	WriteBuffer[Index++] = Packet;
	WriteBuffer[Index++] = 0x48000000 | TotalWords;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;
	WriteBuffer[Index++] = XHI_NOOP_PACKET;

	for(unsigned int i = 0; i < NumNoops; i++) {
		WriteBuffer[Index++] = XHI_NOOP_PACKET;
	}

	/*
	 * Write the data to the FIFO and initiate the transfer of data
	 * present in the FIFO to the ICAP device
	 */
	Status = XHwIcap_DeviceWrite(InstancePtr, (u32 *)&WriteBuffer[0],
			Index);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	/*
	 * Wait till the write is done.
	 */
	while (XHwIcap_IsDeviceBusy(InstancePtr) != FALSE);


	/*
	 * Read the frame of the data including the NULL frame.
	 */
	Status = XHwIcap_DeviceRead(InstancePtr, FrameBuffer, TotalWords);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	/*
	 * Send DESYNC command
	 */
	Status = XHwIcap_CommandDesync(InstancePtr);
	if (Status != XST_SUCCESS)  {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
};
/** @} */
