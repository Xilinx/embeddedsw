/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdprxss_hdcp22.c
*
* This file contains a minimal set of functions for the High-Bandwidth Content
* Protection core to configure.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver  Who Date     Changes
* ---- --- -------- -----------------------------------------------------
* 1.00 jb  02/18/19 Initial release.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xdprxss_hdcp22.h"
#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)
#include "xdprxss.h"
/************************** Constant Definitions *****************************/


/***************** Macros (Inline Functions) Definitions *********************/


/**************************** Type Definitions *******************************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* This function is called when the DP-RX HDCP22 Ake_Init write interrupt has
* occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XHdcp22_PortDpRxProcessAkeInit(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/*Set Ake_Init message received event in HDCP22 rx instance*/
	if (DpRxSsPtr->Hdcp22Ptr) {

		/* Set Active protocol to HDCP22 and Enable it */
		XDpRxSs_HdcpDisable(DpRxSsPtr);
		XDpRxSs_HdcpSetProtocol(DpRxSsPtr,XDPRXSS_HDCP_22);
		XDpRxSs_HdcpEnable(DpRxSsPtr);

		/*Reset HDCP FIFOs as we have got a new authentication request*/
		for(u8 Index = 0; Index < 2; Index++) {
			XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
					XDP_RX_SOFT_RESET,
					XDP_RX_SOFT_RESET_HDCP22_MASK);
			XDp_WriteReg(DpRxSsPtr->DpPtr->Config.BaseAddr,
					XDP_RX_SOFT_RESET, 0);
		}

		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
				XDPRX_HDCP22_RX_DPCD_FLAG_AKE_INIT_RCVD);
	}
}

/*****************************************************************************/
/**
* This function is called when the DP-RX HDCP22 Ake_No_Stored_Km write
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XHdcp22_PortDpRxProcessAkeNoStoredKm(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/*Set Ake_No_Stored_Km message received event in HDCP22 rx instance*/
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
			XDPRX_HDCP22_RX_DPCD_FLAG_AKE_NO_STORED_KM_RCVD);
	}
}

/*****************************************************************************/
/**
 * This function is called when the DP-RX HDCP22 Ake_Stored_Km write
 * interrupt has occurred.
 *
 * @param RefPtr is a callback reference to the HDCP22 RX instance.
 * @param Type indicates the cause of the interrupt.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
static void XHdcp22_PortDpRxProcessAkeStoredKm(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/*Set Ake_Stored_Km message received event in HDCP22 rx instance*/
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
				XDPRX_HDCP22_RX_DPCD_FLAG_AKE_STORED_KM_RCVD);
	}
}

/*****************************************************************************/
/**
 * This function is called when the DP-RX HDCP22 Lc_Init write
 * interrupt has occurred.
 *
 * @param RefPtr is a callback reference to the HDCP22 RX instance.
 * @param Type indicates the cause of the interrupt.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
static void XHdcp22_PortDpRxProcessLcInit(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/*Set Lc_Init message received event in HDCP22 rx instance*/
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
				XDPRX_HDCP22_RX_DPCD_FLAG_LC_INIT_RCVD);
	}
}

/*****************************************************************************/
/**
* This function is called when the DP-RX HDCP22 Ske_Send_Eks write
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XHdcp22_PortDpRxProcessSkeSendEks(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/*Set Ske_Send_Eks message received event in HDCP22 rx instance*/
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
				XDPRX_HDCP22_RX_DPCD_FLAG_SKE_SEND_EKS_RCVD);
	}
}

/*****************************************************************************/
/**
* This function is called when the DP-RX HDCP22 Hprime Read complete
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XHdcp22_PortDpRxProcessHprimeReadDone(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/* Set H' Read complete event in HDCP22 rx instance */
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
				XDPRX_HDCP22_RX_DPCD_FLAG_HPRIME_READ_DONE);
	}
}

/*****************************************************************************/
/**
* This function is called when the DP-RX HDCP22 Pairing Info Read complete
* interrupt has occurred.
*
* @param RefPtr is a callback reference to the HDCP22 RX instance.
* @param Type indicates the cause of the interrupt.
*
* @return None.
*
* @note   None.
******************************************************************************/
static void XHdcp22_PortDpRxProcessPairingReadDone(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);
	/* There is no processing is dependent on Pairing read done interrupt
	 * event, so no need to set the interrupt event
	 * */
}

/*****************************************************************************/
/**
 * This function is called when the DP-RX HDCP22 Stream Type write
 * interrupt has occurred.
 *
 * @param RefPtr is a callback reference to the HDCP22 RX instance.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
static void XHdcp22_PortDpRxProcessStreamType(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/* Set Stream Type in HDCP22 rx */
	if (DpRxSsPtr->Hdcp22Ptr)
		XHdcp22_RxSetStreamType(DpRxSsPtr->Hdcp22Ptr);
}

/*****************************************************************************/
/**
 * This function is called when the DP-RX HDCP22 Repeater ReceiverId List write
 * interrupt has occurred.
 *
 * @param RefPtr is a callback reference to the HDCP22 RX instance.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
static void XHdcp22_PortDpRxProcessRepeaterAuthRcvIdLstDone(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/* Set Repeater Rceiver ID List Ack Read complete event in HDCP22 rx
	 * instance */
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
			XDPRX_HDCP22_RX_DPCD_FLAG_RPTR_RCVID_LST_ACK_READ_DONE);
	}
}

/*****************************************************************************/
/**
 * This function is called when the DP-RX HDCP22 Repeater Stream Manage write
 * interrupt has occurred.
 *
 * @param RefPtr is a callback reference to the HDCP22 RX instance.
 *
 * @return None.
 *
 * @note   None.
 ******************************************************************************/
static void XHdcp22_PortDpRxProcessRepeaterAuthStreamManageDone(void *RefPtr)
{
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)RefPtr;

	Xil_AssertVoid(RefPtr);

	/* Set Repeater Rceiver ID List Ack Read complete event in HDCP22 rx
	 * instance */
	if (DpRxSsPtr->Hdcp22Ptr) {
		XHdcp22Rx_SetDpcdMsgRdWrtAvailable(DpRxSsPtr->Hdcp22Ptr,
			XDPRX_HDCP22_RX_DPCD_FLAG_RPTR_STREAM_MANAGE_READ_DONE);
	}
}
/*****************************************************************************/
/**
*
* This function reads a register from a HDCP22 port device.
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
static u32 XHdcp22_DpRx_AuxReadHandler(void *InstancePtr, u32 Offset,
		void *Buf, u32 BufSize)
{
	XDp *HwDp = (XDp *)InstancePtr;
	UINTPTR Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	u32 NumRead = 0;
	u8 *ReadBuf = (u8 *)Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine RegOffset */
	RegOffset = XDP_RX_DPCD_HDCP22_TABLE;
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

	return NumRead;
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
static u32 XHdcp22_DpRx_AuxWriteHandler(void *InstancePtr, u32 Offset,
		const void *Buf, u32 BufSize)
{
	XDp *HwDp = (XDp *)InstancePtr;
	UINTPTR Base = HwDp->Config.BaseAddr;
	u32 RegOffset = 0;
	u32 NumWritten = 0;
	const u8 *WriteBuf = Buf;

	/* Verify arguments. */
	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Buf != NULL);

	/* Determine RegOffset */
	RegOffset = XDP_RX_DPCD_HDCP22_TABLE;
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
		} else {
			/* Otherwise - must read and modify existing memory */
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
		/* Write Value */
		XDprx_WriteReg(Base, (RegOffset & ~0x03ul), Value);

		/* Update for loop */
		WriteBuf += NumThisTime;
		BufSize -= NumThisTime;
		RegOffset += NumThisTime;
		NumWritten += NumThisTime;
	} while (BufSize > 0);

	return NumWritten;
}

/*****************************************************************************/
/**
* This function is to raise CP_IRQ.
*
* @param	InstancePtr is the device to write to.
* @return	None
*
* @note		None.
*
******************************************************************************/
static void XHdcp22_DpRx_CpIrqSetHandler(void *InstancePtr)
{
	Xil_AssertVoid(InstancePtr);

	XDp_GenerateCpIrq(InstancePtr);
}

/*****************************************************************************/
/**
* This function is to set AUX_DEFFERS for HDCP22 msgs.
*
* @param	InstancePtr is the device to write to.
* @param	setClr is to enable or disable AUX_DEFFERS.
*
* @return	None
*
* @note		None.
*
******************************************************************************/
static void XHdcp22_DpRx_AuxDefferSetClrHandler(void *InstancePtr, u8 SetClr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr);

	return XDp_EnableDisableHdcp22AuxDeffers(InstancePtr, SetClr);
}

/*****************************************************************************/
/**
*
* This function clears all pending events from the HDCP22 event queue.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None
*
* @note   None.
*
******************************************************************************/
static void XDpRxSs_Hdcp22ClearEvents(XDpRxSs *InstancePtr)
{
	/* Verify argument. */
	Xil_AssertVoid(InstancePtr);

	InstancePtr->Hdcp22EventQueue.Head = 0;
	InstancePtr->Hdcp22EventQueue.Tail = 0;
}

/*****************************************************************************/
/**
*
* This function gets an event from the HDCP22 event queue.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return When the queue is filled, the next event is returned.
*         When the queue is empty, XDPRXSS_HDCP22_NO_EVT is returned.
*
* @note   None.
*
******************************************************************************/
static XDpRxSs_Hdcp22Event XDpRxSs_Hdcp22GetEvent(XDpRxSs *InstancePtr)
{
	XDpRxSs_Hdcp22Event Event;

	/* Verify argument. */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Check if there are any events in the queue */
	if (InstancePtr->Hdcp22EventQueue.Tail ==
			InstancePtr->Hdcp22EventQueue.Head) {
		return XDPRXSS_HDCP22_NO_EVT;
	}

	Event = InstancePtr->Hdcp22EventQueue.Queue[InstancePtr->
		Hdcp22EventQueue.Tail];

	/* Update tail pointer */
	if (InstancePtr->Hdcp22EventQueue.Tail ==
			(XDPRXSS_HDCP22_MAX_QUEUE_SIZE - 1)) {
		InstancePtr->Hdcp22EventQueue.Tail = 0;
	}
	else {
		InstancePtr->Hdcp22EventQueue.Tail++;
	}

	return Event;
}

/*****************************************************************************/
/**
*
* This function processes pending events from the HDCP22 event queue.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return	None
*
* @note   None.
*
******************************************************************************/
static void XDpRxSs_Hdcp22ProcessEvents(XDpRxSs *InstancePtr)
{
	XDpRxSs_Hdcp22Event Event;

	/* Verify argument */
	Xil_AssertVoid(InstancePtr);

	Event = XDpRxSs_Hdcp22GetEvent(InstancePtr);

	switch (Event) {

		/* Connect */
		case XDPRXSS_HDCP22_CONNECT_EVT :
			XDpRxSs_HdcpSetProtocol(InstancePtr,
					InstancePtr->HdcpProtocol);
			break;

		default :
			break;
	}
}

/*****************************************************************************/
/**
* This function initializes the included sub-core to it's static configuration
*
* @param  DpRxSsPtr is a pointer to the Subsystem instance to be worked on.
*
* @return XST_SUCCESS/XST_FAILURE
*
******************************************************************************/
int XDpRxSs_SubcoreInitHdcp22(void *InstancePtr)
{
	int Status;
	XHdcp22_Rx_Config *ConfigPtr;
	XDpRxSs *DpRxSsPtr = (XDpRxSs *)InstancePtr;

	/* Verify argument */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/* Is the HDCP 2.2 RX present? */
	if (DpRxSsPtr->Hdcp22Ptr) {

		/* Are the keys loaded? */
		if (DpRxSsPtr->Hdcp22Lc128Ptr && DpRxSsPtr->Hdcp22PrivateKeyPtr) {

			/* Get core configuration */
			ConfigPtr  = XHdcp22Rx_LookupConfig(
					DpRxSsPtr->Config.Hdcp22SubCore.
					Hdcp22Config.DeviceId);
			if (ConfigPtr == NULL)
			{
				xdbg_printf(XDBG_DEBUG_GENERAL,
					"DPRXSS ERR:: HDCP 2.2 device not found"
					"\r\n");
				return XST_FAILURE;
			}

			/* Calculate absolute base address of HDCP22 sub-core */
			DpRxSsPtr->Config.Hdcp22SubCore.Hdcp22Config.AbsAddr +=
				DpRxSsPtr->Config.BaseAddress;

			/* HDCP22 config initialize */
			ConfigPtr->BaseAddress += DpRxSsPtr->Config.BaseAddress;

			/* Initialize core */
			Status = XHdcp22Rx_CfgInitialize(DpRxSsPtr->Hdcp22Ptr,
					ConfigPtr,
					DpRxSsPtr->Config.Hdcp22SubCore.
					Hdcp22Config.AbsAddr);
			if (Status != XST_SUCCESS)
			{
				xdbg_printf(XDBG_DEBUG_GENERAL,
					"DPRXSS ERR:: HDCP 2.2 Initialization"
					" failed\r\n");
				return XST_FAILURE;
			}

			/* Initialize HDCP22 timer instance with
			 * DP timer instance*/
			if (DpRxSsPtr->TmrCtrPtr) {
				XHdcp22_timer_attach(DpRxSsPtr->Hdcp22Ptr,
						DpRxSsPtr->TmrCtrPtr);
			}

			/* Set-up the AUX read/write Handlers these Handlers
			 * will be used to read RX's local DPCD registers for
			 * HDCP port*/
			XHdcp22Rx_SetCallback(DpRxSsPtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_DP_AUX_READ,
					(void *)XHdcp22_DpRx_AuxReadHandler,
					(void *)DpRxSsPtr->DpPtr);
			XHdcp22Rx_SetCallback(DpRxSsPtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_DP_AUX_WRITE,
					(void *)XHdcp22_DpRx_AuxWriteHandler,
					(void *)DpRxSsPtr->DpPtr);
			XHdcp22Rx_SetCallback(DpRxSsPtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_DP_CP_IRQ_SET,
					(void *)XHdcp22_DpRx_CpIrqSetHandler,
					(void *)DpRxSsPtr->DpPtr);
			XHdcp22Rx_SetCallback(DpRxSsPtr->Hdcp22Ptr,
					XHDCP22_RX_HANDLER_DP_AUX_DEFER_SET_CLR,
					(void *)XHdcp22_DpRx_AuxDefferSetClrHandler,
					(void *)DpRxSsPtr->DpPtr);

			/* Register callbacks */
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_AKE_INIT,
					&XHdcp22_PortDpRxProcessAkeInit,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_AKE_NO_STORED_KM,
					&XHdcp22_PortDpRxProcessAkeNoStoredKm,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_AKE_STORED_KM,
					&XHdcp22_PortDpRxProcessAkeStoredKm,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_LC_INIT,
					&XHdcp22_PortDpRxProcessLcInit,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_SKE_SEND_EKS,
					&XHdcp22_PortDpRxProcessSkeSendEks,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_HPRIME_READ_DONE,
					&XHdcp22_PortDpRxProcessHprimeReadDone,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_PAIRING_READ_DONE,
					&XHdcp22_PortDpRxProcessPairingReadDone,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_STREAM_TYPE,
					&XHdcp22_PortDpRxProcessStreamType,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_REPEAT_AUTH_RCVID_LST_DONE,
					&XHdcp22_PortDpRxProcessRepeaterAuthRcvIdLstDone,
					DpRxSsPtr);
			XDp_RxSetCallback(DpRxSsPtr->DpPtr,
					XDP_RX_HANDLER_HDCP22_REPEAT_AUTH_STREAM_MANAGE_DONE,
					&XHdcp22_PortDpRxProcessRepeaterAuthStreamManageDone,
					DpRxSsPtr);

			/* Load Production Keys */
			XHdcp22Rx_LoadLc128(DpRxSsPtr->Hdcp22Ptr,
					DpRxSsPtr->Hdcp22Lc128Ptr);
			XHdcp22Rx_LoadPublicCert(DpRxSsPtr->Hdcp22Ptr,
					DpRxSsPtr->Hdcp22PrivateKeyPtr+40);
			XHdcp22Rx_LoadPrivateKey(DpRxSsPtr->Hdcp22Ptr,
					DpRxSsPtr->Hdcp22PrivateKeyPtr+562);

			/*Clear the HDCP22 event queue */
			XDpRxSs_Hdcp22ClearEvents(DpRxSsPtr);
		} else {
			xdbg_printf(XDBG_DEBUG_GENERAL,
				"DPRXSS ERR:: HDCP22 keys have not loaded\n\r");
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function is to poll the HDCP22 Rx core.
*
* @param InstancePtr is a pointer to the XDpRxSs instance.
*
* @return
*
* @note   None.
*
******************************************************************************/
void XDpRxSs_Hdcp22Poll(void *Instance)
{
	/* Verify argument. */
	Xil_AssertVoid(Instance);

	XDpRxSs *InstancePtr = (XDpRxSs *)Instance;

	/* Only poll when the HDCP is ready */
	if (InstancePtr->HdcpIsReady) {

		/* Process any pending events from the RX event queue */
		XDpRxSs_Hdcp22ProcessEvents(InstancePtr);

		/* HDCP 2.2 */
		if (InstancePtr->Hdcp22Ptr) {
			if (XHdcp22Rx_IsEnabled(InstancePtr->Hdcp22Ptr)) {
				XHdcp22Rx_Poll(InstancePtr->Hdcp22Ptr);
			}
		}
	}
}

#endif /*#if (XPAR_XHDCP22_RX_NUM_INSTANCES > 0)*/
