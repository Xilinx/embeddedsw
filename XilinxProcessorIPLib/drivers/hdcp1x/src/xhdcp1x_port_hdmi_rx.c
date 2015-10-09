/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
*
* @file xhdcp1x_port_hdmi_rx.c
* @addtogroup hdcp1x_v1_0
* @{
*
* This contains the implementation of the HDCP port driver for HDMI RX
* interfaces
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00  fidus  07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#if defined(XPAR_XV_HDMIRX_NUM_INSTANCES) && (XPAR_XV_HDMIRX_NUM_INSTANCES > 0)
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_hdmi.h"
#include "xv_hdmirx.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/* Adaptor definition at the end of this file. */
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortHdmiRxAdaptor;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes *****************************/

static int XHdcp1x_PortHdmiRxEnable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortHdmiRxDisable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortHdmiRxInit(XHdcp1x *InstancePtr);
static int XHdcp1x_PortHdmiRxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize);
static int XHdcp1x_PortHdmiRxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize);
static void XHdcp1x_ProcessAKsvWrite(void *CallbackRef);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function enables a HDCP port device.
*
* @param	InstancePtr is the id of the device to enable.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortHdmiRxEnable(XHdcp1x *InstancePtr)
{
	XV_HdmiRx *HdmiRx = NULL;
	u8 Buf[4];
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Determine HdmiRx */
	HdmiRx = InstancePtr->Port.PhyIfPtr;

	/* Initialize the Bstatus register */
	memset(Buf, 0, 4);
	Buf[1] |= (XHDCP1X_PORT_BIT_BSTATUS_HDMI_MODE >> 8);
	XHdcp1x_PortHdmiRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
									Buf, 2);

	/* Initialize the Bcaps register */
	memset(Buf, 0, 4);
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_HDMI;
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_FAST_REAUTH;
	XHdcp1x_PortHdmiRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, Buf, 1);

	/* Initialize some debug registers */
	Buf[0] = 0xDE;
	Buf[1] = 0xAD;
	Buf[2] = 0xBE;
	Buf[3] = 0xEF;
	XHdcp1x_PortHdmiRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_DBG, Buf, 4);

	/* Bind for interrupt callback */
	XV_HdmiRx_SetCallback(HdmiRx, XV_HDMIRX_HANDLER_HDCP,
			XHdcp1x_ProcessAKsvWrite, InstancePtr);

	return (Status);
}

/*****************************************************************************/
/**
* This function disables a HDCP port device.
*
* @param	InstancePtr is the id of the device to disable.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortHdmiRxDisable(XHdcp1x *InstancePtr)
{
	u8 Offset = 0;
	u8 Value = 0;
	u32 HdmiRxBase = 0;
	u32 RegValue;
	int NumLeft = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Determine HdmiRxBase */
	HdmiRxBase =
		((XV_HdmiRx *)InstancePtr->Port.PhyIfPtr)->Config.BaseAddress;

	/* Disable the hdcp ddc slave */
	RegValue = XV_HdmiRx_ReadReg(HdmiRxBase, XV_HDMIRX_DDC_CTRL_SET_OFFSET);
	RegValue &= ~XV_HDMIRX_DDC_CTRL_HDCP_EN_MASK;
	XV_HdmiRx_WriteReg(HdmiRxBase, XV_HDMIRX_DDC_CTRL_SET_OFFSET, RegValue);

	/* Clear the hdcp registers */
	Value = 0;
	Offset = 0;
	NumLeft = 256;
	while (NumLeft-- > 0) {
		XHdcp1x_PortHdmiRxWrite(InstancePtr, Offset++, &Value, 1);
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function initializes a HDCP port device.
*
* @param	InstancePtr is the device to initialize.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_FAILURE otherwise.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortHdmiRxInit(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Disable it */
	if (XHdcp1x_PortHdmiRxDisable(InstancePtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function reads a register from a HDCP port device.
*
* @param	InstancePtr is the device to read from.
* @param	Offset is the offset to start reading from.
* @param	Buf is the buffer to copy the data read.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes read.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortHdmiRxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize)
{
	XV_HdmiRx *HdmiRx = InstancePtr->Port.PhyIfPtr;
	u32 NumLeft = BufSize;
	u8 *ReadBuf = Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Write the offset */
	XV_HdmiRx_DdcHdcpSetAddress(HdmiRx, Offset);

	/* Read the buffer */
	while (NumLeft-- > 0) {
		*ReadBuf++ = XV_HdmiRx_DdcHdcpReadData(HdmiRx);
	}

	return ((int)BufSize);
}

/*****************************************************************************/
/**
* This function writes a register from a HDCP port device.
*
* @param	InstancePtr is the device to write to.
* @param	Offset is the offset to start writing to.
* @param	Buf is the buffer containing the data to write.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes written.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortHdmiRxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize)
{
	XV_HdmiRx *HdmiRx = InstancePtr->Port.PhyIfPtr;
	u32 NumLeft = BufSize;
	const u8 *WriteBuf = Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Write the offset */
	XV_HdmiRx_DdcHdcpSetAddress(HdmiRx, Offset);

	/* Write the buffer */
	while (NumLeft-- > 0) {
		XV_HdmiRx_DdcHdcpWriteData(HdmiRx, *WriteBuf++);
	}

	return ((int)BufSize);
}

/*****************************************************************************/
/**
* This function process a write to the AKsv register from the tx device.
*
* @param	CallbackRef is the device to whose register was written.
*
* @return	None.
*
* @note		This function initiates the side effects of the tx device
*		writing the Aksv register. This is currently updates some status
*		bits as well as kick starts a re-authentication process.
*
******************************************************************************/
static void XHdcp1x_ProcessAKsvWrite(void *CallbackRef)
{
	XHdcp1x *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Update statistics */
	InstancePtr->Port.Stats.IntCount++;

	/* Clear bit 1 of the Ainfo register */
	XHdcp1x_PortHdmiRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO,
								&Value, 1);
	Value &= 0xFDu;
	XHdcp1x_PortHdmiRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO,
								&Value, 1);

	/* Invoke authentication callback if set */
	if (InstancePtr->Port.IsAuthCallbackSet) {
		(*(InstancePtr->Port.AuthCallback))(InstancePtr->Port.AuthRef);
	}
}

/*****************************************************************************/
/**
* This tables defines the adaptor for the HDMI RX HDCP port driver
*
******************************************************************************/
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortHdmiRxAdaptor =
{
	&XHdcp1x_PortHdmiRxInit,
	&XHdcp1x_PortHdmiRxEnable,
	&XHdcp1x_PortHdmiRxDisable,
	&XHdcp1x_PortHdmiRxRead,
	&XHdcp1x_PortHdmiRxWrite,
	NULL,
	NULL,
	NULL,
	NULL,
};

#endif
/* defined(XPAR_XV_HDMIRX_NUM_INSTANCES) && (XPAR_XV_HDMIRX_NUM_INSTANCES > 0) */
/** @} */
