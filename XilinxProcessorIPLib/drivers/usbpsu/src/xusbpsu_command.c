/******************************************************************************
* Copyright (C) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2023 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*****************************************************************************/

/****************************************************************************/
/**
*
* @file xusbpsu_command.c
* @addtogroup usbpsu_api USBPSU APIs
* @{
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   pm  03/03/20 First release
* 1.8	pm  24/07/20 Fixed MISRA-C and Coverity warnings
* 1.10	pm  24/07/21 Fixed MISRA-C and Coverity warnings
* 1.12	pm  10/08/22 Update doxygen tag and addtogroup version
* 1.15  np  26/03/24 Add doxygen and editorial fixes
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
* Returns zeroed parameters to be used by Endpoint commands.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
*
* @return	Zeroed Params structure pointer.
*
*
*****************************************************************************/
struct XUsbPsu_EpParams *XUsbPsu_GetEpParams(struct XUsbPsu *InstancePtr)
{
	if (InstancePtr == NULL) {
		return NULL;
	}

	InstancePtr->EpParams.Param0 = 0x00U;
	InstancePtr->EpParams.Param1 = 0x00U;
	InstancePtr->EpParams.Param2 = 0x00U;

	return &InstancePtr->EpParams;
}

/****************************************************************************/
/**
* @brief
* Enables endpoint for sending/receiving data.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	UsbEpNum USB endpoint number.
* @param	Dir Direction of endpoint
* 				- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
* @param	Maxsize Size of Endpoint size.
* @param	Type Endpoint type Control/Bulk/Interrupt/Isoc.
* @param	Restore Typically False, True if saved state has to be restored.
*
*
* @return	XST_SUCCESS else XST_FAILURE.
*
*
****************************************************************************/
s32 XUsbPsu_EpEnable(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir,
		     u16 Maxsize, u8 Type, u8 Restore)
{
	struct XUsbPsu_Ep *Ept;
	struct XUsbPsu_Trb *TrbStHw, *TrbLink;
	void *RetPtr;
	u32 RegVal;
	u32 PhyEpNum;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEpNum <= (u8)16U);
	Xil_AssertNonvoid((Dir == XUSBPSU_EP_DIR_IN) ||
			  (Dir == XUSBPSU_EP_DIR_OUT));
	Xil_AssertNonvoid((Maxsize >= 64U) && (Maxsize <= 1024U));

	PhyEpNum = (u32)XUSBPSU_PhysicalEp((u32)UsbEpNum, (u32)Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	Ept->UsbEpNum	= UsbEpNum;
	Ept->Direction	= Dir;
	Ept->Type	= Type;
	Ept->MaxSize	= Maxsize;
	Ept->PhyEpNum	= (u8)PhyEpNum;
	Ept->CurUf	= 0U;
	if (InstancePtr->IsHibernated == (u8)FALSE) {
		Ept->TrbEnqueue	= 0U;
		Ept->TrbDequeue	= 0U;
	}

	if (((Ept->EpStatus & XUSBPSU_EP_ENABLED) == 0U)
	    || (InstancePtr->IsHibernated == (u8)TRUE)) {
		if (XUsbPsu_StartEpConfig(InstancePtr, UsbEpNum,
					  Dir)	== XST_FAILURE) {
			return (s32)XST_FAILURE;
		}
	}

	if (XUsbPsu_SetEpConfig(InstancePtr, UsbEpNum, Dir, Maxsize,
				Type, Restore) == XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	if (((Ept->EpStatus & XUSBPSU_EP_ENABLED) == 0U)
	    || (InstancePtr->IsHibernated == (u8)TRUE)) {
		if (XUsbPsu_SetXferResource(InstancePtr, UsbEpNum,
					    Dir)	== XST_FAILURE) {
			return (s32)XST_FAILURE;
		}

		Ept->EpStatus |= XUSBPSU_EP_ENABLED;

		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DALEPENA);
		RegVal |= (u32)(XUSBPSU_DALEPENA_EP(Ept->PhyEpNum));
		XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DALEPENA, RegVal);

		/* Following code is only applicable for ISO XFER */
		TrbStHw = &Ept->EpTrb[0U];

		/* Link TRB. The HWO bit is never reset */
		TrbLink = &Ept->EpTrb[NO_OF_TRB_PER_EP];

		RetPtr = memset(TrbLink, 0x0, sizeof(struct XUsbPsu_Trb));
		if (RetPtr == NULL) {
			return (s32)XST_FAILURE;
		}

		TrbLink->BufferPtrLow = (UINTPTR)TrbStHw;
		TrbLink->BufferPtrHigh = ((UINTPTR)TrbStHw >> 16U) >> 16U;
		TrbLink->Ctrl |= XUSBPSU_TRBCTL_LINK_TRB;
		TrbLink->Ctrl |= XUSBPSU_TRB_CTRL_HWO;

		/* flush the link trb */
		if (InstancePtr->ConfigPtr->IsCacheCoherent == (u8)0U) {
			Xil_DCacheFlushRange((INTPTR)TrbLink,
					     sizeof(struct XUsbPsu_Trb));
		}
	}

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief
* Disables Endpoint.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	UsbEpNum USB endpoint number.
* @param	Dir Direction of endpoint
*			- XUSBPSU_EP_DIR_IN/XUSBPSU_EP_DIR_OUT.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
*
****************************************************************************/
s32 XUsbPsu_EpDisable(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir)
{
	u32	RegVal;
	u8	PhyEpNum;
	struct XUsbPsu_Ep *Ept;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEpNum <= (u8)16U);
	Xil_AssertNonvoid((Dir == XUSBPSU_EP_DIR_IN) ||
			  (Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = (u8)XUSBPSU_PhysicalEp(UsbEpNum, Dir);
	Ept = &InstancePtr->eps[PhyEpNum];

	/* make sure HW endpoint isn't stalled */
	if ((Ept->EpStatus & XUSBPSU_EP_STALL) != (u32)0U) {
		XUsbPsu_EpClearStall(InstancePtr, Ept->UsbEpNum,
				     Ept->Direction);
	}

	RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DALEPENA);
	RegVal &= ~((u32)XUSBPSU_DALEPENA_EP(PhyEpNum));
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DALEPENA, RegVal);

	Ept->Type = 0U;
	Ept->EpStatus = 0U;
	Ept->MaxSize = 0U;
	Ept->TrbEnqueue	= 0U;
	Ept->TrbDequeue	= 0U;

	return (s32)XST_SUCCESS;
}

/****************************************************************************/
/**
* Sends Endpoint command to the Endpoint.
*
* @param	InstancePtr Pointer to the XUsbPsu instance.
* @param	UsbEpNum USB endpoint number.
* @param	Dir Direction of endpoint
*			- XUSBPSU_EP_DIR_IN/ XUSBPSU_EP_DIR_OUT.
* @param	Cmd Endpoint command.
* @param	Params Endpoint command parameters.
*
* @return	XST_SUCCESS else XST_FAILURE.
*
*
*****************************************************************************/
s32 XUsbPsu_SendEpCmd(struct XUsbPsu *InstancePtr, u8 UsbEpNum, u8 Dir,
		      u32 Cmd, struct XUsbPsu_EpParams *Params)
{
	u32	PhyEpNum;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(UsbEpNum <= (u8)16U);
	Xil_AssertNonvoid((Dir == XUSBPSU_EP_DIR_IN) ||
			  (Dir == XUSBPSU_EP_DIR_OUT));

	PhyEpNum = (u32)XUSBPSU_PhysicalEp((u32)UsbEpNum, (u32)Dir);

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEPCMDPAR0(PhyEpNum),
			 Params->Param0);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEPCMDPAR1(PhyEpNum),
			 Params->Param1);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEPCMDPAR2(PhyEpNum),
			 Params->Param2);

	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DEPCMD(PhyEpNum),
			 Cmd | XUSBPSU_DEPCMD_CMDACT);

	if (XUsbPsu_WaitClearTimeout(InstancePtr, XUSBPSU_DEPCMD(PhyEpNum),
				     XUSBPSU_DEPCMD_CMDACT, 500U) == (s32)XST_FAILURE) {
		return (s32)XST_FAILURE;
	}

	return (s32)XST_SUCCESS;
}

#ifdef XUSBPSU_HIBERNATION_ENABLE

/*****************************************************************************/
/**
* Sends generic command for the gadget.
*
* @param	InstancePtr Pointer to the XUsbPsu instance to be worked
* 		on.
* @param	cmd Command to be sent.
* @param	param Parameter for the command, to be written in DGCMDPAR
* 		register.
*
* @return
*		- XST_SUCCESS on success
*		- XST_FAILURE on timeout
*		- XST_REGISTER_ERROR on status error
*
*
******************************************************************************/
s32 XUsbPsu_SendGadgetGenericCmd(struct XUsbPsu *InstancePtr, u32 cmd,
				 u32 param)
{
	u32		RegVal, retry = 500U;
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DGCMDPAR, param);
	XUsbPsu_WriteReg(InstancePtr, XUSBPSU_DGCMD,
			 cmd | XUSBPSU_DGCMD_CMDACT);

	while (retry > 0U) {
		RegVal = XUsbPsu_ReadReg(InstancePtr, XUSBPSU_DGCMD);
		if ((RegVal & XUSBPSU_DGCMD_CMDACT) == (u32)0U) {
			if (XUSBPSU_DGCMD_STATUS(RegVal) != (u32)0U) {
				return (s32)XST_REGISTER_ERROR;
			}
			return (s32)XST_SUCCESS;
		}

		retry = retry - 1U;
	}

	return (s32)XST_FAILURE;
}

#endif /* XUSBPSU_HIBERNATION_ENABLE */
/** @} */
