/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdptxss_hdcp22.c
*
* This file contains a minimal set of functions for the High-Bandwidth Content
* Protection core to configure.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 jb  02/21/19 Initial release.
* </pre>
******************************************************************************/

/****************************** Include Files ********************************/
#include "xdptxss_hdcp22.h"
#include "xdptxss.h"


/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/
#define XDP_REMOTE_RX_HDCP22_DPCD_OFFSET	0x69000u


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
* This function reads a register from a HDCP22 port device.
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
static int XHdcp22_PortDpTxRead(void *InstancePtr, u32 Offset,
		void *Buf, u32 BufSize)
{
	XDp *DpHw = (XDp *)InstancePtr;
	u32 Address = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine Address */
	Address = Offset;
	Address += XDP_REMOTE_RX_HDCP22_DPCD_OFFSET;

	/* Read it */
	if (XDp_TxAuxRead(DpHw, Address, BufSize, Buf) == XST_SUCCESS) {
		return BufSize;
	}

	return 0;
}

/*****************************************************************************/
/**
* This function writes a register from a HDCP22 port device.
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
static int XHdcp22_PortDpTxWrite(void *InstancePtr, u32 Offset,
		const void *Buf, u32 BufSize)
{
	XDp *DpHw = (XDp *)InstancePtr;
	u32 Address = 0;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine Address */
	Address = Offset;
	Address += XDP_REMOTE_RX_HDCP22_DPCD_OFFSET;

	/* Write it */
	if (XDp_TxAuxWrite(DpHw, Address, BufSize, (u8 *)Buf) == XST_SUCCESS) {
		return BufSize;
	}

	return 0;
}

/*****************************************************************************/
/**
*
* This function clears all pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
*
* @return None
*
* @note   None.
*
******************************************************************************/
static void XDpTxSs_HdcpClearEvents(XDpTxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	InstancePtr->HdcpEventQueue.Head = 0;
	InstancePtr->HdcpEventQueue.Tail = 0;
}

/*****************************************************************************/
/**
*
* This function gets an event from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
*
* @return When the queue is filled, the next event is returned.
*         When the queue is empty, XDPTXSS_HDCP_NO_EVT is returned.
*
* @note   None.
*
******************************************************************************/
static XDpTxSs_HdcpEvent XDpTxSs_HdcpGetEvent(XDpTxSs *InstancePtr)
{
	XDpTxSs_HdcpEvent Event = 0;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there are any events in the queue */
	if (InstancePtr->HdcpEventQueue.Tail ==
			InstancePtr->HdcpEventQueue.Head) {
		return XDPTXSS_HDCP_NO_EVT;
	}

	Event = InstancePtr->HdcpEventQueue.Queue[
		InstancePtr->HdcpEventQueue.Tail];

	/* Update tail pointer */
	if (InstancePtr->HdcpEventQueue.Tail ==
			(XDPTXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
		InstancePtr->HdcpEventQueue.Tail = 0;
	} else {
		InstancePtr->HdcpEventQueue.Tail++;
	}

	return Event;
}

/*****************************************************************************/
/**
*
* This function processes pending events from the HDCP event queue.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return None
*
* @note   None.
*
******************************************************************************/
static void XDpTxSs_HdcpProcessEvents(XDpTxSs *InstancePtr)
{
	XDpTxSs_HdcpEvent Event;

	/* Verify argument. */
	Xil_AssertVoid(InstancePtr != NULL);

	Event = XDpTxSs_HdcpGetEvent(InstancePtr);
	switch (Event) {
		/* Connect */
		case XDPTXSS_HDCP_CONNECT_EVT :
			break;

		/* Disconnect, Reset the protocol */
		case XDPTXSS_HDCP_DISCONNECT_EVT :
			XDpTxSs_HdcpReset(InstancePtr);
			break;

		/* Authenticate */
		case XDPTXSS_HDCP_AUTHENTICATE_EVT :
			XDpTxSs_Authenticate(InstancePtr);
			break;

		default :
			break;
	}
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  DpTxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XDpTxSs_SubcoreInitHdcp22(void *InstancePtr)
{
	int Status;
	XHdcp22_Tx_Dp_Config *Hdcp22TxConfig;
	XDpTxSs *DpTxSsPtr = (XDpTxSs *)InstancePtr;

	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is the HDCP 2.2 TX present? */
	if (DpTxSsPtr->Hdcp22Ptr) {
		/* Is the key loaded? */
		if (DpTxSsPtr->Hdcp22Lc128Ptr && DpTxSsPtr->Hdcp22SrmPtr) {
			/* Get core configuration */
			/* Initialize HDCP 2.2 TX */
			Hdcp22TxConfig = XHdcp22Tx_Dp_LookupConfig(
					DpTxSsPtr->Config.Hdcp22SubCore.
					Hdcp22Config.DeviceId);
			if (Hdcp22TxConfig == NULL) {
				xdbg_printf(XDBG_DEBUG_GENERAL,
						"DPTXSS ERR:: HDCP 2.2 device"
						"not found\r\n");
				return XST_FAILURE;
			}

			/* Calculate absolute base address of HDCP22 sub-core */
			DpTxSsPtr->Config.Hdcp22SubCore.Hdcp22Config.AbsAddr +=
				DpTxSsPtr->Config.BaseAddress;

			/* HDCP22 config initialize */
			Hdcp22TxConfig->BaseAddress +=
				DpTxSsPtr->Config.BaseAddress;

			Status = XHdcp22Tx_Dp_CfgInitialize(DpTxSsPtr->Hdcp22Ptr,
					Hdcp22TxConfig,
					DpTxSsPtr->Config.Hdcp22SubCore.
					Hdcp22Config.AbsAddr);

			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,
						"DPTXSS ERR:: HDCP 2.2 "
						"Initialization failed\r\n");
				return Status;
			}

			/* Initialize HDCP22 timer instance
			 * with DP timer instance*/
			if (DpTxSsPtr->TmrCtrPtr) {
				XHdcp22Tx_Dp_timer_attach(DpTxSsPtr->Hdcp22Ptr,
						DpTxSsPtr->TmrCtrPtr);
			} else {
				xdbg_printf(XDBG_DEBUG_GENERAL,
						"DPTXSS ERR:: Timer attaching"
						" for HDCP22 TX is failed\r\n");
				return XST_FAILURE;
			}

			/*Register DP AUX read write Handlers*/
			/* Set-up the DDC Handlers */
			XHdcp22Tx_Dp_SetCallback(DpTxSsPtr->Hdcp22Ptr,
					XHDCP22_TX_HANDLER_DP_AUX_READ,
					(void *)XHdcp22_PortDpTxRead,
					(void *)DpTxSsPtr->DpPtr);
			XHdcp22Tx_Dp_SetCallback(DpTxSsPtr->Hdcp22Ptr,
					XHDCP22_TX_HANDLER_DP_AUX_WRITE,
					(void *)XHdcp22_PortDpTxWrite,
					(void *)DpTxSsPtr->DpPtr);

			/* Set polling value */
			XHdcp22Tx_Dp_SetMessagePollingValue(
					DpTxSsPtr->Hdcp22Ptr, 10);

			XHdcp22Tx_Dp_LogReset(DpTxSsPtr->Hdcp22Ptr,
					FALSE);

			/* Load key */
			XHdcp22Tx_Dp_LoadLc128(DpTxSsPtr->Hdcp22Ptr,
					DpTxSsPtr->Hdcp22Lc128Ptr);

			/* Load SRM */
			Status = XHdcp22Tx_Dp_LoadRevocationTable(
					DpTxSsPtr->Hdcp22Ptr,
					DpTxSsPtr->Hdcp22SrmPtr);
			if (Status != XST_SUCCESS) {
				xdbg_printf(XDBG_DEBUG_GENERAL,
						"DPTXSS ERR:: HDCP 2.2 "
						"failed to load SRM\r\n");
				return Status;
			}

			/*Clear the HDCP22 event queue */
			XDpTxSs_HdcpClearEvents(DpTxSsPtr);
		} else {
			xdbg_printf(XDBG_DEBUG_GENERAL,
					"DPTXSS ERR:: HDCP22 keys"
					" have not loaded\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is to poll the HDCP22 Tx core.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XDpTxSs_HdcpPoll(void *Instance)
{
	XDpTxSs *InstancePtr = (XDpTxSs *)Instance;

	/* Verify argument. */
	Xil_AssertNonvoid(Instance != NULL);

	/* Only poll when the HDCP is ready */
	if (InstancePtr->HdcpIsReady) {

		/* Process any pending events from the TX event queue */
		XDpTxSs_HdcpProcessEvents(InstancePtr);

		/* HDCP 2.2 */
		if (InstancePtr->Hdcp22Ptr) {
			if (XHdcp22Tx_Dp_IsEnabled(InstancePtr->Hdcp22Ptr)) {
				XHdcp22Tx_Dp_Poll(InstancePtr->Hdcp22Ptr);
			}
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function pushes an event into the HDCP event queue.
*
* @param InstancePtr is a pointer to the XV_HdmiTxSs instance.
* @param Event is the event to be pushed in the queue.
*
* @return
*  - XST_SUCCESS if action was successful
*  - XST_FAILURE if action was not successful
*
* @note   None.
*
******************************************************************************/
int XDpTxSs_HdcpPushEvent(void *Instance,
		XDpTxSs_HdcpEvent Event)
{
	XDpTxSs *InstancePtr = (XDpTxSs *)Instance;

	/* Verify argument. */
	Xil_AssertNonvoid(Instance != NULL);
	Xil_AssertNonvoid(Event < XDPTXSS_HDCP_INVALID_EVT);

	/* Write event into the queue */
	InstancePtr->HdcpEventQueue.Queue[InstancePtr->HdcpEventQueue.Head] =
		Event;

	/* Update head pointer */
	if (InstancePtr->HdcpEventQueue.Head ==
			(XDPTXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
		InstancePtr->HdcpEventQueue.Head = 0;
	} else {
		InstancePtr->HdcpEventQueue.Head++;
	}

	/* Check tail pointer. When the two pointer are equal, then the buffer
	 * is full. In this case then increment the tail pointer as well to
	 * remove the oldest entry from the buffer.
	 */
	if (InstancePtr->HdcpEventQueue.Tail ==
			InstancePtr->HdcpEventQueue.Head)
	{
		if (InstancePtr->HdcpEventQueue.Tail ==
				(XDPTXSS_HDCP_MAX_QUEUE_SIZE - 1)) {
			InstancePtr->HdcpEventQueue.Tail = 0;
		} else {
			InstancePtr->HdcpEventQueue.Tail++;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function determines if the connected DP sink is HDCP 2.2 capable.
*
* @param InstancePtr is a pointer to the XDpTxSs instance.
*
* @return
*  - TRUE if sink is HDCP 2.2 capable and ready to authenticate.
*  - FALSE if sink does not support HDCP 2.2 or is not ready.
*
******************************************************************************/
u8 XDpTxSs_IsSinkHdcp22Capable(void *Instance)
{
	XDpTxSs *InstancePtr = (XDpTxSs *)Instance;

	/* Verify argument. */
	Xil_AssertNonvoid(Instance != NULL);

	if (InstancePtr->Hdcp22Ptr) {
		if (XHdcp22Tx_Dp_IsDwnstrmCapable(InstancePtr->Hdcp22Ptr)) {
			return TRUE;
		}
	}

	return FALSE;
}
