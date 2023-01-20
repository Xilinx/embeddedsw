/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xplmi_dma.h"
#include "xplmi_plat.h"
#include "xplmi_scheduler.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_rpucore.h"
#include "xpm_npdomain.h"
#include "xpm_debug.h"

#define XPM_TCM_BASEADDRESS_MODE_OFFSET	0x80000U

/* Max number of HBM stacks on any device */
#define XPM_HBM_MAX_STACKS		2U

/* Scheduler Owner ID for HBM Monitoring Task */
#define XPM_HBM_TEMP_MON_SCHED_ID	PM_DEV_HBMMC_0

/*
 * Handler to read the temperature of a given HBM Stack
 */
static XStatus XPmMem_GetTempData(u32 BaseAddress, u32 *TempData)
{
	XStatus Status = XST_FAILURE;
	u32 TempVal = 0U;

	/* Unlock PCSR */
	XPm_UnlockPcsr(BaseAddress);

	/* Write CFG2 */
	XPm_Out32(BaseAddress + HBM_PHY_MS_CFG2_OFFSET, 0x200U);

	/* Write CFG1 */
	XPm_Out32(BaseAddress + HBM_PHY_MS_CFG1_OFFSET, 0xA0000FU);

	/* Write CFG64 to indicate a pending command */
	XPm_RMW32(BaseAddress + HBM_PHY_MS_CFG64_OFFSET, 0x1U, 0x1U);

	/* Wait until pending command is handled */
	Status = XPm_PollForZero(BaseAddress + HBM_PHY_MS_CFG64_OFFSET, 0x1, 2000U);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Read data */
	TempVal = XPm_In32(BaseAddress + HBM_PHY_MS_CFG66_OFFSET);

	/* Write a copy to CFG6 */
	XPm_Out32(BaseAddress + HBM_PHY_MS_CFG6_OFFSET, TempVal);

	/* Shift and extract data */
	TempVal >>= HBM_PHY_MS_CFG66_DATA_SHIFT;

	/* Check for data validity (valid bit must be 0) */
	if (0U != (TempVal & HBM_PHY_MS_CFG66_DATA_VALID_BIT)) {
		TempVal = 0U;
		goto done;
	}

	Status = XST_SUCCESS;

done:
	/* Data may be valid or invalid, clear other bits */
	*TempData = (TempVal & HBM_PHY_MS_CFG66_DATA_MASK);

	/* Lock PCSR */
	XPm_LockPcsr(BaseAddress);

	return Status;
}

/*
 * Handler for the HBM Temp monitoring task (called periodically)
 */
static int XPmMem_HBMTempMonitor(void *data)
{
	int Status = XST_FAILURE;
	u32 S0Temp = 0U, S1Temp = 0U;
	u32 TempVal = 0U, TempMax = 0U;
	const u32 *HbmPhyMs = (u32 *)data;

	/* Read HBM temp config register from RTCA */
	u32 HbmCfgAndMax = XPm_In32(XPLMI_RTCFG_HBM_TEMP_CONFIG_AND_MAX);

	/* Read PCSRs for blocks to check if the stacks are configured */
	u32 S0PcsrCntrl = XPm_In32(HbmPhyMs[0] + NPI_PCSR_CONTROL_OFFSET);
	u32 S1PcsrCntrl = XPm_In32(HbmPhyMs[1] + NPI_PCSR_CONTROL_OFFSET);

	/* HBM Stack 0 must be configured and temp monitoring must be enabled for it to read data */
	if ((NPI_PCSR_CONTROL_PCOMPLETE_MASK == (S0PcsrCntrl & NPI_PCSR_CONTROL_PCOMPLETE_MASK)) &&
	    (0U != (HbmCfgAndMax & XPM_RTCA_HBM_TEMP_CONFIG_STACK0_EN_MASK))) {
		/* Read the temperature data */
		(void)XPmMem_GetTempData(HbmPhyMs[0], &S0Temp);
		TempVal = S0Temp;
	}

	/* HBM Stack 1 must be configured and temp monitoring must be enabled for it to read data */
	if ((NPI_PCSR_CONTROL_PCOMPLETE_MASK == (S1PcsrCntrl & NPI_PCSR_CONTROL_PCOMPLETE_MASK)) &&
	    (0U != (HbmCfgAndMax & XPM_RTCA_HBM_TEMP_CONFIG_STACK1_EN_MASK))) {
		/* Read the temperature data */
		(void)XPmMem_GetTempData(HbmPhyMs[1], &S1Temp);
		TempVal |= (S1Temp << XPM_RTCA_HBM_TEMP_VAL_STACK1_TEMP_SHIFT);
	}

	/* Calculate max temp among both stacks */
	TempMax = (S0Temp >= S1Temp) ? S0Temp : S1Temp;

	/* Write temp values to RTCA */
	XPm_Out32(XPLMI_RTCFG_HBM_TEMP_VAL, TempVal);
	XPm_RMW32(XPLMI_RTCFG_HBM_TEMP_CONFIG_AND_MAX,
		  XPM_RTCA_HBM_TEMP_CONFIG_STACKS_MAX_TEMP_MASK,
		  (TempMax << XPM_RTCA_HBM_TEMP_CONFIG_STACKS_MAX_TEMP_SHIFT));

	Status = XST_SUCCESS;

	return Status;
}

/****************************************************************************/
/**
 * @brief  This function is used to start the task which enables HBM temperature
 * monitoring in PLM. This will only work on the devices with HBM nodes present
 * in the topology. If the feature is not enabled for any of the HBM stacks
 * (controlled through PLM RTCA), then the task is never started. During the task
 * execution, if found that no HBM stacks are configured in the design, then it
 * is removed from the task queue.
 *
 * @param  None
 *
 * @return XST_SUCCESS if successful, return error code otherwise
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmMem_HBMTempMonInitTask(void)
{
	XStatus Status = XST_FAILURE;
	static u32 HbmPhyMsAddr[XPM_HBM_MAX_STACKS];	/* HBM_PHY_MS block addresses */

	/* Read HBM temp config register from RTCA */
	u32 HbmCfgAndMax = XPm_In32(XPLMI_RTCFG_HBM_TEMP_CONFIG_AND_MAX);
	if (0U == (HbmCfgAndMax & XPM_RTCA_HBM_TEMP_CONFIG_STACK_EN_MASK)) {
		/* Do not fail since temp monitoring is not enabled for any HBM stack */
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if device has any HBM nodes, and compute HBM_PHY_MS base addresses */
	for (u32 i = (u32)XPM_NODEIDX_DEV_HBMMC_0; i <= (u32)XPM_NODEIDX_DEV_HBMMC_15; ++i) {
		const XPm_Device *Hbm = XPmDevice_GetByIndex(i);
		if (NULL == Hbm) {
			/* Do not fail since this device has no HBM stacks */
			Status = XST_SUCCESS;
			goto done;
		}
		/* Derive HBM PHY MiddleStack block addresses (TODO: Should pass through topology) */
		switch (i) {
		case (u32)XPM_NODEIDX_DEV_HBMMC_7:
			HbmPhyMsAddr[0] = Hbm->Node.BaseAddress + HBM_PHY_MS_OFF_FROM_PREV_HBMMC_MC;
			break;
		case (u32)XPM_NODEIDX_DEV_HBMMC_15:
			HbmPhyMsAddr[1] = Hbm->Node.BaseAddress + HBM_PHY_MS_OFF_FROM_PREV_HBMMC_MC;
			break;
		default:
			/* Nothing to do */
			break;
		}
	}

	/* Schedule the HBM temp monitoring task with periodicity of HBM_TEMP_MON_PERIOD */
	Status = XPlmi_SchedulerAddTask(XPM_HBM_TEMP_MON_SCHED_ID, XPmMem_HBMTempMonitor,
			NULL, HBM_TEMP_MON_PERIOD, XPLM_TASK_PRIORITY_0,
			(void *)HbmPhyMsAddr, XPLMI_PERIODIC_TASK);

done:
	return Status;
}

static const XPm_StateCap XPmDDRDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Cap = (u32)PM_CAP_CONTEXT,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = XPM_MAX_CAPABILITY | PM_CAP_UNUSABLE,
	},
};

static const XPm_StateTran XPmDDRDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Latency = XPM_DEF_LATENCY,
	},
};

/****************************************************************************/
/**
 * @brief  This function checks whether DRAMs are in self-refresh mode
 *
 * @return XST_SUCCESS if at least one DDRMC is enabled in the design and
 *         all the active DRAMs are in self-refresh mode,
 *         XST_FAILURE if no DDRMCs are enabled in the design or no DRAMs are
 *         in self-refresh mode
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmDDRDevice_IsInSelfRefresh(void)
{
	XStatus Status = XST_FAILURE;
	u32 IsActive = 0, IsInSelfRefresh = 0;

	for (u32 i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; i++) {
		const XPm_Device *Device = XPmDevice_GetById(DDRMC_DEVID(i));
		if ((NULL == Device) ||
		    ((u32)XPM_DEVSTATE_UNUSED == Device->Node.State)) {
			continue;
		}

		u32 BaseAddress = Device->Node.BaseAddress;
		u32 PcsrCntrl = XPm_In32(BaseAddress + NPI_PCSR_CONTROL_OFFSET);
		/* Check if DDRMC is enabled in the design */
		if (DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK != (PcsrCntrl & DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}
		IsActive++;

		u32 DramMode = XPm_In32(BaseAddress + DDRMC_MAIN_UB_OFFSET + DDRMC_MAIN_REG_COM_4_OFFSET);
		/* Check if DRAM is in self refresh mode */
		if (DDRMC_MAIN_SELF_REFRESH_MODE == (DramMode & DDRMC_MAIN_DRAM_MODE_REPORT_MASK)) {
			IsInSelfRefresh++;
		}
	}

	/**
	 * DRAMs are in self-refresh, if:
	 *  - At least one DDRMC is enabled in the design
	 *  - All the active DRAMs are in self-refresh mode
	 */
	if ((IsActive > 0) && (IsActive == IsInSelfRefresh)) {
		Status = XST_SUCCESS;
	}

	return Status;
}

static XStatus XPmDDRDevice_EnterSelfRefresh(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	u32 BaseAddress;
	u32 Reg, IsActive;
	u32 i;

	for (i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX;
	     i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(i));
		if (NULL == Device) {
			continue;
		}
		BaseAddress = Device->Node.BaseAddress;
		PmIn32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, IsActive);
		if (DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK != (IsActive & DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}

		/* Unlock DDRMC UB */
		XPm_UnlockPcsr(BaseAddress);

		/* Enable self-refresh */
		Reg = BaseAddress + DDRMC_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC_UB_PMC2UB_INTERRUPT_SPARE_0_MASK);
		Reg = BaseAddress + DDRMC_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_ACK_SPARE_0_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + DDRMC_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_DONE_SPARE_0_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		XPm_Out32(Reg, 0);

		/* Lock PCSR */
		XPm_LockPcsr(BaseAddress);
	}

	Device = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN));
	Status = XPmNpDomain_ClockGate((XPm_Node *)Device, 0);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus XPmDDRDevice_ExitSelfRefresh(void)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *Device;
	u32 BaseAddress;
	u32 Reg, IsActive;
	u32 i;

	Device = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN));
	Status = XPmNpDomain_ClockGate((XPm_Node *)Device, 1);
	if (XST_SUCCESS != Status) {
		DbgErr = XPM_INT_ERR_NOC_CLOCK_GATING;
		goto done;
	}

	for (i = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; i <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX;
	     i++) {
		Device = XPmDevice_GetById(DDRMC_DEVID(i));
		if (NULL == Device) {
			continue;
		}
		BaseAddress = Device->Node.BaseAddress;
		PmIn32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, IsActive);
		if (DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK != (IsActive & DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}

		/* Unlock DDRMC UB */
		XPm_UnlockPcsr(BaseAddress);

		/* Disable self-refresh */
		Reg = BaseAddress + DDRMC_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC_UB_PMC2UB_INTERRUPT_SR_EXIT_MASK);
		Reg = BaseAddress + DDRMC_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_ACK_SR_EXIT_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + DDRMC_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_DONE_SR_EXIT_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		XPm_Out32(Reg, 0);

		/* Lock PCSR */
		XPm_LockPcsr(BaseAddress);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static XStatus HandleDDRDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
		} else if ((u32)XPM_DEVSTATE_RUNTIME_SUSPEND == NextState) {
			Status = XPmDDRDevice_EnterSelfRefresh();
		} else {
			Status = XST_FAILURE;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNTIME_SUSPEND:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDDRDevice_ExitSelfRefresh();
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmDDRDeviceFsm = {
	DEFINE_DEV_STATES(XPmDDRDeviceStates),
	DEFINE_DEV_TRANS(XPmDDRDevTransitions),
	.EnterState = HandleDDRDeviceState,
};

XStatus HaltRpuCore(const XPm_Device *Rpu0, const XPm_Device *Rpu1,
			   const u32 Id, u32 *RpuMode)
{
	XStatus Status = XST_FAILURE;
	u32 Mode;

	Status = XPm_RpuGetOperMode(PM_DEV_RPU0_0, &Mode);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (XPM_RPU_MODE_SPLIT == Mode) {
		if (((PM_DEV_TCM_0_A == Id) || (PM_DEV_TCM_0_B == Id)) &&
		    ((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		if (((PM_DEV_TCM_1_A == Id) || (PM_DEV_TCM_1_B == Id)) &&
		    ((u8)XPM_DEVSTATE_RUNNING !=  Rpu1->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu1);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	if (XPM_RPU_MODE_LOCKSTEP == Mode)
	{
		if (((PM_DEV_TCM_0_A == Id) || (PM_DEV_TCM_0_B == Id) ||
		     (PM_DEV_TCM_1_A == Id) || (PM_DEV_TCM_1_B == Id)) &&
		     ((u8)XPM_DEVSTATE_RUNNING != Rpu0->Node.State)) {
			Status = XPmRpuCore_Halt(Rpu0);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
	}

	*RpuMode = Mode;
	Status = XST_SUCCESS;

done:
	return Status;
}

void XPm_GetRpuDevice(const XPm_Device **Rpu0Device,const XPm_Device **Rpu1Device,
	const u32 Id){
	/*warning fix*/
	(void)Id;
	(void)Rpu0Device;
	(void)Rpu1Device;
	*Rpu0Device = XPmDevice_GetById(PM_DEV_RPU0_0);
	*Rpu1Device = XPmDevice_GetById(PM_DEV_RPU0_1);
	return;
}

u32 XPm_CombTcm(const u32 Id, const u32 Mode)
{
	u32 Offset = 0;
	if ((PM_DEV_TCM_1_A == Id) || (PM_DEV_TCM_1_B == Id)) {
		if (XPM_RPU_MODE_LOCKSTEP == Mode) {
			Offset = XPM_TCM_BASEADDRESS_MODE_OFFSET;
		}
	}
	return Offset;
}

void XPm_AssignDdrFsm(XPm_MemDevice *MemDevice)
{
	MemDevice->Device.DeviceFsm = &XPmDDRDeviceFsm;
}
