/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pin.h"
static XPm_PinNode *PmMioPins[XPM_NODEIDX_STMIC_MAX];
static u16 PmNumPins;

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
