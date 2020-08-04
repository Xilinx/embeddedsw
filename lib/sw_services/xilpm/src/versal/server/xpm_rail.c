/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_rail.h"
#include "xpm_debug.h"
#include "xpm_node.h"

/****************************************************************************/
/**
 * @brief  Initialize rail node base class
 *
 * @param  Rail: Pointer to an uninitialized power rail struct
 * @param  RailId: Node Id assigned to a Power Rail node
 * @param  Args: Arguments for power rail
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Args is dependent on the rail type. Passed arguments will be different
 *		 for mode type and power good types.
 *
 ****************************************************************************/
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, u32 *Args)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0;
	u32 Type;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 NodeIndex = NODEINDEX(RailId);
	if ((u32)XPM_NODEIDX_POWER_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Type = Args[1];
	switch (Type) {
	case (u32)XPM_RAILTYPE_MODE:
		/*TODO: Support for mode type will be added with rail control */
		DbgErr = XPM_INT_ERR_NO_FEATURE;
		Status = XST_NO_FEATURE;
		break;
	case (u32)XPM_RAILTYPE_PGOOD:
		Rail->Source = (XPm_PgoodSource)Args[2];
		BaseAddress = Args[3];
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

	Status = XPmPower_Init(&Rail->Power, RailId, BaseAddress, NULL);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
