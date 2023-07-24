/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc. All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xhdcp1x_port_dp_tx.c
* @addtogroup hdcp1x Overview
* @{
*
* This contains the implementation of the HDCP port driver for DP TX
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
#if defined(XPAR_XDPTXSS_NUM_INSTANCES) && (XPAR_XDPTXSS_NUM_INSTANCES > 0)
#include "xhdcp1x_port.h"
#include "xhdcp1x_port_dp.h"
#include "xdp.h"
#include "xdp_hw.h"
#include "xil_assert.h"
#include "xil_types.h"

/************************** Constant Definitions *****************************/

/* Adaptor definition at the end of this file. */
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpTxAdaptor;

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes *****************************/

static int XHdcp1x_PortDpTxEnable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpTxDisable(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpTxInit(XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpTxIsCapable(const XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpTxIsRepeater(const XHdcp1x *InstancePtr);
static int XHdcp1x_PortDpTxGetRepeaterInfo(const XHdcp1x *InstancePtr,
		u16 *Info);
static int XHdcp1x_PortDpTxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize);
static int XHdcp1x_PortDpTxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize);
static void XHdcp1x_PortDpTxIntrHandler(XHdcp1x *InstancePtr, u32 IntCause);
static void XHdcp1x_CheckForRxStatusChange(XHdcp1x *InstancePtr);

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function enables a HDCP port device.
*
* @param InstancePtr is the id of the device to enable.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_NOT_ENABLE otherwise.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortDpTxEnable(XHdcp1x *InstancePtr)
{
	u8 Value = 0;
	int Status = XST_NOT_ENABLED;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Read anything to ensure that the remote end is present */
	if ((XHdcp1x_PortDpTxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, 1)) > 0) {
		Status = XST_SUCCESS;
	}

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
static int XHdcp1x_PortDpTxDisable(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Nothing to do at this time */

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
static int XHdcp1x_PortDpTxInit(XHdcp1x *InstancePtr)
{
	int Status = XST_SUCCESS;
	u32 Base = 0;
	u32 Value = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->Port.PhyIfPtr != NULL);

	/* Ensure that the dp video path routes through the hdcp core */
	Base = ((XDptx *)InstancePtr->Port.PhyIfPtr)->Config.BaseAddr;
	Value  = XDptx_ReadReg(Base, XDP_TX_HDCP_ENABLE);
	Value |= XDP_TX_HDCP_ENABLE_BYPASS_DISABLE_MASK;
	XDptx_WriteReg(Base, XDP_TX_HDCP_ENABLE, Value);

	/* Disable it */
	if (XHdcp1x_PortDpTxDisable(InstancePtr) != XST_SUCCESS) {
		Status = XST_FAILURE;
	}

	return (Status);
}

/*****************************************************************************/
/**
* This function confirms the presence/capability of the remote HDCP device.
*
* @param	InstancePtr is the device to query.
*
* @return	Truth value.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortDpTxIsCapable(const XHdcp1x *InstancePtr)
{
	u8 Value = 0;
	int IsCapable = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for hdcp capable */
	if (XHdcp1x_PortDpTxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_HDCP_CAPABLE) != 0) {
			IsCapable = TRUE;
		}
	}

	return (IsCapable);
}

/*****************************************************************************/
/**
* This function confirms if the remote HDCP device is a repeater.
*
* @param	InstancePtr is the device to query.
*
* @return	Truth value.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortDpTxIsRepeater(const XHdcp1x *InstancePtr)
{
	u8 Value = 0;
	int IsRepeater = FALSE;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check for repeater */
	if (XHdcp1x_PortDpTxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, 1) > 0) {
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_REPEATER) != 0) {
			IsRepeater = TRUE;
		}
	}

	return (IsRepeater);
}

/*****************************************************************************/
/**
* This function retrieves the repeater information.
*
* @param	InstancePtr is the device to query.
*
* @return
*		- XST_SUCCESS if successful.
*		- XST_DEVICE_BUSY if the device is busy.
*		- XST_NO_FEATURE if feature not part of repeater.
*		- XST_RECV_ERROR if receiver read failed.
*
* @note		None.
*
******************************************************************************/
static int XHdcp1x_PortDpTxGetRepeaterInfo(const XHdcp1x *InstancePtr,
		u16 *Info)
{
	u8 Value = 0;
	int Status = XST_SUCCESS;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Info != NULL);

	/* Read the remote capabilities */
	if (XHdcp1x_PortDpTxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BCAPS,
			&Value, 1) > 0) {
		/* Check for repeater */
		if ((Value & XHDCP1X_PORT_BIT_BCAPS_REPEATER) != 0) {
			/* Read the remote status */
			XHdcp1x_PortDpTxRead(InstancePtr,
				XHDCP1X_PORT_OFFSET_BSTATUS, &Value, 1);

			/* Check for ready */
			if ((Value & XHDCP1X_PORT_BIT_BSTATUS_READY) != 0) {
				u8 Buf[2];
				u16 U16Value = 0;

				/* Read the Binfo */
				XHdcp1x_PortDpTxRead(InstancePtr,
					XHDCP1X_PORT_OFFSET_BINFO, Buf, 2);

				/* Determine U16Value */
				XHDCP1X_PORT_BUF_TO_UINT(U16Value, Buf, 16);

				/* Update Info */
				*Info = (U16Value & 0x0FFFu);
			}
			else {
				Status = XST_DEVICE_BUSY;
			}
		}
		else {
			Status = XST_NO_FEATURE;
		}
	}
	else {
		Status = XST_RECV_ERROR;
	}

	return (Status);
}

/*****************************************************************************/
/**
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
static int XHdcp1x_PortDpTxRead(const XHdcp1x *InstancePtr, u8 Offset,
		void *Buf, u32 BufSize)
{
	XDptx *DpHw = InstancePtr->Port.PhyIfPtr;
	u32 Address = 0;
	int NumRead = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Determine Address */
	Address = Offset;
	Address += 0x68000u;

	/* Read it */
	if (XDp_TxAuxRead(DpHw, Address, BufSize, Buf) == XST_SUCCESS) {
		NumRead = BufSize;
	}

	return (NumRead);
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
static int XHdcp1x_PortDpTxWrite(XHdcp1x *InstancePtr, u8 Offset,
		const void *Buf, u32 BufSize)
{
	XDptx *DpHw = InstancePtr->Port.PhyIfPtr;
	u32 Address = 0;
	int NumWritten = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Truncate if necessary */
	if ((BufSize + Offset) > 0x100u) {
		BufSize = (0x100u - Offset);
	}

	/* Determine Address */
	Address = Offset;
	Address += 0x68000u;

	/* Write it */
	if (XDp_TxAuxWrite(DpHw, Address, BufSize, (u8 *)Buf) == XST_SUCCESS) {
		NumWritten = BufSize;
	}

	return (NumWritten);
}

/*****************************************************************************/
/**
* This handles an interrupt generated by a HDCP port device.
*
* @param	InstancePtr is the device to write to.
* @param	IntCause is the interrupt cause bit map.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_PortDpTxIntrHandler(XHdcp1x *InstancePtr, u32 IntCause)
{
	int HpdDetected = 0;

	/* Verify arguments. */
	Xil_AssertVoid(InstancePtr != NULL);

	/* Determine HpdDetected */
	if (IntCause & XDP_TX_INTERRUPT_STATUS_HPD_PULSE_DETECTED_MASK) {
		HpdDetected = TRUE;
	}
	else if (IntCause & XDP_TX_INTERRUPT_STATUS_HPD_EVENT_MASK) {
		HpdDetected = TRUE;
	}

	/* Check for HPD irq */
	if (HpdDetected) {
		XHdcp1x_CheckForRxStatusChange(InstancePtr);
	}
}

/*****************************************************************************/
/**
* This function checks for a link integrity check failure or re-auth request.
*
* @param	InstancePtr is the device to process the failure.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XHdcp1x_CheckForRxStatusChange(XHdcp1x *InstancePtr)
{
	u8 Value = 0;

	/* Read the Bstatus register */
	if (XHdcp1x_PortDpTxRead(InstancePtr, XHDCP1X_PORT_OFFSET_BSTATUS,
			&Value, 1) > 0) {
		u8 ReauthMask = 0;

		/* Determine ReauthMask */
		ReauthMask  = XHDCP1X_PORT_BIT_BSTATUS_LINK_FAILURE;
		ReauthMask |= XHDCP1X_PORT_BIT_BSTATUS_REAUTH_REQUEST;

		/* Check for link failure or re-authentication requested */
		if ((Value & ReauthMask) != 0) {
			/* Invoke authentication callback if set */
			if (InstancePtr->Port.IsAuthCallbackSet) {
				(*(InstancePtr->Port.AuthCallback))(
					InstancePtr->Port.AuthRef);
			}
		}
	}
}

/*****************************************************************************/
/**
* This tables defines the adaptor for the DP TX HDCP port driver
*
******************************************************************************/
const XHdcp1x_PortPhyIfAdaptor XHdcp1x_PortDpTxAdaptor =
{
	&XHdcp1x_PortDpTxInit,
	&XHdcp1x_PortDpTxEnable,
	&XHdcp1x_PortDpTxDisable,
	&XHdcp1x_PortDpTxRead,
	&XHdcp1x_PortDpTxWrite,
	&XHdcp1x_PortDpTxIsCapable,
	&XHdcp1x_PortDpTxIsRepeater,
	NULL,
	&XHdcp1x_PortDpTxGetRepeaterInfo,
	&XHdcp1x_PortDpTxIntrHandler,
	NULL,
};

#endif
/* defined(XPAR_XDP_TX_NUM_INSTANCES) && (XPAR_XDP_TX_NUM_INSTANCES > 0) */
/** @} */
