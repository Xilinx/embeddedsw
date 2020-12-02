/******************************************************************************
* Copyright (c) 2018-2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpsmfw_ipi_manager.c
*
* This file contains IPI manager functions for PSM Firmware
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who		Date		Changes
* ---- ---- -------- ------------------------------
* 1.00  ma   04/09/2018 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#include "xpsmfw_api.h"
#include "xpsmfw_ipi_manager.h"
#include "ipi.h"

#ifdef XPAR_XIPIPSU_0_DEVICE_ID
/* Instance of IPI Driver */
static XIpiPsu IpiInst;
static XIpiPsu *IpiInstPtr = &IpiInst;
u32 IpiMaskList[XPSMFW_IPI_MASK_COUNT] = {0U};

s32 XPsmfw_IpiManagerInit(void)
 {
	s32 Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;
	u32 i;

	/* Load Config for PSM IPI */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);

	if (IpiCfgPtr == NULL) {
		Status = XST_FAILURE;
		XPsmFw_Printf(DEBUG_ERROR, "IPI lookup config failed\r\n");
		goto Done;
	}
	/* Init Mask List */
	for (i = 0U; i < XPSMFW_IPI_MASK_COUNT; i++) {
		IpiMaskList[i] = IpiCfgPtr->TargetList[i].Mask;
	}
	/* Initialize the IPI driver */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);

	/* Enable IPIs from all Masters */
	for (i = 0U; i < XPSMFW_IPI_MASK_COUNT; i++) {
		XIpiPsu_InterruptEnable(IpiInstPtr, IpiCfgPtr->TargetList[i].Mask);
	}
	XPsmFw_Printf(DEBUG_DETAILED, "IPI interrupts are enabled\r\n");

Done:
	return Status;
}

/**
 * XPsmFw_DispatchIpiHandler() - Interrupt handler for IPI
 *
 * @IpiInstPtr		Pointer to the IPI instance
 */
int XPsmFw_DispatchIpiHandler(u32 SrcMask)
{
	int Status = XST_FAILURE;
	u32 MaskIndex;
	u32 Payload[XPSMFW_IPI_MAX_MSG_LEN];
	u32 Response[XPSMFW_IPI_MAX_MSG_LEN];

	XPsmFw_Printf(DEBUG_DETAILED, "In IPI handler\r\n");

	for (MaskIndex = 0U; MaskIndex < XPSMFW_IPI_MASK_COUNT; MaskIndex++) {
		if ((SrcMask & IpiMaskList[MaskIndex]) != 0U) {
			Status = XIpiPsu_ReadMessage(IpiInstPtr, IpiMaskList[MaskIndex],
			        &Payload[0], XPSMFW_IPI_MAX_MSG_LEN, XIPIPSU_BUF_TYPE_MSG);
			if (XST_SUCCESS != Status) {
				XPsmFw_Printf(DEBUG_ERROR, "Failure to read IPI msg\r\n");
			} else {
				Status = XPsmFw_ProcessIpi(&Payload[0]);

				Response[0] = (u32)Status;
				Status = XPsmFw_IpiSendResponse(IPI_PSM_IER_PMC_MASK,
								Response);
			}
		} else {
			Status = XST_SUCCESS;
		}
	}
	return Status;
}

/****************************************************************************/
/**
 * @brief	Sends IPI request to the target.
 *
 * @param IpiMask	IPI interrupt mask of target
 * @param Payload	API ID and call arguments to be written in
 *			IPI buffer
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *		or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_IpiSend(u32 IpiMask, u32 *Payload)
{
	XStatus Status = XST_FAILURE;

	Status = XIpiPsu_WriteMessage(IpiInstPtr, IpiMask, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: ERROR writing to IPI request buffer\n", __func__);
		goto done;
	}

	Status = XIpiPsu_TriggerIpi(IpiInstPtr, IpiMask);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief	Sends IPI response to the target.
 *
 * @param IpiMask	IPI interrupt mask of target
 * @param Payload	IPI response
 *
 * @return	XST_SUCCESS if successful else XST_FAILURE or an error code
 *		or a reason code
 *
 * @note	None
 *
 ****************************************************************************/
XStatus XPsmFw_IpiSendResponse(u32 IpiMask, u32 *Payload)
{
	XStatus Status = XST_FAILURE;

	Status = XIpiPsu_WriteMessage(IpiInstPtr, IpiMask, Payload,
				      PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		XPsmFw_Printf(DEBUG_ERROR, "%s: ERROR writing to IPI request buffer\n", __func__);
		goto done;
	}

done:
	return Status;
}
#else

s32 XPsmfw_IpiManagerInit(void)
{
	return XST_FAILURE;
}

int XPsmFw_DispatchIpiHandler(u32 SrcMask)
{
	(void)SrcMask;

	return XST_FAILURE;
}

XStatus XPsmFw_IpiSend(u32 IpiMask, u32 *Payload)
{
	(void)IpiMask;
	(void)Payload;

	return XST_FAILURE;
}

XStatus XPsmFw_IpiSendResponse(u32 IpiMask, u32 *Payload)
{
	(void)IpiMask;
	(void)Payload;

	return XST_FAILURE;
}
#endif
