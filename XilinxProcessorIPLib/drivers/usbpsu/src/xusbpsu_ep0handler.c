/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_ep0handler.c
* @addtogroup usbpsu USBPSU v1.12
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pm  03/03/20 First release
* 1.8	pm  24/07/20 Fixed MISRA-C and Coverity warnings
* 1.12	pm  10/08/22 Update doxygen tag and addtogroup version
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/
#include "xusbpsu_endpoint.h"
#include "xusbpsu_local.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/****************************************************************************/
/**
* Initiates DMA on Control Endpoint 0 to receive Setup packet.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_RecvSetup(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Trb	*TrbPtr;
	struct XUsbPsu_Ep	*Ept;
	s32	Ret;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	/* Setup packet always on EP0 */
	Ept = &InstancePtr->eps[0U];
	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != 0U) {
		return (s32)XST_FAILURE;
	}

	TrbPtr = &InstancePtr->Ep0_Trb;

	TrbPtr->BufferPtrLow = (UINTPTR)&InstancePtr->SetupData;
	TrbPtr->BufferPtrHigh = ((UINTPTR)&InstancePtr->SetupData >> 16U)
									 >> 16U;
	TrbPtr->Size = 8U;
	TrbPtr->Ctrl = XUSBPSU_TRBCTL_CONTROL_SETUP;

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_LST
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
						 sizeof(struct XUsbPsu_Trb));
		Xil_DCacheFlushRange((UINTPTR)&InstancePtr->SetupData,
						 sizeof(SetupPacket));
	}

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	InstancePtr->Ep0State = XUSBPSU_EP0_SETUP_PHASE;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT,
				XUSBPSU_DEPCMD_STARTTRANSFER, Params);
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
* Handles Transfer complete event of Control Endpoints EP0 OUT and EP0 IN.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0XferComplete(struct XUsbPsu *InstancePtr,
				 const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep *Ept;
	SetupPacket *Ctrl;
	u16 Length;

	Ept = &InstancePtr->eps[Event->Epnumber];
	Ctrl = &InstancePtr->SetupData;

	Ept->EpStatus &= ~XUSBPSU_EP_BUSY;
	Ept->ResourceIndex = 0U;

	switch (InstancePtr->Ep0State) {
	case XUSBPSU_EP0_SETUP_PHASE:
		if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
			Xil_DCacheInvalidateRange(
					(INTPTR)&InstancePtr->SetupData,
					sizeof(InstancePtr->SetupData));
		}
		Length = Ctrl->wLength;
		if (Length == 0U) {
			InstancePtr->IsThreeStage = 0U;
			InstancePtr->ControlDir = XUSBPSU_EP_DIR_OUT;
		} else {
			InstancePtr->IsThreeStage = 1U;
			InstancePtr->ControlDir = !!(Ctrl->bRequestType &
							XUSBPSU_USB_DIR_IN);
		}

		Xil_AssertVoid(InstancePtr->Chapter9 != NULL);

		InstancePtr->Chapter9(InstancePtr->AppData,
						&InstancePtr->SetupData);
		break;

	case XUSBPSU_EP0_DATA_PHASE:
		XUsbPsu_Ep0DataDone(InstancePtr, Event);
		break;

	case XUSBPSU_EP0_STATUS_PHASE:
		XUsbPsu_Ep0StatusDone(InstancePtr);
		break;

	default:
		/* Default case is a required MISRA-C guideline. */
		break;
	}
}

/****************************************************************************/
/**
* Handles Transfer Not Ready event of Control Endpoints EP0 OUT and EP0 IN.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_Ep0XferNotReady(struct XUsbPsu *InstancePtr,
				 const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep *Ept;

	Ept = &InstancePtr->eps[Event->Epnumber];

	switch (Event->Status) {
	case DEPEVT_STATUS_CONTROL_DATA:
		/*
		 * We already have a DATA transfer in the controller's cache,
		 * if we receive a XferNotReady(DATA) we will ignore it, unless
		 * it's for the wrong direction.
		 *
		 * In that case, we must issue END_TRANSFER command to the Data
		 * Phase we already have started and issue SetStall on the
		 * control endpoint.
		 */
		if (Event->Epnumber != InstancePtr->ControlDir) {
			XUsbPsu_Ep0_EndControlData(InstancePtr, Ept);
			XUsbPsu_Ep0StallRestart(InstancePtr);
		}
		break;

	case DEPEVT_STATUS_CONTROL_STATUS:
		(void)XUsbPsu_Ep0StartStatus(InstancePtr, Event);
		break;

	default:
		/* Default case is a required MIRSA-C guideline. */
		break;
	}
}

/****************************************************************************/
/**
* Initiates DMA to send data on Control Endpoint EP0 IN to Host.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	BufferPtr is pointer to data.
* @param	BufferLen is Length of data buffer.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_Ep0Send(struct XUsbPsu *InstancePtr, u8 *BufferPtr, u32 BufferLen)
{
	/* Control IN - EP1 */
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Ep	*Ept;
	struct XUsbPsu_Trb	*TrbPtr;
	s32 Ret;

	Ept = &InstancePtr->eps[1U];
	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != 0U) {
		return (s32)XST_FAILURE;
	}

	Ept->RequestedBytes = BufferLen;
	Ept->BytesTxed = 0U;
	Ept->BufferPtr = BufferPtr;

	TrbPtr = &InstancePtr->Ep0_Trb;

	TrbPtr->BufferPtrLow  = (UINTPTR)BufferPtr;
	TrbPtr->BufferPtrHigh  = ((UINTPTR)BufferPtr >> 16U) >> 16U;
	TrbPtr->Size = BufferLen;
	TrbPtr->Ctrl = XUSBPSU_TRBCTL_CONTROL_DATA;

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_LST
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
		Xil_DCacheFlushRange((INTPTR)BufferPtr, BufferLen);
	}

	InstancePtr->Ep0State = XUSBPSU_EP0_DATA_PHASE;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, 0U, XUSBPSU_EP_DIR_IN,
					XUSBPSU_DEPCMD_STARTTRANSFER, Params);
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
* Initiates DMA to receive data on Control Endpoint EP0 OUT from Host.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	BufferPtr is pointer to data.
* @param	Length is Length of data to be received.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_Ep0Recv(struct XUsbPsu *InstancePtr, u8 *BufferPtr, u32 Length)
{
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Ep	*Ept;
	struct XUsbPsu_Trb	*TrbPtr;
	u32 Size;
	s32 Ret;

	Ept = &InstancePtr->eps[0U];
	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != 0U) {
		return (s32)XST_FAILURE;
	}

	Ept->RequestedBytes = Length;
	Size = Length;
	Ept->BytesTxed = 0U;
	Ept->BufferPtr = BufferPtr;

	/*
	 * 8.2.5 - An OUT transfer size (Total TRB buffer allocation)
	 * must be a multiple of MaxPacketSize even if software is expecting a
	 * fixed non-multiple of MaxPacketSize transfer from the Host.
	 */
	if (!IS_ALIGNED(Length, Ept->MaxSize)) {
		u16 TmpSize = Ept->MaxSize;
		Size = (u32)roundup(Length, (u32)TmpSize);
		Ept->UnalignedTx = 1U;
	}

	TrbPtr = &InstancePtr->Ep0_Trb;

	TrbPtr->BufferPtrLow = (UINTPTR)BufferPtr;
	TrbPtr->BufferPtrHigh = ((UINTPTR)BufferPtr >> 16U) >> 16U;
	TrbPtr->Size = Size;
	TrbPtr->Ctrl = XUSBPSU_TRBCTL_CONTROL_DATA;

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_LST
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
		Xil_DCacheInvalidateRange((INTPTR)BufferPtr, Length);
	}

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	InstancePtr->Ep0State = XUSBPSU_EP0_DATA_PHASE;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT,
				XUSBPSU_DEPCMD_STARTTRANSFER, Params);
	if (Ret != XST_SUCCESS) {
		return (s32)XST_FAILURE;
	}

	Ept->EpStatus |= XUSBPSU_EP_BUSY;
	Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
						Ept->UsbEpNum, Ept->Direction);

	return (s32)XST_SUCCESS;
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/*****************************************************************************/
/**
* Restarts EP0 endpoint
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
*		on.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_RestoreEp0(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_Ep *Ept;
	s32 Ret;
	u8 EpNum;

	for (EpNum = 0U; EpNum < 2U; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		Ret = XUsbPsu_EpEnable(InstancePtr, Ept->UsbEpNum,
			Ept->Direction, Ept->MaxSize, Ept->Type, (u8)TRUE);
		if (Ret == XST_FAILURE) {
			return (s32)XST_FAILURE;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) != (u32)0U) {
			XUsbPsu_Ep0StallRestart(InstancePtr);
		} else {
			Ret = XUsbPsu_RestartEp(InstancePtr, Ept->PhyEpNum);
			if (Ret == XST_FAILURE) {
				return (s32)XST_FAILURE;
			}
		}
	}

	return (s32)XST_SUCCESS;
}

#endif /* #ifdef XUSBPSU_HIBERNATION_ENABLE */
/** @} */
