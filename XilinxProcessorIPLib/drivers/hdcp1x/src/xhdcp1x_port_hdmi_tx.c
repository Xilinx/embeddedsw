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
* @file xhdcp1x_port_hdmi_tx.c
*
* This contains the implementation of the HDCP port driver for HDMI TX
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

#if defined(XPAR_XHDMI_TX_NUM_INSTANCES) && (XPAR_XHDMI_TX_NUM_INSTANCES > 0)
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_hdmi.h"
#include "xhdmi_tx.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/
#define WRITE_CHUNK_SZ			(8)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
*
* This reads a register from the hdcp port device
*
* @param InstancePtr  the device to read from
* @param Offdry  the offset to start reading from
* @param Buf  the buffer to copy the data read
* @param BufSize  the size of the buffer
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
	XHdmi_Tx* HdmiTx = InstancePtr->PhyIfPtr;
	u8 Slave = 0x3Au;
	int NumRead = 0;

	/* Write the address and check for failure */
	if (XHdmiTx_DdcWrite(HdmiTx, Slave, 1, &Offset, FALSE)
		!= XST_SUCCESS) {
		NumRead = -1;
	}
	/* Read the data back and check for failure */
	else if (XHdmiTx_DdcRead(HdmiTx, Slave, BufSize, Buf, TRUE)
		!= XST_SUCCESS) {
		NumRead = -2;
	}
	/* Success - just update NumRead */
	else {
		NumRead = (int) BufSize;
	}

	return (NumRead);
}

/*****************************************************************************/
/**
*
* This writes a register from the hdcp port device
*
* @param InstancePtr  the device to write to
* @param Offset  the offset to start writing at
* @param Buf  the buffer containing the data to write
* @param BufSize  the size of the buffer
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
	XHdmi_Tx* HdmiTx = InstancePtr->PhyIfPtr;
	u8 Slave = 0x3Au;
	u8 TxBuf[WRITE_CHUNK_SZ+1];
	int NumWritten = 0;
	int ThisTime = 0;

	/* Iterate through the buffer */
	do {
		/* Determine ThisTime */
		ThisTime = WRITE_CHUNK_SZ;
		if (ThisTime > BufSize) {
			ThisTime = BufSize;
		}

		/* Format TxBuf */
		TxBuf[0] = Offset;
		memcpy(&(TxBuf[1]), Buf, ThisTime);

		/* Write the TxBuf */
		if (XHdmiTx_DdcWrite(HdmiTx, Slave, (ThisTime+1), TxBuf, TRUE)
			!= XST_SUCCESS) {
			/* Update NumWritten and break */
			NumWritten = -1;
			break;
		}

		/* Update for loop */
		NumWritten += ThisTime;
		Buf += ThisTime;
		BufSize -= ThisTime;

	} while ((BufSize != 0) && (NumWritten > 0));

	/* Return */
	return (NumWritten);
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
int XHdcp1x_PortHdmiTxEnable(XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int Status = XST_NOT_ENABLED;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Read anything to ensure that the remote end is present */
	if ((RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1)) > 0) {
		Status = XST_SUCCESS;
	}

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
int XHdcp1x_PortHdmiTxDisable(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Nothing to do at this time */

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
int XHdcp1x_PortHdmiTxInit(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Disable it */
	if (XHdcp1x_PortHdmiTxDisable(InstancePtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
*
* This function confirms the presence/capability of the remote hdcp device
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiTxIsCapable(const XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int IsCapable = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for hdcp capable */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_HDMI) != 0) {
			IsCapable = TRUE;
		}
	}

	return (IsCapable);
}

/*****************************************************************************/
/**
*
* This function confirms if the remote hdcp device is a repeater
*
* @param InstancePtr  the device to query
*
* @return
*   Truth value
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiTxIsRepeater(const XHdcp1x_Port *InstancePtr)
{
	u8 Value = 0;
	int IsRepeater = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for repeater */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_REPEATER) != 0) {
			IsRepeater = TRUE;
		}
	}

	return (IsRepeater);
}

/*****************************************************************************/
/**
*
* This function retrieves the repeater information
*
* @param InstancePtr  the device to query
*
* @return
*   XST_SUCCESS if successful.
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiTxGetRepeaterInfo(const XHdcp1x_Port *InstancePtr, u16 *Info)
{
	u8 Value = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Info != NULL);

	/* Read the remote capabilities */
	if (RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, &Value, 1) > 0) {

		u8 ReadyMask = 0;

		/* Determine ReadyMask */
		ReadyMask  = XHDCP1X_PORT_BIT_BCAPS_REPEATER;
		ReadyMask |= XHDCP1X_PORT_BIT_BCAPS_READY;

		/* Check for repeater and ksv fifo ready */
		if ((Value & ReadyMask) == ReadyMask) {

			u8 Buf[2];
			u16 U16Value = 0;

			/* Read the Bstatus */
			RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
					Buf, 2);

			/* Determine Value */
			XHDCP1X_PORT_BUF_TO_UINT(U16Value, Buf, 16);

			/* Update Info */
			*Info = (U16Value & 0x0FFFu);
		}
		else {
			Status = XST_DEVICE_BUSY;
		}
	}
	else {
		Status = XST_RECV_ERROR;
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
* @param Buf  the buffer to copy the data read
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes read
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiTxRead(const XHdcp1x_Port* InstancePtr, u8 Offset,
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
* @param Buf  the buffer containing the data to write
* @param BufSize  the size of the buffer
*
* @return
*   The number of bytes written
*
* @note
*   None.
*
******************************************************************************/
int XHdcp1x_PortHdmiTxWrite(XHdcp1x_Port *InstancePtr, u8 Offset,
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

/*****************************************************************************/
/**
*
* This tables defines the adaptor for the HDMI TX HDCP port driver
*
******************************************************************************/
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortHdmiTxAdaptor =
{
	&XHdcp1x_PortHdmiTxInit,
	&XHdcp1x_PortHdmiTxEnable,
	&XHdcp1x_PortHdmiTxDisable,
	&XHdcp1x_PortHdmiTxRead,
	&XHdcp1x_PortHdmiTxWrite,
	&XHdcp1x_PortHdmiTxIsCapable,
	&XHdcp1x_PortHdmiTxIsRepeater,
	&XHdcp1x_PortHdmiTxGetRepeaterInfo,
	NULL,
};

#endif  /* defined(XPAR_XHDMI_TX_NUM_INSTANCES) && (XPAR_XHDMI_TX_NUM_INSTANCES > 0) */
