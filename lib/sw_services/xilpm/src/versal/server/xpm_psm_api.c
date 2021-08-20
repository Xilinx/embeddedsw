/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "sleep.h"
#include "xplmi.h"
#include "xplmi_modules.h"
#include "xpm_defs.h"
#include "xpm_psm_api.h"
#include "xpm_psm.h"
#include "xpm_common.h"
#include "xpm_core.h"
#include "xpm_device.h"
#include "xpm_ipi.h"
#include "xpm_regs.h"
#include "xpm_subsystem.h"
#include "xpm_requirement.h"
#include "xpm_cpmdomain.h"

static XPlmi_ModuleCmd XPlmi_PsmCmds[PSM_API_MAX+1];
static XPlmi_Module XPlmi_Psm =
{
	XPLMI_MODULE_XILPSM_ID,
	XPlmi_PsmCmds,
	PSM_API_MAX+1,
	NULL,
};

u32 ProcDevList[PROC_DEV_MAX] = {
	[ACPU_0] = PM_DEV_ACPU_0,
	[ACPU_1] = PM_DEV_ACPU_1,
	[RPU0_0] = PM_DEV_RPU0_0,
	[RPU0_1] = PM_DEV_RPU0_1,
};

/* This replicates PsmToPlmEvent stored at PSM reserved RAM location */
volatile struct PsmToPlmEvent_t *PsmToPlmEvent =
				(struct PsmToPlmEvent_t *)PSM_TO_PLM_EVENT_ADDR;

static int XPm_ProcessPsmCmd(XPlmi_Cmd * Cmd)
{
	XStatus Status = XST_FAILURE, EventStatus;
	u32 Idx;
	const XPm_Power *Lpd;

	PmDbg("Processing Psm Event\n\r");

	Lpd = XPmPower_GetById(PM_POWER_LPD);
	if (NULL == Lpd) {
		goto done;
	}
	Status = XST_SUCCESS;

	/* Check for the power up/down event register */
	for (Idx = 0; ((u8)XPM_POWER_STATE_OFF != Lpd->Node.State) && (Idx < ARRAY_SIZE(ProcDevList)); Idx++) {
		if (PsmToPlmEvent->Event[Idx] == PWR_UP_EVT) {
			/* Clear power up event register bit */
			PsmToPlmEvent->Event[Idx] = 0;

			EventStatus = XPm_WakeUpEvent(ProcDevList[Idx]);
			if (EventStatus != XST_SUCCESS) {
				Status = EventStatus;
				PmErr("Err %d in wakeup of 0x%x\r\n",
						EventStatus, ProcDevList[Idx]);
			}
		} else if (PsmToPlmEvent->Event[Idx] == PWR_DWN_EVT) {
			/* Clear power down event register bit */
			PsmToPlmEvent->Event[Idx] = 0;

			EventStatus = XPm_PwrDwnEvent(ProcDevList[Idx]);
			if (EventStatus != XST_SUCCESS) {
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

done:
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
	u32 Idx;

	for (Idx = 1; Idx < XPlmi_Psm.CmdCnt; Idx++) {
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
	XPm_Core *Core;
	XPm_Subsystem *Subsystem;
	u32 SubsystemId;
	const XPm_Power *Lpd;
	u32 CpuIdleFlag = 0;
	u32 PsmPggs0Val;
	u32 PsmPggs1Val;
	const XPm_Psm *Psm;

	if (((u32)XPM_NODECLASS_DEVICE != NODECLASS(DeviceId)) ||
	    ((u32)XPM_NODESUBCL_DEV_CORE != NODESUBCLASS(DeviceId))) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	Core = (XPm_Core *)XPmDevice_GetById(DeviceId);

	/* Do not process anything if proc is already down */
	if ((u8)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((u8)XPM_DEVSTATE_SUSPENDING != Core->Device.Node.State) {
		Status = XPmCore_ProcessPendingForcePwrDwn(DeviceId);
		goto done;
	}

	Status = XPmCore_GetCPUIdleFlag(Core, &CpuIdleFlag);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Update the state and its parent use counts in case of CPU idle */
	if (1U == CpuIdleFlag) {
		Status = XPmCore_AfterDirectPwrDwn(Core);
		goto done;
	}

	if (NULL != Core->CoreOps->PowerDown) {
		Status = Core->CoreOps->PowerDown(Core);
	}

	SubsystemId = XPmDevice_GetSubsystemIdOfCore((XPm_Device *)Core);

	Subsystem = XPmSubsystem_GetById(SubsystemId);
	if (NULL == Subsystem) {
		Status = XST_FAILURE;
		goto done;
	}

	if ((u8)SUSPENDING == Subsystem->State) {
		Status = XPmRequirement_UpdateScheduled(Subsystem, 1U);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
		if (NULL == Psm) {
			Status = XST_FAILURE;
			goto done;
		}

		/**
		 * Read PSM_PGGS_0 and PSM_PGGS_1 registers value and do not
		 * power down LPD if values are non zero, because both registers
		 * are used by users and if LPD powers down then PGGS values
		 * will be lost.
		 */
		PmIn32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_PGGS0_OFFSET,
		       PsmPggs0Val);
		PmIn32(Psm->PsmGlobalBaseAddr + PSM_GLOBAL_PGGS1_OFFSET,
		       PsmPggs1Val);

		/* Release devices requested by PLM to turn of LPD domain */
		Lpd = XPmPower_GetById(PM_POWER_LPD);
		if (((Lpd->UseCount > 0U) && (Lpd->UseCount <= 3U)) &&
		    (0U == PsmPggs0Val) && (0U == PsmPggs1Val) &&
		    (((u32)XPM_NODETYPE_DEV_CORE_APU == NODETYPE(DeviceId)) ||
		     ((u32)XPM_NODETYPE_DEV_CORE_RPU == NODETYPE(DeviceId)))) {
			Status = XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_IPI_PMC,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				PmErr("Error %d in  XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_IPI_PMC)\r\n");
				goto done;
			}
#ifdef DEBUG_UART_PS
			Status = XPmDevice_Release(PM_SUBSYS_PMC, NODE_UART,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				PmErr("PMC Error %d in  XPmDevice_Release(PM_SUBSYS_DEFAULT, PM_DEV_UART_0)\r\n");
				goto done;
			}
#endif
			/* Power down PSM core */
			Core = (XPm_Core *)XPmDevice_GetById(PM_DEV_PSM_PROC);
			if (NULL != Core->CoreOps->PowerDown) {
				Status = Core->CoreOps->PowerDown(Core);
				if (XST_SUCCESS != Status) {
					PmErr("Error %d in PSM core PowerDown\r\n");
					goto done;
				}
			}

			Status = XPmDevice_Release(PM_SUBSYS_PMC, PM_DEV_PSM_PROC,
						   XPLMI_CMD_SECURE);
			if (XST_SUCCESS != Status) {
				PmErr("Error %d in  XPmDevice_Release(PM_SUBSYS_DEFAULT, PM_DEV_PSM_PROC)\r\n");
				goto done;
			}
		}
		/* Clear the pending suspend cb reason */
		Subsystem->PendCb.Reason = 0U;

		Status = XPmSubsystem_SetState(SubsystemId, (u32)SUSPENDED);
	} else {
		Status = XST_SUCCESS;
	}

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
	u32 CpuIdleFlag = 0;
	XPm_Core *Core = (XPm_Core *)XPmDevice_GetById(DeviceId);

	/* Do not process anything if proc is already running */
	if ((u8)XPM_DEVSTATE_RUNNING == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	Status = XPmCore_GetCPUIdleFlag(Core, &CpuIdleFlag);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (1U == CpuIdleFlag) {
		/* Update the state and its parent use counts in case of CPU idle */
		Status = XPmCore_AfterDirectWakeUp(Core);
	} else {
		Status = XPm_RequestWakeUp(PM_SUBSYS_PMC, DeviceId, 0, 0, 0,
					   XPLMI_CMD_SECURE);
	}

done:
	return Status;
}

/****************************************************************************/
/**
 * @brief This Function sends a CCIX_EN IPI to PSM if it design used is a
 * valid CPM CCIX design.
 *
 * @param PowerId	NodeId of CPM Power Domain
 *
 * @return XST_SUCCESS if successful else XST_FAILURE or an error code.
 *
 * @note none
 *
 ****************************************************************************/
XStatus XPm_CCIXEnEvent(u32 PowerId)
{
	u32 Payload[PAYLOAD_ARG_CNT];
	XStatus Status = XST_FAILURE;
	const XPm_CpmDomain *Cpm;
	u32 RegAddr;
	u32 RegVal;

	switch (PowerId) {
	case PM_POWER_CPM5:
		RegAddr	= CPM5_PCIE_ATTRIB_0_TDVSEC_NXT_PTR;
		Status = XST_SUCCESS;
		break;
	case PM_POWER_CPM:
		RegAddr	= PCIE_ATTRIB_0_TDVSEC_NXT_PTR;
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPM_INVALID_PWRDOMAIN;
		break;
	}

	if (XST_SUCCESS != Status) {
		goto done;
	}

	Cpm = (XPm_CpmDomain *)XPmPower_GetById(PowerId);
	if (NULL == Cpm) {
		Status = XPM_INVALID_PWRDOMAIN;
		goto done;
	}

	PmIn32(RegAddr, RegVal);

	if (RegVal == DVSEC_PCSR_START_ADDR)  {
		Payload[0] = PSM_API_CCIX_EN;
		Payload[1] = PowerId;
		Payload[2] = Cpm->CpmSlcrBaseAddr;

		Status = XPm_IpiSend(PSM_IPI_INT_MASK, Payload);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		Status = XPm_IpiReadStatus(PSM_IPI_INT_MASK);
	} else {
		Status = XST_SUCCESS;
	}

done:
	return Status;
}
