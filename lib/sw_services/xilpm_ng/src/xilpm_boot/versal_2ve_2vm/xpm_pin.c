/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_pin.h"
#include "xpm_update.h"
static XPm_PinNode *PmMioPins[XPM_NODEIDX_STMIC_MAX] XPM_INIT_DATA(PmMioPins) = { NULL };
static u32 PmNumPins XPM_INIT_DATA(PmNumPins) = 0U;

/****************************************************************************/
/**
 * @brief  This function initializes the XPm_PinNode data staructure.
 *
 * @param Pin		XPm_PinNode data staructure.
 * @param PinId		PinNode ID.
 * @param BaseAddress	BaseAddress of the pin.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 ****************************************************************************/
XStatus XPmPin_Init(XPm_PinNode *Pin, u32 PinId, u32 BaseAddress)
{
	XStatus Status = XST_FAILURE;
	u32 PinIdx;

	PinIdx = NODEINDEX(PinId);
	if ((PinIdx >= (u32)XPM_NODEIDX_STMIC_MAX) ||
	    (PinIdx == (u32)XPM_NODEIDX_STMIC_MIN)) {
		goto done;
	}

	XPmNode_Init(&Pin->Node, PinId, (u8)XPM_PINSTATE_UNUSED, BaseAddress);
	PmMioPins[PinIdx] = Pin;
	PmNumPins++;
	Status = XST_SUCCESS;

done:
	return Status;
}
/****************************************************************************/
/**
 * @brief  This function returns instance to XPm_PinNode based on PinId.
 *
 * @param PinId		PinNode ID.
 *
 * @return Instance of XPm_PinNode if successful else NULL.
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetById(u32 PinId)
{
	XPm_PinNode *PinNode = NULL;
	u32 PinIndex = NODEINDEX(PinId);
	if ((u32)XPM_NODECLASS_STMIC != NODECLASS(PinId)) {
		goto done;
	} else if ((u32)XPM_NODESUBCL_PIN != NODESUBCLASS(PinId)) {
		goto done;
	} else if (((u32)XPM_NODETYPE_LPD_MIO != NODETYPE(PinId)) &&
		   ((u32)XPM_NODETYPE_PMC_MIO != NODETYPE(PinId))) {
		goto done;
	} else if (PinIndex >= (u32)XPM_NODEIDX_STMIC_MAX) {
		goto done;
	} else {
		/* Required by MISRA */
	}

	PinNode =  PmMioPins[PinIndex];

done:
	return PinNode;
}

/****************************************************************************/
/**
 * @brief  Get requested pin node by node index
 *
 * @param PinIndex     Pin Index.
 *
 * @return Pointer to requested XPm_PinNode, NULL otherwise
 *
 * @note Requires only node index
 *
 ****************************************************************************/
XPm_PinNode *XPmPin_GetByIndex(const u32 PinIndex)
{
	XPm_PinNode *Pin = NULL;

	Pin = PmMioPins[PinIndex];

	return Pin;
}