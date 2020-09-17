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
 * @param  NumArgs: Number of arguments for power rail
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or error code
 *
 * @note Args is dependent on the rail type. Passed arguments will be different
 *		 for mode type and power good types.
 *
 ****************************************************************************/
XStatus XPmRail_Init(XPm_Rail *Rail, u32 RailId, u32 *Args, u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	u32 BaseAddress = 0;
	u32 Type, i, j, k;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	u32 NodeIndex = NODEINDEX(RailId);
	if ((u32)XPM_NODEIDX_POWER_MAX <= NodeIndex) {
		DbgErr = XPM_INT_ERR_INVALID_NODE_IDX;
		Status = XST_INVALID_PARAM;
		goto done;
	}

	if (1U < NumArgs) {
		Type = Args[1];
		switch (Type) {
		case (u32)XPM_RAILTYPE_MODE_PMBUS:
			Rail->ParentId = Args[2];
			Rail->NumModes = (u8)Args[3];
			if (Rail->NumModes > (u8)MAX_MODES) {
				DbgErr = XPM_INT_ERR_INVALID_PARAM;
				Status = XST_INVALID_PARAM;
				goto done;
			}

			k = 4;
			/* Format as below:
			 * add node rail , parent regulator,
			 * num_modes_supported, mode0 id+len of command bytes,
			 * i2c commands,
			 * mode1 id+len of command bytes, i2c commands.
			 *
			 * For example,
			 * pm_add_node 0x432802b 0x1 0x442c002 0x2 0x300
			 *             0x02000002 0x01021a02 0x00 0x301
			 *             0x02000002 0x01021a02 0x80
			 */
			for (i = 0U; i < Rail->NumModes; i++) {
				Rail->I2cModes[i].CmdLen = (u8)(Args[k++] >> 8) &
							   0xFFU;
				for (j = 0; j < Rail->I2cModes[i].CmdLen; j++) {
					(void)XPlmi_MemCpy(&Rail->I2cModes[i].CmdArr[j * 4U],
							   &Args[k++], 4);
				}
			}

			Status = XST_SUCCESS;
			break;
		case (u32)XPM_RAILTYPE_PGOOD:
			Rail->Source = (XPm_PgoodSource)Args[2];
			BaseAddress =  Args[3];
			Rail->Power.Node.BaseAddress = Args[3];
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
	}

	if (NULL == XPmPower_GetById(RailId)) {
		Status = XPmPower_Init(&Rail->Power, RailId, BaseAddress, NULL);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_POWER_DOMAIN_INIT;
		}
	}

	Rail->Power.Node.State = (u8)XPM_POWER_STATE_ON;

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}
