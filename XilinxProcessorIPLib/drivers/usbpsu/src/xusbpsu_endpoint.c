/******************************************************************************
* Copyright (C) 2016 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_endpoint.c
* @addtogroup usbpsu_v1_10
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16 First release
* 1.3   vak 04/03/17 Added CCI support for USB
* 1.4	bk  12/01/18 Modify USBPSU driver code to fit USB common example code
*		       for all USB IPs
*	myk 12/01/18 Added hibernation support for device mode
* 1.4	vak 30/05/18 Removed xusb_wrapper files
* 1.6	pm  22/07/19 Removed coverity warnings
*	pm  28/08/19 Removed 80-character warnings
* 1.7 	pm  23/03/20 Restructured the code for more readability and modularity
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
* Returns Transfer Index assigned by Core for an Endpoint transfer.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEpNum is USB endpoint number.
* @param	Dir is direction of endpoint
* 				- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT
*
* @return	Transfer Resource Index.
*
* @note		None.
*
*****************************************************************************/
u32 XUsbPsu_EpGetTransferIndex(struct XUsbPsu *InstancePtr, u8 UsbEpNum,
								u8 Dir)
{
	u8 PhyEpNum;
	u32 ResourceIndex;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEpNum <= (u8)16U);
	Xil_AssertNonvoid((Dir == XUSBPSU_EP_DIR_IN) ||
						(Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = (u8)XUSBPSU_PhysicalEp(UsbEpNum, Dir);
	ResourceIndex = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DEPCMD(PhyEpNum));

	return (u32)XUSBPSU_DEPCMD_GET_RSC_IDX(ResourceIndex);
}

/****************************************************************************/
/**
* Sends Start New Configuration command to Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEpNum is USB endpoint number.
* @param	Dir is direction of endpoint
*			- XUSBPSU_EP_DIR_IN/ XUSBPSU_EP_DIR_OUT.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note
* 		As per data book this command should be issued by software
*		under these conditions:
*		1. After power-on-reset with XferRscIdx=0 before starting
*		   to configure Physical Endpoints 0 and 1.
*		2. With XferRscIdx=2 before starting to configure
*		   Physical Endpoints > 1
*		3. This command should always be issued to
*		   Endpoint 0 (DEPCMD0).
*
*****************************************************************************/
s32 XUsbPsu_StartEpConfig(struct XUsbPsu *InstancePtr, u32 UsbEpNum, u8 Dir)
{
	struct XUsbPsu_EpParams *Params;
	u32	Cmd;
	u8 PhyEpNum;

	PhyEpNum = (u8)XUSBPSU_PhysicalEp(UsbEpNum, (u32)Dir);
	Params =  XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	if (PhyEpNum != 1U) {
		Cmd = XUSBPSU_DEPCMD_DEPSTARTCFG;
		/* XferRscIdx == 0 for EP0 and 2 for the remaining */
		if (PhyEpNum > 1U) {
			if (InstancePtr->IsConfigDone != 0U) {
				return (s32)XST_SUCCESS;
			}
			InstancePtr->IsConfigDone = 1U;
			Cmd |= XUSBPSU_DEPCMD_PARAM(2U);
		}

		return XUsbPsu_SendEpCmd(InstancePtr, 0U, XUSBPSU_EP_DIR_OUT,
								 Cmd, Params);
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* Sends Set Endpoint Configuration command to Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEpNum is USB endpoint number.
* @param	Dir is direction of endpoint
* 				-XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
* @param	Size is size of Endpoint size.
* @param	Type is Endpoint type Control/Bulk/Interrupt/Isoc.
* @param	Restore should be true if saved state should be restored;
*			typically this would be false
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_SetEpConfig(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir,
						u16 Size, u8 Type, u8 Restore)
{
	struct XUsbPsu_Ep *Ept;
	struct XUsbPsu_EpParams *Params;
	u8 PhyEpNum;

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	PhyEpNum = XUSBPSU_PhysicalEp(UsbEpNum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	Params->Param0 = XUSBPSU_DEPCFG_EP_TYPE(Type)
		| XUSBPSU_DEPCFG_MAX_PACKET_SIZE(Size);

	/*
	 * Set burst size to 1 as recommended
	 */
	if (InstancePtr->AppData->Speed == XUSBPSU_SPEED_SUPER) {
		Params->Param0 |= XUSBPSU_DEPCFG_BURST_SIZE(1U);
	}

	Params->Param1 = XUSBPSU_DEPCFG_XFER_COMPLETE_EN
		| XUSBPSU_DEPCFG_XFER_NOT_READY_EN;

	if (Restore == (u8)TRUE) {
		Params->Param0 |= XUSBPSU_DEPCFG_ACTION_RESTORE;
		Params->Param2 = Ept->EpSavedState;
	}

	/*
	 * We are doing 1:1 mapping for endpoints, meaning
	 * Physical Endpoints 2 maps to Logical Endpoint 2 and
	 * so on. We consider the direction bit as part of the physical
	 * endpoint number. So USB endpoint 0x81 is 0x03.
	 */
	Params->Param1 |= XUSBPSU_DEPCFG_EP_NUMBER(PhyEpNum);

	if (Dir != XUSBPSU_EP_DIR_OUT) {
		Params->Param0 |= XUSBPSU_DEPCFG_FIFO_NUMBER((u32)PhyEpNum >>
									 1U);
	}

	if (Ept->Type == XUSBPSU_ENDPOINT_XFER_ISOC) {
		Params->Param1 |= XUSBPSU_DEPCFG_BINTERVAL_M1(Ept->Interval -
									 1U);
		Params->Param1 |= XUSBPSU_DEPCFG_XFER_IN_PROGRESS_EN;
	}

	return XUsbPsu_SendEpCmd(InstancePtr, UsbEpNum, Dir,
						 XUSBPSU_DEPCMD_SETEPCONFIG,
						 Params);
}

/****************************************************************************/
/**
* Sends Set Transfer Resource command to Endpoint.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
* @param	UsbEpNum is USB endpoint number.
* @param	Dir is direction of endpoint - XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT
*
*
* @return	XST_SUCCESS else XST_FAILURE.
*
* @note		None.
*
*****************************************************************************/
s32 XUsbPsu_SetXferResource(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir)
{
	struct XUsbPsu_EpParams *Params;

	Params = XUsbPsu_GetEpParams(InstancePtr);
	Xil_AssertNonvoid(Params != NULL);

	Params->Param0 = XUSBPSU_DEPXFERCFG_NUM_XFER_RES(1U);

	return XUsbPsu_SendEpCmd(InstancePtr, UsbEpNum, Dir,
					 XUSBPSU_DEPCMD_SETTRANSFRESOURCE,
					 Params);
}

/****************************************************************************/
/**
* Stops any active transfer.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_StopActiveTransfers(struct XUsbPsu *InstancePtr)
{
	u32 Epnum;

	for (Epnum = 2U; Epnum < XUSBPSU_ENDPOINTS_NUM; Epnum++) {
		struct XUsbPsu_Ep *Ept;

		Ept = &InstancePtr->eps[Epnum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		XUsbPsu_StopTransfer(InstancePtr, Ept->UsbEpNum,
				Ept->Direction, (u8)TRUE);
	}
}

/****************************************************************************/
/**
* Clears stall on all stalled Eps.
*
* @param	InstancePtr is a pointer to the XUsbPsu instance.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_ClearStallAllEp(struct XUsbPsu *InstancePtr)
{
	u32 Epnum;

	for (Epnum = 1U; Epnum < XUSBPSU_ENDPOINTS_NUM; Epnum++) {
		struct XUsbPsu_Ep *Ept;

		Ept = &InstancePtr->eps[Epnum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) == (u32)0U) {
			continue;
		}

		XUsbPsu_EpClearStall(InstancePtr, Ept->UsbEpNum,
							Ept->Direction);
	}
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/*****************************************************************************/
/**
* Restarts non EP0 endpoints
*
* @param	InstancePtr is a pointer to the XUsbPsu instance to be worked
*		on.
*
* @return	XST_SUCCESS on success or else XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
s32 XUsbPsu_RestoreEps(struct XUsbPsu *InstancePtr)
{
	struct XUsbPsu_Ep *Ept;
	s32 Ret;
	u8 EpNum;

	for (EpNum = 2U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		Ret = XUsbPsu_EpEnable(InstancePtr, Ept->UsbEpNum,
					Ept->Direction, Ept->MaxSize,
					Ept->Type, (u8)TRUE);
		if (Ret == XST_FAILURE) {
			xil_printf("Failed to enable EP %d on wakeup: %d\r\n",
					EpNum, Ret);
			return (s32)XST_FAILURE;
		}
	}

	for (EpNum = 2U; EpNum < XUSBPSU_ENDPOINTS_NUM; EpNum++) {
		Ept = &InstancePtr->eps[EpNum];

		if ((Ept->EpStatus & XUSBPSU_EP_ENABLED) == (u32)0U) {
			continue;
		}

		if ((Ept->EpStatus & XUSBPSU_EP_STALL) != (u32)0U) {
			XUsbPsu_EpSetStall(InstancePtr, Ept->UsbEpNum,
								Ept->Direction);
		} else {
			Ret = XUsbPsu_RestartEp(InstancePtr, Ept->PhyEpNum);
			if (Ret == XST_FAILURE) {
			xil_printf("Failed to restart EP %d on wakeup: %d\r\n",
						EpNum, Ret);
				return (s32)XST_FAILURE;
			}
		}
	}

	return (s32)XST_SUCCESS;
}

#endif /* XUSBPSU_HIBERNATION_ENABLE */
/** @} */
