/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp1x_port_dp_rx.c
* @addtogroup hdcp1x_v4_6
* @{
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
* 3.0   yas    02/13/16 Upgraded function XHdcp1x_PortDpRxEnable support
*                       HDCP Repeater functionality.
* 3.1   yas    07/28/16 Added fucntion XHdcp1x_PortDpRxSetRepeater
* 4.0   yas    08/16/16 Used UINTPTR instead of u32 for BaseAddress
*                       of DisplayPort DPCD registers
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#if defined(XPAR_XDPRXSS_NUM_INSTANCES) && (XPAR_XDPRXSS_NUM_INSTANCES > 0)
#include <stdlib.h>
#include <string.h>
#include "xhdcp1x_port.h"
#include "xhdcp1x_cipher.h"
#include "xhdcp1x_port_dp.h"
#include "xdp.h"
#include "xdp_hw.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/* Adaptor definition at the end of this file. */
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpRxAdaptor;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes *****************************/

static int XHdcp1x_PortDpRxEnable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpRxDisable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpRxInit(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpRxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize);
static int XHdcp1x_PortDpRxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize);
static int XHdcp1x_PortDpRxSetRepeater(XHdcp1x *InstancePtr, u8 RptrConf);
static void XHdcp1x_PortDpRxProcessAKsvWrite(void *CallbackRef);
static void XHdcp1x_PortDpRxProcessRoRead(void *CallbackRef);
static void XHdcp1x_PortDpRxProcessBinfoRead(void *CallbackRef);

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
static int XHdcp1x_PortDpRxEnable(XHdcp1x *InstancePtr)
{
	XDprx *HwDp = InstancePtr->Port.PhyIfPtr;
	u32 IntMask = 0;
	u8 Buf[4];
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Initialize Buf */
	memset(Buf, 0, 4);

	/* Initialize Bstatus register */
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
			Buf, 1);

	/* Initialize Binfo register */
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BINFO,
			Buf, 2);

	/* Initialize Bcaps register */
	Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_HDCP_CAPABLE;

	/* Checking for Repeater flag */
	if (InstancePtr->IsRepeater) {
		Buf[0] |= XHDCP1X_PORT_BIT_BCAPS_REPEATER;
	}

	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS, Buf, 1);

	/* Checking for Repeater flag */
	if (InstancePtr->IsRepeater) {
		/* Update the Bz register to set the REPATER bit in the cipher*/
		XHdcp1x_WriteReg(InstancePtr->Config.BaseAddress,
			XHDCP1X_CIPHER_REG_CIPHER_Bz,
			HDCP1X_CIPHER_BIT_REPEATER_ENABLE);
	}

	/* Initialize some debug registers */
	Buf[0] = 0xDE;
	Buf[1] = 0xAD;
	Buf[2] = 0xBE;
	Buf[3] = 0xEF;
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_DBG, Buf, 4);

	/* Register callbacks */
	XDp_RxSetCallback(HwDp, XDP_RX_HANDLER_HDCP_AKSV,
		&XHdcp1x_PortDpRxProcessAKsvWrite, InstancePtr);
	XDp_RxSetCallback(HwDp, XDP_RX_HANDLER_HDCP_BINFO,
		&XHdcp1x_PortDpRxProcessBinfoRead, InstancePtr);
	XDp_RxSetCallback(HwDp, XDP_RX_HANDLER_HDCP_RO,
		&XHdcp1x_PortDpRxProcessRoRead, InstancePtr);

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
static int XHdcp1x_PortDpRxDisable(XHdcp1x *InstancePtr)
{
	XDprx *HwDp = InstancePtr->Port.PhyIfPtr;
	u32 IntMask = 0;
	u8 Offset = 0;
	u8 Value = 0;
	int NumLeft = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Disable interrupts */
	IntMask  = XDP_RX_INTERRUPT_MASK_HDCP_AKSV_WRITE_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_RO_READ_MASK;
	IntMask |= XDP_RX_INTERRUPT_MASK_HDCP_BINFO_READ_MASK;
	XDp_RxInterruptDisable(HwDp, IntMask);

	/* Clear hdcp registers */
	Value = 0;
	Offset = 5;
	/* First clear all the HDCP 1.4 registers from the BKSV (0x0 - 0x4,
	 * 5 bytes)to 0x13. Not clearing the KSV FIFO, V' registers
	 * reserved and debug registers.
	 */
	NumLeft = 0x0E;
	while (NumLeft-- >= 0) {
		XHdcp1x_PortDpRxWrite(InstancePtr, Offset++, &Value,
			sizeof(Value));
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
static int XHdcp1x_PortDpRxInit(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

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
static int XHdcp1x_PortDpRxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize)
{
	XDprx *HwDp = InstancePtr->Port.PhyIfPtr;
	UINTPTR Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	int NumRead = 0;
	u8 *ReadBuf = Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Determine RegOffset */
	RegOffset = XDP_RX_DPCD_HDCP_TABLE;
	RegOffset += Offset;

	/* Iterate through the reads */
	do {
		u32 Value = 0;
		u32 Alignment = 0;
		u32 NumThisTime = 0;
		u32 Idx = 0;

		/* Determine Alignment */
		Alignment = (RegOffset & 0x03ul);

		/* Determine NumThisTime */
		NumThisTime = 4;
		if (Alignment) {
			NumThisTime = (4 - Alignment);
		}
		if (NumThisTime > BufSize) {
			NumThisTime = BufSize;
		}

		/* Determine Value */
		Value = XDprx_ReadReg(Base, (RegOffset & ~0x03ul));

		/* Check for adjustment of Value */
		if (Alignment) {
			Value >>= (8 * Alignment);
		}

		/* Update theBuf */
		for (Idx = 0; Idx < NumThisTime; Idx++) {
			ReadBuf[Idx] = (u8)(Value & 0xFFul);
			Value >>= 8;
		}

		/* Update for loop */
		ReadBuf += NumThisTime;
		BufSize -= NumThisTime;
		RegOffset += NumThisTime;
		NumRead += NumThisTime;
	} while (BufSize > 0);

	return (NumRead);
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
static int XHdcp1x_PortDpRxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize)
{
	XDprx *HwDp = InstancePtr->Port.PhyIfPtr;
	UINTPTR Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	int NumWritten = 0;
	const u8 *WriteBuf = Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Determine RegOffset */
	RegOffset = XDP_RX_DPCD_HDCP_TABLE;
	RegOffset += Offset;

	/* Iterate through the writes */
	do {
		u32 Value = 0;
		u32 Alignment = 0;
		int NumThisTime = 0;
		int Idx = 0;

		/* Determine Alignment */
		Alignment = (RegOffset & 0x03ul);

		/* Determine NumThisTime */
		NumThisTime = 4;
		if (Alignment) {
			NumThisTime = (4 - Alignment);
		}
		if (NumThisTime > (int)BufSize) {
			NumThisTime = BufSize;
		}

		/* Check for simple case */
		if (NumThisTime == 4) {
			/* Determine Value */
			for (Idx = 3; Idx >= 0; Idx--) {
				Value <<= 8;
				Value |= WriteBuf[Idx];
			}
		}
		/* Otherwise - must read and modify existing memory */
		else {
			if (Offset == XHDCP1X_PORT_OFFSET_KSVFIFO) {
				for (Idx = (NumThisTime - 1); Idx >= 0; Idx--)
				{
					Value <<= 8;
					Value |= WriteBuf[Idx];
				}
			} else {
				u32 Mask = 0;
				u32 Temp = 0;

				/* Determine Mask */
				Mask = 0xFFu;
				if (Alignment) {
					Mask <<= (8 * Alignment);
				}

				/* Initialize Value */
				Value = XDprx_ReadReg(Base, (RegOffset & ~0x03ul));

				/* Update theValue */
				for (Idx = 0; Idx < NumThisTime; Idx++) {
					Temp = WriteBuf[Idx];
					Temp <<= (8 * (Alignment + Idx));
					Value &= ~Mask;
					Value |= Temp;
					Mask <<= 8;
				}
			}
		}

		/* Write Value */
		XDprx_WriteReg(Base, (RegOffset & ~0x03ul), Value);

		/* Update for loop */
		WriteBuf += NumThisTime;
		BufSize -= NumThisTime;
		if (Offset != XHDCP1X_PORT_OFFSET_KSVFIFO) {
			RegOffset += NumThisTime;
		}
		NumWritten += NumThisTime;
	} while (BufSize > 0);

	return (NumWritten);
}

/*****************************************************************************/
/**
* This function set the REPEATER bit in the BCaps of the device.
*
* @param	InstancePtr is the device to write to.
* @param	RptrConf is the repeater capability for the device.
*
* @return	XST_SUCCESS.
*
* @note		This function sets the REPEATER bit in the BCaps register for the
* 		upstream device to read. This can be used to update the device
* 		configuration if it changes in real time.
*
******************************************************************************/
static int XHdcp1x_PortDpRxSetRepeater(XHdcp1x *InstancePtr, u8 RptrConf)
{
	u8 Value = 0;

	/* Set the Ready bit in the BCaps Register */
	XHdcp1x_PortDpRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, XHDCP1X_PORT_SIZE_BCAPS);
	if(RptrConf) {
		Value |= XHDCP1X_PORT_BIT_BCAPS_REPEATER;
	}
	else {
		Value &= ~XHDCP1X_PORT_BIT_BCAPS_REPEATER;
	}
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, XHDCP1X_PORT_SIZE_BCAPS);

	return(XST_SUCCESS);
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
*		writing the Aksv register. This is currently updates some
*		status bits as well as kick starts a re-authentication
*		process.
*
******************************************************************************/
static void XHdcp1x_PortDpRxProcessAKsvWrite(void *CallbackRef)
{
	XHdcp1x *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Port.Stats.IntCount++;

	/* Clear bit 0 of  Ainfo register */
	XHdcp1x_PortDpRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO,
						&Value, 1);
	Value &= 0xFEu;
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_AINFO,
						&Value, 1);

	/* Clear bits 3:2 of  Bstatus register */
	XHdcp1x_PortDpRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
						&Value, 1);
	Value &= 0xF3u;
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
						&Value, 1);

	/* Invoke authentication callback if set */
	if (InstancePtr->Port.IsAuthCallbackSet) {
		(*(InstancePtr->Port.AuthCallback))(InstancePtr->Port.AuthRef);
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
static void XHdcp1x_PortDpRxProcessRoRead(void *CallbackRef)
{
	XHdcp1x *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Port.Stats.IntCount++;

	/* Clear bit 1 of  Bstatus register */
	XHdcp1x_PortDpRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
					&Value, 1);
	Value &= 0xFDu;
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
					&Value, 1);
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
*		the Binfo register. This is currently limited to the clearing
*		of bits within device's Bstatus register.
*
******************************************************************************/
static void XHdcp1x_PortDpRxProcessBinfoRead(void *CallbackRef)
{
	XHdcp1x *InstancePtr = CallbackRef;
	u8 Value = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Update statistics */
	InstancePtr->Port.Stats.IntCount++;

	/* Clear bit 0 of Bstatus register */
	XHdcp1x_PortDpRxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
					&Value, 1);
	Value &= 0xFEu;
	XHdcp1x_PortDpRxWrite(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
					&Value, 1);
}

/*****************************************************************************/
/**
* This tables defines adaptor for DP RX HDCP port driver
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
	&XHdcp1x_PortDpRxSetRepeater,
	NULL,
	NULL,
	NULL,
};

#endif
/* defined(XPAR_XDP_RX_NUM_INSTANCES) && (XPAR_XDP_RX_NUM_INSTANCES > 0) */
/** @} */
