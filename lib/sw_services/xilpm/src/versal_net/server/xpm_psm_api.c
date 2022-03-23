/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_psm_api.h"
#include "xpm_nodeid.h"
#include "xpm_ipi.h"
#include "xpm_api.h"
#include "xplmi.h"

u32 ProcDevList[PROC_DEV_MAX] = {
	[ACPU_0] = PM_DEV_ACPU_0_0,
	[ACPU_1] = PM_DEV_ACPU_0_1,
    [ACPU_2] = PM_DEV_ACPU_0_2,
    [ACPU_3] = PM_DEV_ACPU_0_3,
    [ACPU_4] = PM_DEV_ACPU_1_0,
    [ACPU_5] = PM_DEV_ACPU_1_1,
    [ACPU_6] = PM_DEV_ACPU_1_2,
    [ACPU_7] = PM_DEV_ACPU_1_3,
	[RPU0_0] = PM_DEV_RPU_A_0,
	[RPU0_1] = PM_DEV_RPU_A_1,
    [RPU1_0] = PM_DEV_RPU_B_0,
    [RPU1_1] = PM_DEV_RPU_B_1,
};
/* This replicates PsmToPlmEvent stored at PSM reserved RAM location */
volatile struct PsmToPlmEvent_t *PsmToPlmEvent;

/****************************************************************************/
/**
 * @brief This Function requests for PSM_TO_PLM_EVENT_ADDR to PSM.
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_GetPsmToPlmEventAddr(void)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0};

	Payload[0] = PSM_API_GET_PSM_TO_PLM_EVENT_ADDR;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiRead(PSM_IPI_INT_MASK, &Response);
	if (XST_SUCCESS == Status) {
		PsmToPlmEvent = (struct PsmToPlmEvent_t *)Response[1];
	}

done:
	return Status;
}
