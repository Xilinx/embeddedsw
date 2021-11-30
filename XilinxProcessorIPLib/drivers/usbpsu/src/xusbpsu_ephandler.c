/******************************************************************************
* Copyright (C) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_ephandler.c
* @addtogroup Overview
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pm  03/23/20 First release
* 1.8	pm  24/07/20 Fixed MISRA-C and Coverity warnings
*
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
* @brief
* Stops transfer on Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEpNum is USB endpoint number.
* @param	Dir is direction of endpoint
* 				- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
* @param	Force flag to stop/pause transfer.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XUsbPsu_StopTransfer(struct XUsbPsu *InstancePtr, u8 UsbEpNum,
			u8 Dir, u8 Force)
{
	struct XUsbPsu_Ep *Ept;
	struct XUsbPsu_EpParams *Params;
	u8 PhyEpNum;
	u32 Cmd;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(UsbEpNum <= (u8)16U);
	Xil_AssertVoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = XUSBPSU_PhysicalEp(UsbEpNum, Dir);
	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	Ept = &InstancePtr->eps[PhyEpNum];

	if (Ept->ResourceIndex == 0U) {
		return;
	}

	/*
	 * - Issue EndTransfer WITH CMDIOC bit set
	 * - Wait 100us
	 */
	Cmd = XUSBPSU_DEPCMD_ENDTRANSFER;
	Cmd |= (Force == (u8)TRUE) ? XUSBPSU_DEPCMD_HIPRI_FORCERM : 0U;
	Cmd |= XUSBPSU_DEPCMD_CMDIOC;
	Cmd |= XUSBPSU_DEPCMD_PARAM(Ept->ResourceIndex);
	(void)XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
							Cmd, Params);
	if (Force == (u8)TRUE) {
		Ept->ResourceIndex = 0U;
	}

	Ept->EpStatus &= ~XUSBPSU_EP_BUSY;
	XUsbPsu_Sleep(100U);
}

/****************************************************************************/
/**
* Reset and Deactivate transfer Endpoint.
*
* @param        InstancePtr is a pointer to the XUsbPsu instance.
* @param        UsbEpNum is USB endpoint number.
* @param        Dir is direction of endpoint
*    		- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
*
* @return       None.
*
* @note         None.
*
****************************************************************************/
void XUsbPsu_EpTransferDeactive(struct XUsbPsu *InstancePtr, u8 UsbEpNum,
								u8 Dir)
{
	struct XUsbPsu_Ep *Ept;
    u8 PhyEpNum;
    u32 RegVal;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(UsbEpNum <= (u8)16U);
	Xil_AssertVoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));


	PhyEpNum = XUSBPSU_PhysicalEp(UsbEpNum, Dir);

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DALEPENA);
	RegVal &= ~((u32)XUSBPSU_DALEPENA_EP(PhyEpNum));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DALEPENA, RegVal);

	Ept = &InstancePtr->eps[PhyEpNum];

	if (Ept != NULL) {
		Ept->Type = 0U;
		Ept->EpStatus = 0U;
		Ept->MaxSize = 0U;
		Ept->TrbEnqueue = 0U;
		Ept->TrbDequeue = 0U;
	}
}

/****************************************************************************/
/**
* Query endpoint state and save it in EpSavedState
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Ept is a pointer to the XUsbPsu pointer structure.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XUsbPsu_SaveEndpointState(struct XUsbPsu *InstancePtr,
						 struct XUsbPsu_Ep *Ept)
{
	Xil_AssertVoid(InstancePtr != NULL);

	struct XUsbPsu_EpParams *Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	if (XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
			XUSBPSU_DEPCMD_GETEPSTATE, Params) == XST_FAILURE) {
	}
	Ept->EpSavedState = XUsbPsu_ReadReg(InstancePtr,
				 XUSBPSU_DEPCMDPAR2(Ept->PhyEpNum));
}

/****************************************************************************/
/**
* Clears Stall on all endpoints.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
****************************************************************************/
void XUsbPsu_ClearStalls(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_EpParams *Params;
	u32 Epnum;
	struct XUsbPsu_Ep *Ept;

	Xil_AssertVoid(InstancePtr != NULL);

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	for (Epnum = 1U; Epnum < XUSBPSU_ENDPOINTS_NUM; Epnum++) {

		Ept = &InstancePtr->eps[Epnum];

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) == 0U) {
			continue;
		}

		Ept->EpStatus &= ~XUSBPSU_EP_STALL;

		(void)XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum,
						Ept->Direction,
						XUSBPSU_DEPCMD_CLEARSTALL,
						Params);
	}
}

/****************************************************************************/
/**
* @brief
* Initiates DMA to send data on endpoint to Host.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEp is USB endpoint number.
* @param	BufferPtr is pointer to data. This data buffer is cache-aligned.
* @param	BufferLen is length of data buffer.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		This function is expected to initiates DMA to send data on
*		endpoint towards Host. This data buffer should be aligned.
*
*****************************************************************************/
s32 XUsbPsu_EpBufferSend(struct XUsbPsu *InstancePtr, u8 UsbEp,
						 u8 *BufferPtr, u32 BufferLen)
{
	u8	PhyEpNum;
	u32	cmd;
	s32	RetVal;
	struct XUsbPsu_Trb	*TrbPtr;
	struct XUsbPsu_Ep *Ept;
	struct XUsbPsu_EpParams *Params;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEp <= (u8)16U);
	Xil_AssertNonvoid(BufferPtr != NULL);

	PhyEpNum = XUSBPSU_PhysicalEp(UsbEp, XUSBPSU_EP_DIR_IN);
	if (PhyEpNum == 1U) {
		RetVal = XUsbPsu_Ep0Send(InstancePtr, BufferPtr, BufferLen);
		return RetVal;
	}

	Ept = &InstancePtr->eps[PhyEpNum];

	if (Ept->Direction != XUSBPSU_EP_DIR_IN) {
		return (s32)XST_FAILURE;
	}

	Ept->RequestedBytes = BufferLen;
	Ept->BytesTxed = 0U;
	Ept->BufferPtr = BufferPtr;

	TrbPtr = &Ept->EpTrb[Ept->TrbEnqueue];

	Ept->TrbEnqueue++;
	if (Ept->TrbEnqueue == NO_OF_TRB_PER_EP) {
		Ept->TrbEnqueue = 0U;
	}

	TrbPtr->BufferPtrLow  = (UINTPTR)BufferPtr;
	TrbPtr->BufferPtrHigh  = ((UINTPTR)BufferPtr >> 16U) >> 16U;
	TrbPtr->Size = BufferLen & XUSBPSU_TRB_SIZE_MASK;

	switch (Ept->Type) {
	case XUSBPSU_ENDPOINT_XFER_ISOC:
		/*
		 *  According to DWC3 datasheet, XUSBPSU_TRBCTL_ISOCHRONOUS and
		 *  XUSBPSU_TRBCTL_CHN fields are only set when request has
		 *  scattered list so these fields are not set over here.
		 */
		TrbPtr->Ctrl = (XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST
				| XUSBPSU_TRB_CTRL_CSP);

		break;
	case XUSBPSU_ENDPOINT_XFER_INT:
	case XUSBPSU_ENDPOINT_XFER_BULK:
		TrbPtr->Ctrl = (XUSBPSU_TRBCTL_NORMAL
				| XUSBPSU_TRB_CTRL_LST);

		break;
	default:
		/* Do Nothing. Added for making MISRA-C complaint */
		break;
	}

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
		Xil_DCacheFlushRange((INTPTR)BufferPtr, BufferLen);
	}

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);
	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != (u32)0U) {
		cmd = XUSBPSU_DEPCMD_UPDATETRANSFER;
		cmd |= XUSBPSU_DEPCMD_PARAM(Ept->ResourceIndex);
	} else {
		if (Ept->Type == XUSBPSU_ENDPOINT_XFER_ISOC) {
			BufferPtr += BufferLen;
			struct XUsbPsu_Trb	*TrbTempNext;
			TrbTempNext = &Ept->EpTrb[Ept->TrbEnqueue];

			Ept->TrbEnqueue++;
			if (Ept->TrbEnqueue == NO_OF_TRB_PER_EP) {
				Ept->TrbEnqueue = 0U;
			}

			TrbTempNext->BufferPtrLow  = (UINTPTR)BufferPtr;
			TrbTempNext->BufferPtrHigh  = ((UINTPTR)BufferPtr >>
								 16U) >> 16U;
			TrbTempNext->Size = BufferLen & XUSBPSU_TRB_SIZE_MASK;

			TrbTempNext->Ctrl = (XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST
					| XUSBPSU_TRB_CTRL_CSP
					| XUSBPSU_TRB_CTRL_HWO
					| XUSBPSU_TRB_CTRL_IOC
					| XUSBPSU_TRB_CTRL_ISP_IMI);

			if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
				Xil_DCacheFlushRange((INTPTR)TrbTempNext,
						sizeof(struct XUsbPsu_Trb));
				Xil_DCacheFlushRange((INTPTR)BufferPtr,
								 BufferLen);
			}

		}

		cmd = XUSBPSU_DEPCMD_STARTTRANSFER;
		cmd |= XUSBPSU_DEPCMD_PARAM(Ept->CurUf);
	}

	RetVal = XUsbPsu_SendEpCmd(InstancePtr, UsbEp, Ept->Direction,
								cmd, Params);
	if (RetVal != (s32)XST_SUCCESS) {
		return (s32)XST_FAILURE;
	}

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) == (u32)0U) {
		Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
				Ept->UsbEpNum,
				Ept->Direction);

		Ept->EpStatus |= XUSBPSU_EP_BUSY;
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Initiates DMA to receive data on Endpoint from Host.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEp is USB endpoint number.
* @param	BufferPtr is pointer to data. This data buffer is cache-aligned.
* @param	Length is length of data to be received.
*
* @return	XST_SUCCESS else XST_FAILURE
*
* @note		This function is expected to initiates DMA to receive data on
*		the endpoint from the Host. This data buffer should be aligned.
*
*****************************************************************************/
s32 XUsbPsu_EpBufferRecv(struct XUsbPsu *InstancePtr, u8 UsbEp,
						 u8 *BufferPtr, u32 Length)
{
	u8	PhyEpNum;
	u32	cmd;
	u32	Size;
	s32	RetVal;
	struct XUsbPsu_Trb	*TrbPtr;
	struct XUsbPsu_Ep *Ept;
	struct XUsbPsu_EpParams *Params;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEp <= (u8)16U);
	Xil_AssertNonvoid(BufferPtr != NULL);

	PhyEpNum = XUSBPSU_PhysicalEp(UsbEp, XUSBPSU_EP_DIR_OUT);
	if (PhyEpNum == 0U) {
		RetVal = XUsbPsu_Ep0Recv(InstancePtr, BufferPtr, Length);
		return RetVal;
	}

	Ept = &InstancePtr->eps[PhyEpNum];

	if (Ept->Direction != XUSBPSU_EP_DIR_OUT) {
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
		Size = (u32)roundup(Length, (u32)Ept->MaxSize);
		Ept->UnalignedTx = 1U;
	}

	TrbPtr = &Ept->EpTrb[Ept->TrbEnqueue];

	Ept->TrbEnqueue += 1U;
	if (Ept->TrbEnqueue == NO_OF_TRB_PER_EP) {
		Ept->TrbEnqueue = 0U;
	}

	TrbPtr->BufferPtrLow  = (UINTPTR)BufferPtr;
	TrbPtr->BufferPtrHigh = ((UINTPTR)BufferPtr >> 16U) >> 16U;
	TrbPtr->Size = Size;

	switch (Ept->Type) {
	case XUSBPSU_ENDPOINT_XFER_ISOC:
		/*
		 *  According to Linux driver, XUSBPSU_TRBCTL_ISOCHRONOUS and
		 *  XUSBPSU_TRBCTL_CHN fields are only set when request has
		 *  scattered list so these fields are not set over here.
		 */
		TrbPtr->Ctrl = (XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST
				| XUSBPSU_TRB_CTRL_CSP);

		break;
	case XUSBPSU_ENDPOINT_XFER_INT:
	case XUSBPSU_ENDPOINT_XFER_BULK:
		TrbPtr->Ctrl = (XUSBPSU_TRBCTL_NORMAL
				| XUSBPSU_TRB_CTRL_LST);

		break;
	default:
		/* Do Nothing. Added for making MISRA-C complaint */
		break;
	}

	TrbPtr->Ctrl |= (XUSBPSU_TRB_CTRL_HWO
			| XUSBPSU_TRB_CTRL_IOC
			| XUSBPSU_TRB_CTRL_ISP_IMI);


	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
					 sizeof(struct XUsbPsu_Trb));
		Xil_DCacheInvalidateRange((INTPTR)BufferPtr, Length);
	}

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);
	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) != (u32)0U) {
		cmd = XUSBPSU_DEPCMD_UPDATETRANSFER;
		cmd |= XUSBPSU_DEPCMD_PARAM(Ept->ResourceIndex);
	} else {
		if (Ept->Type == XUSBPSU_ENDPOINT_XFER_ISOC) {
			BufferPtr += Length;
			struct XUsbPsu_Trb	*TrbTempNext;
			TrbTempNext = &Ept->EpTrb[Ept->TrbEnqueue];

			Ept->TrbEnqueue++;
			if (Ept->TrbEnqueue == NO_OF_TRB_PER_EP) {
				Ept->TrbEnqueue = 0U;
			}

			TrbTempNext->BufferPtrLow  = (UINTPTR)BufferPtr;
			TrbTempNext->BufferPtrHigh  = ((UINTPTR)BufferPtr >>
								 16U) >> 16U;
			TrbTempNext->Size = Length & XUSBPSU_TRB_SIZE_MASK;

			TrbTempNext->Ctrl = (XUSBPSU_TRBCTL_ISOCHRONOUS_FIRST
					| XUSBPSU_TRB_CTRL_CSP
					| XUSBPSU_TRB_CTRL_HWO
					| XUSBPSU_TRB_CTRL_IOC
					| XUSBPSU_TRB_CTRL_ISP_IMI);

			if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
				Xil_DCacheFlushRange((INTPTR)TrbTempNext,
						sizeof(struct XUsbPsu_Trb));
				Xil_DCacheFlushRange((INTPTR)BufferPtr, Length);
			}

		}

		cmd = XUSBPSU_DEPCMD_STARTTRANSFER;
		cmd |= XUSBPSU_DEPCMD_PARAM(Ept->CurUf);
	}

	RetVal = XUsbPsu_SendEpCmd(InstancePtr, UsbEp, Ept->Direction,
								cmd, Params);
	if (RetVal != XST_SUCCESS) {
		return (s32)XST_FAILURE;
	}

	if ((Ept->EpStatus & XUSBPSU_EP_BUSY) == (u32)0U) {
		Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
				Ept->UsbEpNum,
				Ept->Direction);

		Ept->EpStatus |= XUSBPSU_EP_BUSY;
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Stalls an Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Epnum is USB endpoint number.
* @param	Dir	is direction.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_EpSetStall(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir)
{
	u8	PhyEpNum;
	struct XUsbPsu_Ep *Ept = NULL;
	struct XUsbPsu_EpParams *Params;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Epnum <= (u8)16U);
	Xil_AssertVoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = XUSBPSU_PhysicalEp(Epnum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	(void)XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
					XUSBPSU_DEPCMD_SETSTALL, Params);

	Ept->EpStatus |= XUSBPSU_EP_STALL;
}

/****************************************************************************/
/**
* @brief
* Clears Stall on an Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Epnum is USB endpoint number.
* @param	Dir	is direction.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_EpClearStall(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir)
{
	u8	PhyEpNum;
	struct XUsbPsu_Ep *Ept = NULL;
	struct XUsbPsu_EpParams *Params;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Epnum <= (u8)16U);
	Xil_AssertVoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = XUSBPSU_PhysicalEp(Epnum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertVoid(Params != NULL);

	(void)XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
					XUSBPSU_DEPCMD_CLEARSTALL, Params);

	Ept->EpStatus &= ~XUSBPSU_EP_STALL;
}

/****************************************************************************/
/**
* @brief
* Sets an user handler to be called after data is sent/received by an Endpoint
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Epnum is USB endpoint number.
* @param	Dir is direction of endpoint
* 				- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
* @param	Handler is user handler to be called.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_SetEpHandler(struct XUsbPsu *InstancePtr, u8 Epnum,
			u8 Dir, void (*Handler)(void *, u32, u32))
{
	u8 PhyEpNum;
	struct XUsbPsu_Ep *Ept;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Epnum <= (u8)16U);
	Xil_AssertVoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = XUSBPSU_PhysicalEp(Epnum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];
	Ept->Handler = (void (*)(void *, u32, u32))Handler;
}

/****************************************************************************/
/**
* @brief
* Returns status of endpoint - Stalled or not
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Epnum is USB endpoint number.
* @param	Dir is direction of endpoint
* 				- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
*
* @return
*			1 - if stalled
*			0 - if not stalled
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_IsEpStalled(struct XUsbPsu *InstancePtr, u8 Epnum, u8 Dir)
{
	u8 PhyEpNum;
	struct XUsbPsu_Ep *Ept;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(Epnum <= (u8)16U);
	Xil_AssertNonvoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = XUSBPSU_PhysicalEp(Epnum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	return (s32)(!!(Ept->EpStatus & XUSBPSU_EP_STALL));
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
void XUsbPsu_EpXferComplete(struct XUsbPsu *InstancePtr,
					const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep	*Ept;
	struct XUsbPsu_Trb	*TrbPtr;
	u32	Length;
	u32	Epnum;
	u8	Dir;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Event != NULL);

	Epnum = Event->Epnumber;
	Ept = &InstancePtr->eps[Epnum];
	Dir = Ept->Direction;
	TrbPtr = &Ept->EpTrb[Ept->TrbDequeue];

	Ept->TrbDequeue++;
	if (Ept->TrbDequeue == NO_OF_TRB_PER_EP) {
		Ept->TrbDequeue = 0U;
	}

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheInvalidateRange((INTPTR)TrbPtr,
						 sizeof(struct XUsbPsu_Trb));
	}

	if (Event->Endpoint_Event == XUSBPSU_DEPEVT_XFERCOMPLETE) {
		Ept->EpStatus &= ~(XUSBPSU_EP_BUSY);
		Ept->ResourceIndex = 0U;
	}

	Length = TrbPtr->Size & XUSBPSU_TRB_SIZE_MASK;

	if (Length == 0U) {
		Ept->BytesTxed = Ept->RequestedBytes;
	} else {
		if (Dir == XUSBPSU_EP_DIR_IN) {
			Ept->BytesTxed = Ept->RequestedBytes - Length;
		} else {
			if (Ept->UnalignedTx == 1U) {
				Ept->BytesTxed = (u32)roundup(
							Ept->RequestedBytes,
							(u32)Ept->MaxSize);
				Ept->BytesTxed -= Length;
				Ept->UnalignedTx = 0U;
			} else {
				/*
				 * Get the actual number of bytes transmitted
				 * by host
				 */
				Ept->BytesTxed = Ept->RequestedBytes - Length;
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
* For Isochronous transfer, get the microframe time and calls respective
* Endpoint handler.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	Event is a pointer to the Endpoint event occurred in core.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_EpXferNotReady(struct XUsbPsu *InstancePtr,
					const struct XUsbPsu_Event_Epevt *Event)
{
	struct XUsbPsu_Ep	*Ept;
	u32	Epnum;
	u32	CurUf;
	u32	Mask;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(Event != NULL);

	Epnum = Event->Epnumber;
	Ept = &InstancePtr->eps[Epnum];

	if (Ept->Type == XUSBPSU_ENDPOINT_XFER_ISOC) {
		Mask = ~(u32)((u32)1U << (Ept->Interval - 1U));
		CurUf = Event->Parameters & Mask;
		Ept->CurUf = (u16)(CurUf + (Ept->Interval * 4U));
		if (Ept->Handler != NULL) {
			Ept->Handler(InstancePtr->AppData, 0U, 0U);
		}
	}
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/*****************************************************************************/
/**
* Restarts transfer for active endpoint
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
* 		on.
* @param	EpNum is an endpoint number.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_RestartEp(struct XUsbPsu *InstancePtr, u8 EpNum)
{
	struct XUsbPsu_EpParams *Params;
	struct XUsbPsu_Trb	*TrbPtr;
	struct XUsbPsu_Ep	*Ept;
	u32	Cmd;
	s32	Ret;

	Xil_AssertNonvoid(InstancePtr != NULL);

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	Ept = &InstancePtr->eps[EpNum];

	/* check if we need to restart transfer */
	if ((Ept->ResourceIndex == (u32)0U) && (Ept->PhyEpNum != (u32)0U)) {
		return (s32)XST_SUCCESS;
	}

	if (Ept->UsbEpNum != (u32)0U) {
		TrbPtr = &Ept->EpTrb[Ept->TrbDequeue];
	} else {
		TrbPtr = &InstancePtr->Ep0_Trb;
	}

	TrbPtr->Ctrl |= XUSBPSU_TRB_CTRL_HWO;

	if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
		Xil_DCacheFlushRange((INTPTR)TrbPtr,
						sizeof(struct XUsbPsu_Trb));
		Xil_DCacheInvalidateRange((INTPTR)Ept->BufferPtr,
						Ept->RequestedBytes);
	}

	Params->Param0 = 0U;
	Params->Param1 = (UINTPTR)TrbPtr;

	Cmd = XUSBPSU_DEPCMD_STARTTRANSFER;

	Ret = XUsbPsu_SendEpCmd(InstancePtr, Ept->UsbEpNum, Ept->Direction,
			Cmd, Params);
	if (Ret == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	Ept->EpStatus |= XUSBPSU_EP_BUSY;
	Ept->ResourceIndex = (u8)XUsbPsu_EpGetTransferIndex(InstancePtr,
			Ept->UsbEpNum, Ept->Direction);

	return (s32)XST_SUCCESS;
}

#endif /*#ifdef XUSBPSU_HIBERNATION_ENABLE*/
/** @} */
