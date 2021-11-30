/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_controltransfers.c
* @addtogroup Overview
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16 First release
* 1.3	vak 04/03/17 Added CCI support for USB
* 1.4	bk  12/01/18 Modify USBPSU driver code to fit USB common example code
*		     for all USB IPs.
* 1.4	vak 30/05/18 Removed xusb_wrapper files
* 1.6	pm  28/08/19 Removed 80-character warnings
* 1.7 	pm  23/03/20 Restructured the code for more readability and modularity
* 1.8	pm  24/07/20 Fixed MISRA-C and Coverity warnings
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/
#include "xusbpsu_endpoint.h"
#include "sleep.h"
#include "xusbpsu_local.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* @brief
* Stalls Control Endpoint and restarts to receive Setup packet.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0StallRestart(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_Ep		*Ept;

	Xil_AssertVoid(InstancePtr != NULL);

	/* reinitialize physical ep1 */
	Ept = &InstancePtr->eps[1U];
	Ept->EpStatus = XUSBPSU_EP_ENABLED;

	/* stall is always issued on EP0 */
	XUsbPsu_EpSetStall(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT);

	Ept = &InstancePtr->eps[0U];
	Ept->EpStatus = XUSBPSU_EP_ENABLED;
	InstancePtr->Ep0State = XUSBPSU_EP0_SETUP_PHASE;
	(void)XUsbPsu_RecvSetup(InstancePtr);
}

/****************************************************************************/
/**
* Checks the Data Phase and calls user Endpoint handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0DataDone(struct XUsbPsu *InstancePtr,
				 const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep	*Ept;
	struct XUsbPsu_Trb	*TrbPtr;
	u32	Status;
	u32	Length;
	u32	Epnum;
	u8	Dir;

	Epnum = Event->Epnumber;
	Dir = (u8)(!!Epnum);
	Ept = &InstancePtr->eps[Epnum];
	TrbPtr = &InstancePtr->Ep0_Trb;

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheInvalidateRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
	}

	Status = XUSBPSU_TRB_SIZE_TRBSTS(TrbPtr->Size);
	if (Status == XUSBPSU_TRBSTS_SETUP_PENDING) {
		return;
	}

	Length = TrbPtr->Size & XUSBPSU_TRB_SIZE_MASK;

	if (Length == 0U) {
		Ept->BytesTxed = Ept->RequestedBytes;
	} else {
		if (Dir == XUSBPSU_EP_DIR_IN) {
			Ept->BytesTxed = Ept->RequestedBytes - Length;
		} else {
			if (Ept->UnalignedTx == 1U) {
				Ept->BytesTxed = Ept->RequestedBytes;
				Ept->UnalignedTx = 0U;
			}
		}
	}

	if (Dir == XUSBPSU_EP_DIR_OUT) {
		/* Invalidate Cache */
		if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
			Xil_DCacheInvalidateRange((INTPTR)Ept->BufferPtr,
							 Ept->BytesTxed);
		}
	}

	if (Ept->Handler != NULL) {
		Ept->Handler(InstancePtr->AppData, Ept->RequestedBytes,
							 Ept->BytesTxed);
	}
}

/****************************************************************************/
/**
* Checks the Status Phase and starts next Control transfer.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0StatusDone(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_Trb	*TrbPtr;

	TrbPtr = &InstancePtr->Ep0_Trb;

	if (InstancePtr->IsInTestMode != 0U) {
		s32 Ret;

		Ret = XUsbPsu_SetTestMode(InstancePtr,
					InstancePtr->TestMode);
		if (Ret < 0) {
			XUsbPsu_Ep0StallRestart(InstancePtr);
			return;
		}
	}

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheInvalidateRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
	}

	(void)XUsbPsu_RecvSetup(InstancePtr);
}

/****************************************************************************/
/**
* Starts Status Phase of Control Transfer
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_Ep0StartStatus(struct XUsbPsu *InstancePtr,
				const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep  *Ept;
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Trb *TrbPtr;
	u32 Type;
	s32 Ret;
	u8 Dir;

	Ept = &InstancePtr->eps[Event->Epnumber];
	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);
	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != 0U) {
		return (s32)XST_FAILURE;
	}

	Type = (InstancePtr->IsThreeStage != 0U) ?
					 XUSBPSU_TRBCTL_CONTROL_STATUS3
					: XUSBPSU_TRBCTL_CONTROL_STATUS2;
	TrbPtr = &InstancePtr->Ep0_Trb;
	/* we use same TrbPtr for setup packet */
	TrbPtr->BufferPtrLow = (UINTPTR)&InstancePtr->SetupData;
	TrbPtr->BufferPtrHigh = ((UINTPTR)&InstancePtr->SetupData >> 16U)
								 >> 16U;
	TrbPtr->Size = 0U;
	TrbPtr->Ctrl = Type;

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_LST
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
						 sizeof(struct XUsbPsu_Trb));
	}

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	InstancePtr->Ep0State = XUSBPSU_EP0_STATUS_PHASE;

	/*
	 * Control OUT transfer - Status stage happens on EP0 IN - EP1
	 * Control IN transfer - Status stage happens on EP0 OUT - EP0
	 */
	Dir = !InstancePtr->ControlDir;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, 0U, Dir,
						XUSBPSU_DEPCMD_STARTTRANSFER,
						Params);
	if (Ret != XST_SUCCESS) {
		return (s32)XST_FAILURE;
	}

	Ept->EpStatus |= XUSBPSU_EP_BUSY;
	Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
								Ept->UsbEpNum,
								Ept->Direction);

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* Ends Data Phase - used in case of error.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Ept is a pointer to the Endpoint structure.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0_EndControlData(struct XUsbPsu *InstancePtr,
						struct XUsbPsu_Ep *Ept)
{
	struct XUsbPsu_EpParams *Params;
	u32	Cmd;

	if (Ept->ResourceIndex == 0U) {
		return;
	}

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	Cmd = XUSBPSU_DEPCMD_ENDTRANSFER;
	Cmd |= XUSBPSU_DEPCMD_PARAM(Ept->ResourceIndex);
	(void)XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
						Cmd, Params);
	Ept->ResourceIndex = 0U;
	XUsbPsu_Sleep(200U);
}

/****************************************************************************/
/**
* Enables USB Control Endpoint i.e., EP0OUT and EP0IN of Core.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Size is control endpoint size.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		None.
*
****************************************************************************/
s32 XUsbPsu_EnableControlEp(struct XUsbPsu *InstancePtr, u16 Size)
{
	s32 RetVal;

	Xil_AssertNonvoid((Size >= 64U) && (Size <= 512U));

	RetVal = XUsbPsu_EpEnable(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT, Size,
				XUSBPSU_ENDPOINT_XFER_CONTROL, (u8)FALSE);
	if (RetVal == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	RetVal = XUsbPsu_EpEnable(InstancePtr, 0U, XUSBPSU_EP_DIR_IN, Size,
				XUSBPSU_ENDPOINT_XFER_CONTROL, (u8)FALSE);
	if (RetVal == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}
/** @} */
