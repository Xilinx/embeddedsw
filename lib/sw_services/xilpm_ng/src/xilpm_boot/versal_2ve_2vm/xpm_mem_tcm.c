/******************************************************************************
* Copyright (c) 2025 - 2024 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xpm_mem.h"
#include "xpm_debug.h"
#include "xpm_rpucore.h"
#include "xpm_mem_tcm.h"

/**
 * XPmMem_TcmResetReleaseById - Release reset for TCM by ID.
 *
 * @NodeId: The ID of the TCM to release reset for.
 *
 * This function releases the reset for the specified TCM ID.
 *
 * Return:
 * XST_SUCCESS if successful, otherwise an error code.
 */

XStatus XPmMem_TcmResetReleaseById(u32 NodeId)
{
	XStatus Status = XST_FAILURE;
	/** Get RpuId */
	u32 RpuId = 0U;
	Status = XPm_GetRpuByTcmId(NodeId, &RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get RPU ID for TCM ID: 0x%x\n", NodeId);
		goto done;
	}
	Status = XPmRpuCore_ResetAndHalt(RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to halt RPU ID: 0x%x\n", RpuId);
		goto done;
	}
	Status = XPmRpuCore_ReleaseReset(RpuId);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to release reset for RPU ID: 0x%x\n", RpuId);
		goto done;
	}
done:
	return Status;
}

/**
 * XPm_GetRpuByTcmId - Get the RPU ID for the given TCM ID.
 *
 * @TcmId: The ID of the TCM to get the RPU ID for.
 * @RpuId: Pointer to store the RPU ID.
 *
 * This function maps the TCM ID to the corresponding RPU ID.
 *
 * Return:
 * XST_SUCCESS if successful, otherwise an error code.
 */
XStatus XPm_GetRpuByTcmId(u32 TcmId, u32 *RpuId)
{
	XStatus Status = XST_FAILURE;
	switch (TcmId)
	{
	case PM_DEV_TCM_A_0A:
	case PM_DEV_TCM_A_0B:
	case PM_DEV_TCM_A_0C:
		*RpuId = PM_DEV_RPU_A_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_A_1A:
	case PM_DEV_TCM_A_1B:
	case PM_DEV_TCM_A_1C:
		*RpuId = PM_DEV_RPU_A_1;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_B_0A:
	case PM_DEV_TCM_B_0B:
	case PM_DEV_TCM_B_0C:
		*RpuId = PM_DEV_RPU_B_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_B_1A:
	case PM_DEV_TCM_B_1B:
	case PM_DEV_TCM_B_1C:
		*RpuId = PM_DEV_RPU_B_1;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_C_0A:
	case PM_DEV_TCM_C_0B:
	case PM_DEV_TCM_C_0C:
		*RpuId = PM_DEV_RPU_C_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_C_1A:
	case PM_DEV_TCM_C_1B:
	case PM_DEV_TCM_C_1C:
		*RpuId = PM_DEV_RPU_C_1;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_D_0A:
	case PM_DEV_TCM_D_0B:
	case PM_DEV_TCM_D_0C:
		*RpuId = PM_DEV_RPU_D_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_D_1A:
	case PM_DEV_TCM_D_1B:
	case PM_DEV_TCM_D_1C:
		*RpuId = PM_DEV_RPU_D_1;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_E_0A:
	case PM_DEV_TCM_E_0B:
	case PM_DEV_TCM_E_0C:
		*RpuId = PM_DEV_RPU_E_0;
		Status = XST_SUCCESS;
		break;
	case PM_DEV_TCM_E_1A:
	case PM_DEV_TCM_E_1B:
	case PM_DEV_TCM_E_1C:
		*RpuId = PM_DEV_RPU_E_1;
		Status = XST_SUCCESS;
		break;
	default:
		PmErr("Invalid TCM ID: 0x%x\n", TcmId);
		Status = XST_INVALID_PARAM;
		goto done;
	}
	if (XST_SUCCESS != Status) {
		PmErr("Failed to get RPU ID for TCM ID: 0x%x\n", TcmId);
		goto done;
	}
done:
	return Status;
}
