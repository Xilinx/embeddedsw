/******************************************************************************
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_regulator.h"

XStatus XPmRegulator_SaveRestore(u32* SavedData, u32* ThisData, u32 Op){
	u32* StartAddr = NULL;
	XStatus Status = XST_FAILURE;
	if (XPLMI_STORE_DATABASE  == Op){
		Status = XPmNode_SaveNode((XPm_Node*)ThisData, &StartAddr);
		goto done;
	}
	if (XPLMI_RESTORE_DATABASE == Op){
		Status = XPmNode_RestoreNode(SavedData, (XPm_Node*)ThisData, &StartAddr);
		goto done;
	}
	Status = XPM_UPDATE_UNKNOWN_OP;
done:
	return Status;
}