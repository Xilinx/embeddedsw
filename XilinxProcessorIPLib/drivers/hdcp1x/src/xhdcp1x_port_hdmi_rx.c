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
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xhdcp1x_port_hdmi_rx.c
*
* This contains the implementation of the HDCP port driver for HDMI RX
* interfaces
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- ------ -------- --------------------------------------------------
* 1.00         07/16/15 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xparameters.h"

#if defined(XPAR_XHDMI_RX_NUM_INSTANCES) && (XPAR_XHDMI_RX_NUM_INSTANCES > 0)
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_hdmi.h"
#include "xhdmi_rx.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This reads a register from the hdcp port device
*
* @param InstancePtr  the device to read from
* @param Offset  the offset to start reading from
* @param Buf  the Bufer to copy the data read
* @param BufSize  the size of the Bufer
*
* @return
*   The number of bytes read
*
* @note
*   None.
*
******************************************************************************/
static int RegRead(const XHdcp1x_Port *InstancePtr, u8 Offset, u8 *Buf,
                u32 BufSize)
{
	/* Locals */
	XHdmi_Rx *HdmiRx = InstancePtr->PhyIfPtr;
	u32 NumLeft = BufSize;

	/* Write the offset */
	XHdmiRx_DdcHdcpSetAddress(HdmiRx, Offset);

	/* Read the buffer */
	while (NumLeft-- > 0) {
		*Buf++ = XHdmiRx_DdcHdcpReadData(HdmiRx);
	}

	/* Return */
	return ((int) BufSize);
}

/*****************************************************************************/
/**
*
* This writes a register from the hdcp port device
*
* @param InstancePtr  the device to write to
* @param Offset  the offset to start writing at
* @param Buf  the Bufer containing the data to write
* @param BufSize  the size of the Bufer
*
* @return
*   The number of bytes written
*
* @note
*   None.
*
******************************************************************************/
static int RegWrite(XHdcp1x_Port *InstancePtr, u8 Offset, const u8 *Buf,
		u32 BufSize)
{
	XHdmi_Rx *HdmiRx = InstancePtr->PhyIfPtr;
	u32 NumLeft = BufSize;

	/* Write the offset */
	XHdmiRx_DdcHdcpSetAddress(HdmiRx, Offset);

	/* Write the buffer */
	while (NumLeft-- > 0) {
		XHdmiRx_DdcHdcpWriteData(HdmiRx, *Buf++);
	}

	/* Return */
	return ((int) BufSize);
}

/*****************************************************************************/
/**
*
* This function process a write to the AKsv register from the tx device
*
* @param CallbackRef  the device to whose register was written
*
* @return
*   void
*
* @note
*   This function initiates the side effects of the tx device writing the Aksv
*   register.  This is currently updates some status bits as well as kick
*   starts a re-authentication process.
*
******************************************************************************/
static void ProcessAKsvWrite(void *CallbackRef)
{
	XHdcp1x_Port *InstancePtr = (XHdcp1x_Port *) CallbackRef;
	u8 Value = 0;

	/* Update statistics */
	InstancePtr->Stats.IntCount++;

	/* Clear bit 1 of the Ainfo register */
	RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO, &Value, 1);
	Value &= 0xFDu;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO, &Value, 1);

	/* Invoke authentication callback if set */
	if (InstancePtr->IsAuthCallbackSet) {
		(*(InstancePtr->AuthCallback))(InstancePtr->AuthRef);
	}

	return;
}

/*****************************************************************************/
/**
*
* This function enables a hdcp port device
*
* @param InstancePtr  the id of the device to enable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiRxEnable(XHdcp1x_Port *InstancePtr)
{
	XHdmi_Rx *HdmiRx = NULL;
	u8 Buf[4];
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Determine HdmiRx */
	HdmiRx = InstancePtr->PhyIfPtr;

	/* Initialize the Bstatus register */
	memset(Buf, 0, 4);
	Buf[1] |= (XHDCP1X_PORT_BIT_BSTATUS_HDMI_MODE >> 8);
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf, 2);

	/* Initialize the Bcaps register */
	memset(Buf, 0, 4);
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_HDMI;
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_FAST_REAUTH;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, Buf, 1);

	/* Initialize some debug registers */
	Buf[0] = 0xDE;
	Buf[1] = 0xAD;
	Buf[2] = 0xBE;
	Buf[3] = 0xEF;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_DBG, Buf, 4);

	/* Bind for interrupt callback */
	XHdmiRx_SetCallback(HdmiRx, XHDMI_RX_HANDLER_HDCP,
			ProcessAKsvWrite, InstancePtr);

	/* Enable the hdcp slave over the ddc */
	XHdmiRx_DdcHdcpEnable(HdmiRx);
	XHdmiRx_DdcIntrEnable(HdmiRx);

	return (Status);
}

/*****************************************************************************/
/**
*
* This function disables a hdcp port device
*
* @param InstancePtr  the id of the device to disable
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiRxDisable(XHdcp1x_Port *InstancePtr)
{
	u8 Offset = 0;
	u8 U8Value = 0;
	u32 HdmiRxBase = 0;
	u32 Value;
	int NumLeft = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Determine HdmiRxBase */
	HdmiRxBase = ((XHdmi_Rx*) InstancePtr->PhyIfPtr)->Config.BaseAddress;

	/* Disable the hdcp ddc slave */
	Value = XHdmiRx_ReadReg(HdmiRxBase, XHDMI_RX_DDC_CTRL_SET_OFFSET);
	Value &= ~XHDMI_RX_DDC_CTRL_HDCP_EN_MASK;
	XHdmiRx_WriteReg(HdmiRxBase, XHDMI_RX_DDC_CTRL_SET_OFFSET, Value);

	/* Clear the hdcp registers */
	U8Value = 0;
	Offset = 0;
	NumLeft = 256;
	while (NumLeft-- > 0) {
		RegWrite(InstancePtr, Offset++, &U8Value, 1);
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function initializes a hdcp port device
*
* @param InstancePtr  the device to initialize
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiRxInit(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Disable it */
	if (XHdcp1x_PortHdmiRxDisable(InstancePtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function reads a register from a hdcp port device
*
* @param InstancePtr  the device to read from
* @param Offset  the offset to start reading from
* @param Buf  the Bufer to copy the data read
* @param BufSize  the size of the Bufer
*
* @return
*   The number of bytes read
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiRxRead(const XHdcp1x_Port *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Read it */
	return (RegRead(InstancePtr, Offset, Buf, BufSize));
}

/*****************************************************************************/
/**
*
* This function writes a register from a hdcp port device
*
* @param InstancePtr  the device to write to
* @param Offset  the offset to start writing to
* @param Buf  the Bufer containing the data to write
* @param BufSize  the size of the Bufer
*
* @return
*   The number of bytes written
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiRxWrite(XHdcp1x_Port *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize)
{
	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Write it */
	return (RegWrite(InstancePtr, Offset, Buf, BufSize));
}

#if 0
/*****************************************************************************/
/**
*
* This function serves as the interrupt handler for the hdcp port device
*
* @param CallbackRef  the device that generated the interrupt
*
* @return
*   void
*
* @note
*   None.
*
******************************************************************************/
void XHdcp1x_PortHdmiRxIntrHandler(XHdcp1x_Port *InstancePtr, u32 IntCause)
{
	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Process the Aksv write as it is the only hdcp interrupt */
	ProcessAKsvWrite(InstancePtr);

	/* Return */
	return;
}
#endif

/*****************************************************************************/
/**
*
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
//	&XHdcp1x_PortHdmiRxIntrHandler,
};

#endif  /* defined(XPAR_XHDMI_RX_NUM_INSTANCES) && (XPAR_XHDMI_RX_NUM_INSTANCES > 0) */
