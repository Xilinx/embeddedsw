/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_regulator.h"
#include "xpm_node.h"
#include "xpm_debug.h"
#include "xpm_power.h"

static XPm_Regulator *PmRegulators[XPM_NODEIDX_POWER_REGULATOR_MAX];

XPm_Regulator *XPmRegulator_GetById(u32 Id)
{
	XPm_Regulator *Regulator = NULL;
	u32 NodeClass = NODECLASS(Id);
	u32 NodeSubClass = NODESUBCLASS(Id);
	u32 NodeIndex = NODEINDEX(Id);

	if (((u32)XPM_NODECLASS_POWER == NodeClass) &&
	    ((u32)XPM_NODESUBCL_POWER_REGULATOR == NodeSubClass) &&
	    ((u32)XPM_NODEIDX_POWER_REGULATOR_MAX > NodeIndex)) {
		Regulator = PmRegulators[NodeIndex];
		/* Validate power node ID is same as given ID. */
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
	u32 SlaveAddress, i;
	u32 NodeIndex = NODEINDEX(Id);
	u8 Method;

	if ((u32)XPM_NODEIDX_POWER_REGULATOR_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if( 1U >= NumArgs) {
		DbgErr = XPM_INT_ERR_INVALID_ARGS;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Method = (u8)Args[1] & 0xFFU;
	Regulator->CtrlMethod = (XPm_ControlMethod)Method;

	switch (Regulator->CtrlMethod) {
	case XPM_METHODTYPE_I2C:
		SlaveAddress = (Args[1] >> 16) & 0xFFU;
		Regulator->ParentId = Args[2];
		Regulator->Config.CmdLen = (u8)((Args[1] >> 8) & 0xFFU);
		for (i = 3; i < NumArgs; i++) {
			(void *)memcpy((void *)&Regulator->Config.CmdArr[(i - 3U) * 4U],
				       (void *)&Args[i], 4);
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
	XPmNode_Init(&Regulator->Node, Id, (u8)XPM_POWER_STATE_ON, SlaveAddress);

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
