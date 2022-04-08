/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_psm_api.h"
#include "xpm_nodeid.h"
#include "xpm_ipi.h"
#include "xpm_api.h"
#include "xplmi.h"

static XPlmi_ModuleCmd XPlmi_PsmCmds[PSM_API_MAX+1];
static XPlmi_Module XPlmi_Psm =
{
	XPLMI_MODULE_XILPSM_ID,
	XPlmi_PsmCmds,
	PSM_API_MAX+1,
	NULL,
	NULL
};

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

static int XPm_ProcessPsmCmd(XPlmi_Cmd * Cmd)
{
	XStatus Status = XST_FAILURE, EventStatus;
	u32 Idx;
    Status = XST_SUCCESS;
	PmDbg("Processing Psm Event\n\r");

	/* Check for the power up/down event register */
    /*TBD: Check for LPD power node is not equals to XPM_POWER_STATE_OFF*/
	for (Idx = 0; (Idx < ARRAY_SIZE(ProcDevList)); Idx++) {
		if (PsmToPlmEvent->Event[Idx] == PWR_UP_EVT) {
			/* Clear power up event register bit */
			PsmToPlmEvent->Event[Idx] = 0;
			EventStatus = XPm_WakeUpEvent(ProcDevList[Idx]);
			if (XST_SUCCESS != EventStatus) {
				Status = EventStatus;
				PmErr("Err %d in wakeup of 0x%x\r\n",
						EventStatus, ProcDevList[Idx]);
			}
		} else if (PsmToPlmEvent->Event[Idx] == PWR_DWN_EVT) {
			/* Clear power down event register bit */
			PsmToPlmEvent->Event[Idx] = 0;
			EventStatus = XPm_PwrDwnEvent(ProcDevList[Idx]);
			if (XST_SUCCESS != EventStatus) {
				Status = EventStatus;
				PmErr("Err %d in powerdown of 0x%x\r\n",
						EventStatus, ProcDevList[Idx]);
			}
		} else {
			/* Required due to MISRA */
			PmDbg("Invalid PSM event %d\r\n", PsmToPlmEvent->Event[Idx]);
		}
	}

	Cmd->Response[0] = (u32)Status;

	if (XST_SUCCESS == Status) {
		Cmd->ResumeHandler = NULL;
	} else {
		PmErr("Error %d in handling PSM event\r\n", Status);
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief	Initialize PSM module which processes IPI from PSM
 *
 * @param	None
 *
 * @return	None
 *
 * @note	None
 *
 ****************************************************************************/
void XPm_PsmModuleInit(void)
{
	for (u32 Idx = 1; Idx < XPlmi_Psm.CmdCnt; Idx++) {
		XPlmi_PsmCmds[Idx].Handler = XPm_ProcessPsmCmd;
	}
	XPlmi_ModuleRegister(&XPlmi_Psm);
}

/****************************************************************************/
/**
 * @brief This Function will power up processor by sending IPI to PSM for
 *       performing direct power up operation.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_DirectPwrUp(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0] = PSM_API_DIRECT_PWR_UP;
	Payload[1] = DeviceId;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function will power down processor by sending IPI to PSM for
 *       performing direct power down operation.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_DirectPwrDwn(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	Payload[0] = PSM_API_DIRECT_PWR_DWN;
	Payload[1] = DeviceId;

	Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function is called by PSM to wake processor.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_WakeUpEvent(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;

	/*TBD: Do not process anything if proc is already running */

    /*TBD: read the cpuidleflag*/
	/*TBD: Update the state and its parent use counts in case of CPU idle */
    Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, DeviceId, 0, 0, 0,
				   XPLMI_CMD_SECURE);

	return Status;
}

/****************************************************************************/
/**
 * @brief This Function is called by PSM to perform actions to finish suspend
 *       procedur of processor.
 *
 * @param DeviceId	Device ID of processor
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_PwrDwnEvent(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
    (void)DeviceId;
    /*TBD: Do not process anything if proc is already down */
    /*TBD: do processpendingforcepwrdwn if the core state is not XPM_DEVSTATE_SUSPENDING */

    XPm_DirectPwrDwn(DeviceId);

    /*TBD: Update the state and its parent use counts in case of CPU idle */
    /*TBD: PowerDown the Core*/
    /*TBD: Get the subsystem id, if its state is SUSPENDING then do:
        - update the scheduled requirement
        - Read PSM_PGGS_0 and PSM_PGGS_1 registers value and do not
            power down LPD if values are non zero
        - Release devices requested by PLM to turn of LPD domain
        - Clear the pending suspend cb reason
        - set the subsystem state to SUSPENDED
    */
    Status = XST_SUCCESS;
    return Status;
}

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
