/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xil_util.h"
#include "xpm_common.h"
#include "xpm_regulator.h"
#include "xpm_node.h"
#include "xpm_debug.h"
#include "xpm_power.h"

#if defined (RAIL_CONTROL)
static XPm_Regulator *PmRegulators[XPM_NODEIDX_POWER_REGULATOR_MAX];

XPm_Regulator *XPmRegulator_GetById(u32 Id)
{
	XPm_Regulator *Regulator = NULL;
	u32 NodeClass = NODECLASS(Id);
	u32 NodeSubClass = NODESUBCLASS(Id);
	u32 NodeType = NODETYPE(Id);
	u32 NodeIndex = NODEINDEX(Id);

	if (((u32)XPM_NODECLASS_POWER == NodeClass) &&
	    ((u32)XPM_NODESUBCL_POWER_REGULATOR == NodeSubClass) &&
	    ((u32)XPM_NODETYPE_POWER_REGULATOR == NodeType) &&
	    ((u32)XPM_NODEIDX_POWER_REGULATOR_MAX > NodeIndex)) {
		Regulator = PmRegulators[NodeIndex];
		/* Validate regulator node ID is same as given ID. */
		if ((NULL != Regulator) && (Id != Regulator->Node.Id)) {
			Regulator = NULL;
		}
	}

	return Regulator;
}

XStatus XPmRegulator_Init(XPm_Regulator *Regulator, u32 Id, const u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	u32 NodeIndex = NODEINDEX(Id);
	u32 Method, ControllerId;
	const u32 CopySize = 4U;

	if ((u32)XPM_NODEIDX_POWER_REGULATOR_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (2U >= NumArgs) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Method = (Args[1] & 0xFFU);
	ControllerId = Args[2];

	switch (Method) {
	case (u32)XPM_METHODTYPE_I2C:
		Regulator->I2cAddress = (u16)((Args[1] >> 16) & 0xFFU);
		Regulator->Cntrlr[XPM_I2C_CNTRLR] = XPmDevice_GetById(ControllerId);
		if ((NULL == Regulator->Cntrlr[XPM_I2C_CNTRLR]) ||
		    ((PM_DEV_I2C_PMC != ControllerId) && (PM_DEV_I2C_0 != ControllerId) &&
		     (PM_DEV_I2C_1 != ControllerId))) {
			DbgErr = XPM_INT_ERR_INVALID_NODE;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Regulator->Config.CmdLen = (u8)((Args[1] >> 8) & 0xFFU);
		for (u32 i = 3; i < NumArgs; i++) {
			Status = Xil_SMemCpy((void *)&Regulator->Config.CmdArr[(i - 3U) * 4U],
					     CopySize, (void *)&Args[i], CopySize, CopySize);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}

		Status = XST_SUCCESS;
		break;
	case (u32)XPM_METHODTYPE_GPIO:
		Regulator->Cntrlr[XPM_GPIO_CNTRLR] = XPmDevice_GetById(ControllerId);
		if ((NULL == Regulator->Cntrlr[XPM_GPIO_CNTRLR]) ||
		    ((PM_DEV_GPIO != ControllerId) && (PM_DEV_GPIO_PMC != ControllerId))) {
			DbgErr = XPM_INT_ERR_INVALID_NODE;
			Status = XST_INVALID_PARAM;
			goto done;
		}

		Status = XST_SUCCESS;
		break;
	default:
		DbgErr = XPM_INT_ERR_INVALID_PARAM;
		Status = XST_INVALID_PARAM;
		break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	PmRegulators[NodeIndex] = Regulator;
	XPmNode_Init(&Regulator->Node, Id, (u8)XPM_POWER_STATE_ON, 0U);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
#endif /* RAIL_CONTROL */
