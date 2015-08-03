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
* @file xhdcp1x_port_dp_rx.c
*
* This contains the implementation of the HDCP port driver for Xilinx DP
* RX interfaces
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
#if defined(XPAR_XDP_NUM_INSTANCES) && (XPAR_XDP_NUM_INSTANCES > 0)
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_dp.h"
#include "xdp.h"
#include "xdp_hw.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes *****************************/

static int RegRead(const XHdcp1x_Port *InstancePtr, u8 Offset, u8 *Buf,
		u32 BufSize);
static int RegWrite(XHdcp1x_Port *InstancePtr, u8 Offset, const u8 *Buf,
		u32 BufSize);
static void ProcessAKsvWrite(void *CallbackRef);
static void ProcessRoRead(void *CallbackRef);
static void ProcessBinfoRead(void *CallbackRef);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function enables a HDCP port device.
*
* @param	InstancePtr is the device to enable.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortDpRxEnable(XHdcp1x_Port *InstancePtr)
{
	XDprx *HwDp = InstancePtr->PhyIfPtr;
	u32 IntMask = 0;
	u8 Buf[4];
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Initialize Buf */
	memset(Buf, 0, 4);

	/* Initialize Bstatus register */
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, Buf, 1);

	/* Initialize Binfo register */
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BINFO, Buf, 2);

	/* Initialize Bcaps register */
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_HDCP_CAPABLE;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, Buf, 1);

	/* Initialize some debug registers */
	Buf[0] = 0xDE;
	Buf[1] = 0xAD;
	Buf[2] = 0xBE;
	Buf[3] = 0xEF;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_DBG, Buf, 4);

	/* Register callbacks */
	XDp_RxSetIntrHdcpAksvWriteHandler(HwDp, &ProcessAKsvWrite,
			InstancePtr);
	XDp_RxSetIntrHdcpBinfoReadHandler(HwDp, &ProcessBinfoRead,
			InstancePtr);
	XDp_RxSetIntrHdcpRoReadHandler(HwDp, &ProcessRoRead,
			InstancePtr);

	/* Enable interrupts */
	IntMask  = XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK;
	XDp_RxInterruptEnable(HwDp, IntMask);

	return (Status);
}

/*****************************************************************************/
/**
* This function disables a HDCP port device.
*
* @param	InstancePtr is the device to disable.
*
* @return
*		- XST_SUCCESS if successful.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortDpRxDisable(XHdcp1x_Port *InstancePtr)
{
	XDprx *HwDp = InstancePtr->PhyIfPtr;
	u32 IntMask = 0;
	u8 Offset = 0;
	u8 Value = 0;
	int NumLeft = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Disable interrupts */
	IntMask  = XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK;
	XDp_RxInterruptDisable(HwDp, IntMask);

	/* Clear hdcp registers */
	Value = 0;
	Offset = 0;
	NumLeft = 256;
	while (NumLeft-- > 0) {
		RegWrite(InstancePtr, Offset++, &Value, sizeof(Value));
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
int XHdcp1x_PortDpRxInit(XHdcp1x_Port *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->PhyIfPtr != NULL);

	/* Disable it */
	if (XHdcp1x_PortDpRxDisable(InstancePtr) != XST_SUCCESS) {
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
* @param	Buf is the buffer to copy data read.
* @param	BufSize is the size of the buffer.
*
* @return	Is the number of bytes read.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortDpRxRead(const XHdcp1x_Port *InstancePtr, u8 Offset,
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
* This function writes a register from a HDCP port device.
*
* @param	InstancePtr is the device to write to.
* @param	Offset is the offset to start writing to.
* @param	Buf is the buffer containing data to write.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes written.
*
* @note		None.
*
******************************************************************************/
int XHdcp1x_PortDpRxWrite(XHdcp1x_Port *InstancePtr, u8 Offset,
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
* This reads a register from the HDCP port device.
*
* @param	InstancePtr is the device to read from.
* @param	Offset is the offset to start reading from.
* @param	Buf is the buffer to copy data read.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes read.
*
* @note		None.
*
******************************************************************************/
static int RegRead(const XHdcp1x_Port *InstancePtr, u8 Offset, u8 *Buf,
		u32 BufSize)
{
	XDprx *HwDp = InstancePtr->PhyIfPtr;
	u32 Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	int NumRead = 0;

	/* Determine RegOffset */
	RegOffset  = XDP_RX_DPCD_HDCP_TABLE;
	RegOffset += Offset;

	/* Iterate through the reads */
	do {
		u32 Value = 0;
		u32 Alignment = 0;
		u32 NumThisTime = 0;
		int Idx = 0;

		/* Determine Alignment */
		Alignment = (RegOffset & 0x03ul);

		/* Determine NumThisTime */
		NumThisTime = 4;
		if (Alignment != 0) {
			NumThisTime = (4 - Alignment);
		}
		if (NumThisTime > BufSize) {
			NumThisTime = BufSize;
		}

		/* Determine Value */
		Value = XDprx_ReadReg(Base, (RegOffset & ~0x03ul));

		/* Check for adjustment of Value */
		if (Alignment != 0)
			Value >>= (8 * Alignment);

		/* Update theBuf */
		for (Idx = 0; Idx < NumThisTime; Idx++) {
			Buf[Idx] = (u8) (Value & 0xFFul);
			Value >>= 8;
		}

		/* Update for loop */
		Buf += NumThisTime;
		BufSize -= NumThisTime;
		RegOffset += NumThisTime;
		NumRead += NumThisTime;
	}
	while (BufSize > 0);

	return (NumRead);
}

/*****************************************************************************/
/**
* This writes a register from the HDCP port device.
*
* @param	InstancePtr is the device to write to.
* @param	Offset is the offset to start writing at.
* @param	Buf is the buffer containing data to write.
* @param	BufSize is the size of the buffer.
*
* @return	The number of bytes written.
*
* @note		None.
*
******************************************************************************/
static int RegWrite(XHdcp1x_Port *InstancePtr, u8 Offset, const u8 *Buf,
		u32 BufSize)
{
	XDprx *HwDp = InstancePtr->PhyIfPtr;
	u32 Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	int NumWritten = 0;

	/* Determine RegOffset */
	RegOffset  = XDP_RX_DPCD_HDCP_TABLE;
	RegOffset += Offset;

	/* Iterate through the writes */
	do {
		u32 Value = 0;
		u32 Alignment = 0;
		u32 NumThisTime = 0;
		int Idx = 0;

		/* Determine Alignment */
		Alignment = (RegOffset & 0x03ul);

		/* Determine NumThisTime */
		NumThisTime = 4;
		if (Alignment != 0) {
			NumThisTime = (4 - Alignment);
		}
		if (NumThisTime > BufSize) {
			NumThisTime = BufSize;
		}

		/* Check for simple case */
		if (NumThisTime == 4) {
			/* Determine Value */
			for (Idx = 3; Idx >= 0; Idx--) {
				Value <<= 8;
				Value |= Buf[Idx];
			}
		}
		/* Otherwise - must read and modify existing memory */
		else {
			u32 Mask = 0;
			u32 Temp = 0;

			/* Determine Mask */
			Mask = 0xFFu;
			if (Alignment != 0) {
				Mask <<= (8 * Alignment);
			}

			/* Initialize Value */
			Value = XDprx_ReadReg(Base, (RegOffset & ~0x03ul));

			/* Update theValue */
			for (Idx = 0; Idx < NumThisTime; Idx++) {
				Temp = Buf[Idx];
				Temp <<= (8 * (Alignment + Idx));
				Value &= ~Mask;
				Value |= Temp;
				Mask <<= 8;
			}
		}

		/* Write Value */
		XDprx_WriteReg(Base, (RegOffset & ~0x03ul), Value);

		/* Update for loop */
		Buf += NumThisTime;
		BufSize -= NumThisTime;
		RegOffset += NumThisTime;
		NumWritten += NumThisTime;
	}
	while (BufSize > 0);

	return (NumWritten);
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
static void ProcessAKsvWrite(void *CallbackRef)
{
	XHdcp1x_Port *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Stats.IntCount++;

	/* Clear bit 0 of  Ainfo register */
	RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO, &Value, 1);
	Value &= 0xFEu;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO, &Value, 1);

	/* Clear bits 3:2 of  Bstatus register */
	RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);
	Value &= 0xF3u;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);

	/* Invoke authentication callback if set */
	if (InstancePtr->IsAuthCallbackSet) {
		(*(InstancePtr->AuthCallback))(InstancePtr->AuthRef);
	}
}

/*****************************************************************************/
/**
* This function process a read of the Ro' register by the tx device.
*
* @param	CallbackRef is the device to whose register was read.
*
* @return	None.
*
* @note		This function initiates the side effects of the tx device read
*		the Ro' register. This is currently limited to the clearing of
*		bits within device's Bstatus register.
*
******************************************************************************/
static void ProcessRoRead(void *CallbackRef)
{
	XHdcp1x_Port *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Stats.IntCount++;

	/* Clear bit 1 of  Bstatus register */
	RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);
	Value &= 0xFDu;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);
}

/*****************************************************************************/
/**
* This function process a read of the Binfo register by the tx device.
*
* @param	CallbackRef is the device to whose register was read.
*
* @return	None.
*
* @note		This function initiates the side effects of the tx device read
*		the Binfo register. This is currently limited to the clearing of
*		bits within device's Bstatus register.
*
******************************************************************************/
static void ProcessBinfoRead(void *CallbackRef)
{
	XHdcp1x_Port *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Stats.IntCount++;

	/* Clear bit 0 of  Bstatus register */
	RegRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);
	Value &= 0xFEu;
	RegWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);
}

/*****************************************************************************/
/**
* This tables defines  adaptor for  DP RX HDCP port driver
*
******************************************************************************/
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpRxAdaptor =
{
	&XHdcp1x_PortDpRxInit,
	&XHdcp1x_PortDpRxEnable,
	&XHdcp1x_PortDpRxDisable,
	&XHdcp1x_PortDpRxRead,
	&XHdcp1x_PortDpRxWrite,
	NULL,
	NULL,
	NULL,
	NULL,
};

#endif
/* defined(XPAR_XDP_RX_NUM_INSTANCES) && (XPAR_XDP_RX_NUM_INSTANCES > 0) */
