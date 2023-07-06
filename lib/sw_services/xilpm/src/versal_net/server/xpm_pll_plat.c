/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xil_util.h"
#include "xpm_pll_plat.h"
#include "xpm_psm.h"
#include "xpm_regs.h"

void XPm_PllClearLockError(const XPm_PllClockNode* Pll)
{
	const XPm_Psm *Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL != Psm) {
		if (PM_CLK_APLL1 == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_APLL1_LOCK_MASK);
		} else if (PM_CLK_APLL2 == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_APLL2_LOCK_MASK);
		} else if (PM_CLK_RPLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_RPLL_LOCK_MASK);
		} else if (PM_CLK_FLXPLL == Pll->ClkNode.Node.Id) {
			XPm_Write32(Psm->PsmGlobalBaseAddr + PSM_ERR1_STATUS_OFFSET,
				    PSM_ERR1_STATUS_FLXPLL_LOCK_MASK);
		} else {
			/* Required due to MISRA */
		}
	}
}

void XPmClockPll_PlatReset(XPm_PllClockNode* Pll)
{
	(void)Pll;
}

static XStatus SavePllClockNode(XPm_PllClockNode* ThisData, u32** SavedData)
{
	XStatus Status = XST_FAILURE;

	BEGIN_SAVE_STRUCT((*SavedData), XPmClock_SaveClockNode, (&(ThisData->ClkNode)));
	SaveStruct(Status, done, ThisData->PllMode);
	SaveStruct(Status, done, ThisData->Context);
	END_SAVE_STRUCT((*SavedData));

	Status = XST_SUCCESS;
done:
	XPM_UPDATE_THROW_IF_ERROR(Status, ThisData);
	return Status;
}

static XStatus RestorePllClockNode(u32* SavedData, XPm_PllClockNode* ThisData)
{
	XStatus Status = XST_FAILURE;
	u32* DataAddr = NULL;

	Status = XPmClock_RestoreClockNode(SavedData, &(ThisData->ClkNode), &DataAddr);
	if (XST_SUCCESS != Status)
	{
		goto done;
	}
	RestoreStruct(DataAddr, ThisData->PllMode);
	RestoreStruct(DataAddr, ThisData->Context);
done:
	return Status;
}

XStatus XPmClockPll_DoSaveRestore(u32* SavedData, u32* ThisData, u32 Op)
{
	XStatus Status = XST_FAILURE;
	u32* StartAddr = NULL;
	if (XPLMI_STORE_DATABASE == Op){
		Status = SavePllClockNode((XPm_PllClockNode*)ThisData, &StartAddr);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = RestorePllClockNode(SavedData, (XPm_PllClockNode*)ThisData);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}
