/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpm_core.h"
#include "xpm_psm.h"

XStatus SkipRpuReset(const XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *DevTcmA0A = XPmDevice_GetById(PM_DEV_TCM_A_0A);
	const XPm_Device *DevTcmA0B = XPmDevice_GetById(PM_DEV_TCM_A_0B);
	const XPm_Device *DevTcmA0C = XPmDevice_GetById(PM_DEV_TCM_A_0C);
	const XPm_Device *DevTcmA1A = XPmDevice_GetById(PM_DEV_TCM_A_1A);
	const XPm_Device *DevTcmA1B = XPmDevice_GetById(PM_DEV_TCM_A_1B);
	const XPm_Device *DevTcmA1C = XPmDevice_GetById(PM_DEV_TCM_A_1C);
	const XPm_Device *DevTcmB0A = XPmDevice_GetById(PM_DEV_TCM_B_0A);
	const XPm_Device *DevTcmB0B = XPmDevice_GetById(PM_DEV_TCM_B_0B);
	const XPm_Device *DevTcmB0C = XPmDevice_GetById(PM_DEV_TCM_B_0C);
	const XPm_Device *DevTcmB1A = XPmDevice_GetById(PM_DEV_TCM_B_1A);
	const XPm_Device *DevTcmB1B = XPmDevice_GetById(PM_DEV_TCM_B_1B);
	const XPm_Device *DevTcmB1C = XPmDevice_GetById(PM_DEV_TCM_B_1C);

	if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) {
		if ((XPM_DSTN_CLUSTER_0 == GET_RPU_CLUSTER_ID(Core->Device.Node.Id)) &&
		    (NULL != DevTcmA0A) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA0A->Node.State) &&
		    (NULL != DevTcmA0B) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA0B->Node.State) &&
		    (NULL != DevTcmA0C) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA0C->Node.State) &&
		    (NULL != DevTcmA1A) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA1A->Node.State) &&
		    (NULL != DevTcmA1B) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA1B->Node.State) &&
		    (NULL != DevTcmA1C) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmA1C->Node.State)) {
			Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else if ((XPM_DSTN_CLUSTER_1 == GET_RPU_CLUSTER_ID(Core->Device.Node.Id)) &&
		    (NULL != DevTcmB0A) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB0A->Node.State) &&
		    (NULL != DevTcmB0B) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB0B->Node.State) &&
		    (NULL != DevTcmB0C) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB0C->Node.State) &&
		    (NULL != DevTcmB1A) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB1A->Node.State) &&
		    (NULL != DevTcmB1B) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB1B->Node.State) &&
		    (NULL != DevTcmB1C) && ((u8)XPM_DEVSTATE_RUNNING != DevTcmB1C->Node.State)) {
			Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		} else {
			/* Required for MISRA */
		}
	} else {
		Status = XPmDevice_Reset(&Core->Device, PM_RESET_ACTION_ASSERT);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus ResetAPUGic(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_Power *AcpuPwrNode;
	const XPm_Power *FpdPwrNode = XPmPower_GetById(PM_POWER_FPD);
	u32 NodeId;

	if (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) &&
	    (NULL != FpdPwrNode) && ((u8)XPM_POWER_STATE_OFF != FpdPwrNode->Node.State)) {
		for (NodeId = PM_POWER_ACPU_0_0; NodeId <= PM_POWER_ACPU_3_3; NodeId++) {
			AcpuPwrNode = XPmPower_GetById(NodeId);
			if ((NULL != AcpuPwrNode) && ((u8)XPM_POWER_STATE_OFF !=
			    AcpuPwrNode->Node.State)) {
				break;
			}
		}
		if (PM_POWER_ACPU_3_3 < NodeId) {
			Status = XPmReset_AssertbyId(PM_RST_ACPU_GIC, (u32)PM_RESET_ACTION_PULSE);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

void DisableWake(const struct XPm_Core *Core)
{
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Core->Device.Node.Id)) {
		DISABLE_WAKE0(Core->WakeUpMask);
	} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) {
		DISABLE_WAKE1(Core->WakeUpMask);
	} else {
		/* Required for MISRA */
	}
};

void EnableWake(const struct XPm_Core *Core)
{
	if ((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(Core->Device.Node.Id)) {
		ENABLE_WAKE0(Core->WakeUpMask);
	} else if ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(Core->Device.Node.Id)) {
		ENABLE_WAKE1(Core->WakeUpMask);
	} else {
		/* Required for MISRA */
	}
};
