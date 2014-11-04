/*******************************************************************************
 *
 * Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
*******************************************************************************/
/******************************************************************************/
/**
 *
 * @file xdptx_edid.c
 *
 * This file contains functions related to accessing the Extended Display
 * Identification Data (EDID) of a specified sink using the XDptx driver.
 *
 * @note	None.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 3.0   als  11/04/14 Initial release.
 * </pre>
 *
*******************************************************************************/

/******************************* Include Files ********************************/
#include "xstatus.h"
#include "xdptx.h"

/**************************** Function Definitions ****************************/

/******************************************************************************/
/**
 * This function retrieves an immediately connected RX device's Extended Display
 * Identification Data (EDID) structure.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	Edid is a pointer to the Edid buffer to save to.
 *
 * @return
 *		- XST_SUCCESS if the I2C transactions to read the EDID were
 *		  successful.
 *		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_GetEdid(XDptx *InstancePtr, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(Edid != NULL);

	Status = XDptx_IicRead(InstancePtr, XDPTX_EDID_ADDR, 0,
						XDPTX_EDID_BLOCK_SIZE, Edid);

	return Status;
}

/******************************************************************************/
/**
 * This function retrieves a remote RX device's Extended Display Identification
 * Data (EDID) structure.
 *
 * @param	InstancePtr is a pointer to the XDptx instance.
 * @param	LinkCountTotal is the number of DisplayPort links from the
 *		DisplayPort source to the target DisplayPort device.
 * @param	RelativeAddress is the relative address from the DisplayPort
 *		source to the target DisplayPort device.
 * @param	Edid is a pointer to the Edid buffer to save to.
 *
 * @return
 *		- XST_SUCCESS if the I2C transactions to read the EDID were
 *		  successful.
 *		- XST_ERROR_COUNT_MAX if the EDID read request timed out.
 *		- XST_DEVICE_NOT_FOUND if no RX device is connected.
 *		- XST_FAILURE otherwise.
 *
 * @note	None.
 *
*******************************************************************************/
u32 XDptx_GetRemoteEdid(XDptx *InstancePtr, u8 LinkCountTotal,
						u8 *RelativeAddress, u8 *Edid)
{
	u32 Status;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(LinkCountTotal > 0);
	Xil_AssertNonvoid((RelativeAddress != NULL) || (LinkCountTotal == 1));
	Xil_AssertNonvoid(Edid != NULL);

	Status = XDptx_RemoteIicRead(InstancePtr, LinkCountTotal,
		RelativeAddress, XDPTX_EDID_ADDR, 0, XDPTX_EDID_BLOCK_SIZE,
									Edid);

	return Status;
}

u32 XDptx_GetEdidBlock(XDptx *InstancePtr, u8 *Data, u8 BlockNum)
{
	u32 Status;
	u16 Offset;

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * 128;

	/* Issue the I2C read for the specified EDID block. */
	Status = XDptx_IicRead(InstancePtr, XDPTX_EDID_ADDR, Offset, 128, Data);

	return Status;
}

u32 XDptx_GetRemoteEdidBlock(XDptx *InstancePtr, u8 *Data, u8 BlockNum,
					u8 LinkCountTotal, u8 *RelativeAddress)
{
	u32 Status;
	u16 Offset;

	/* Calculate the I2C offset for the specified EDID block. */
	Offset = BlockNum * 128;

	/* Issue the I2C read for the specified EDID block. */
	Status = XDptx_RemoteIicRead(InstancePtr, LinkCountTotal,
		RelativeAddress, XDPTX_EDID_ADDR, Offset, 128, Data);

	return Status;
}
