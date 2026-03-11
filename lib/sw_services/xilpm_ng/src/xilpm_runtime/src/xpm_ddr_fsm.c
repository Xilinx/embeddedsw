/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_core.h"
#include "xpm_ddr_fsm.h"
#include "xpm_debug.h"
#include "xpm_fsm.h"
#include "xpm_mem.h"
#include "xpm_regs.h"
#include "xpm_device.h"

/****************************************************************************/
/**
 * @brief Bring up DDR from UNUSED state to RUNNING state
 *
 * @param Device DDR device pointer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus ActionDDRBringUp(XPm_Device* const Device)
{
	XStatus Status = XST_SUCCESS;

	/* Power up DDR device and transition to RUNNING state */
	Status = XPmDevice_BringUp(Device);
	if (XST_SUCCESS != Status) {
		PmErr("Failed to bring up DDR device\r\n");
	}

	return Status;
}

/****************************************************************************/
/**
 * @brief Exit DDR from self-refresh mode
 *
 * @param Device DDR device pointer (unused)
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus ActionDDRExitSelfRefresh(XPm_Device* const Device)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *DdrMc;
	u32 BaseAddress;
	u32 IsActive;
	u32 Reg;
	u32 idx;

	(void)Device;

	DdrMc = XPmDevice_GetById(DDRMC_DEVID((u32)XPM_NODEIDX_DEV_DDRMC_MIN));
	if (NULL == DdrMc) {
		Status = XST_DEVICE_NOT_FOUND;
		DbgErr = XPM_INT_ERR_INVALID_DEVICE;
		goto done;
	}

	for (idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; idx <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; idx++) {
		DdrMc = XPmDevice_GetById(DDRMC_DEVID(idx));
		if ((NULL == DdrMc) || ((u8)XPM_DEVSTATE_RUNNING != DdrMc->Node.State)) {
			continue;
		}

		BaseAddress = DdrMc->Node.BaseAddress;
		PmIn32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, IsActive);
		if (NPI_PCSR_CONTROL_PCOMPLETE_MASK != (IsActive & NPI_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}

		/* Unlock DDRMC UB */
		XPm_UnlockPcsr(BaseAddress);

		/* Request for self-refresh exit */
		Reg = BaseAddress + DDRMC5E_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC5E_UB_PMC2UB_INTERRUPT_SR_EXIT_MASK);

		/* Poll UB2PMC_ACK register until sr_exit bit sets */
		Reg = BaseAddress + DDRMC5E_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC5E_UB_UB2PMC_ACK_SR_EXIT_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		/* Clear ACK */
		XPm_RMW32(Reg, DDRMC5E_UB_UB2PMC_ACK_SR_EXIT_MASK, ~DDRMC5E_UB_UB2PMC_ACK_SR_EXIT_MASK);

		/* Poll UB2PMC_DONE register until sr_exit bit sets */
		Reg = BaseAddress + DDRMC5E_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC5E_UB_UB2PMC_DONE_SR_EXIT_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		/* Clear DONE */
		XPm_RMW32(Reg, DDRMC5E_UB_UB2PMC_DONE_SR_EXIT_MASK, ~DDRMC5E_UB_UB2PMC_DONE_SR_EXIT_MASK);

		/* Lock PCSR */
		XPm_LockPcsr(BaseAddress);
	}
done:
	XPm_PrintDbgErr(Status, DbgErr);

	return Status;
}

/****************************************************************************/
/**
 * @brief Enter DDR into self-refresh mode
 *
 * @param Device DDR device pointer (unused)
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus ActionDDREnterSelfRefresh(XPm_Device* const Device)
{
	XStatus Status = XST_FAILURE;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;
	const XPm_Device *DdrMc;
	u32 BaseAddress;
	u32 IsActive;
	u32 Reg;
	u32 idx;

	(void)Device;

	for (idx = (u32)XPM_NODEIDX_DEV_DDRMC_MIN; idx <= (u32)XPM_NODEIDX_DEV_DDRMC_MAX; idx++) {
		DdrMc = XPmDevice_GetById(DDRMC_DEVID(idx));
		if ((NULL == DdrMc) || ((u8)XPM_DEVSTATE_RUNNING != DdrMc->Node.State)) {
			continue;
		}

		BaseAddress = DdrMc->Node.BaseAddress;
		PmIn32(BaseAddress + NPI_PCSR_CONTROL_OFFSET, IsActive);
		if (NPI_PCSR_CONTROL_PCOMPLETE_MASK != (IsActive & NPI_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}

		/* Unlock DDRMC UB PCSR registers */
		XPm_UnlockPcsr(BaseAddress);

		/* Request self-refresh entry */
		Reg = BaseAddress + DDRMC5E_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC5E_UB_PMC2UB_INTERRUPT_SR_ENTRY_MASK);

		/* Poll UB2PMC_ACK register until sr_entry sets */
		Reg = BaseAddress + DDRMC5E_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC5E_UB_UB2PMC_ACK_SR_ENTRY_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		/* Clear ACK */
		XPm_RMW32(Reg, DDRMC5E_UB_UB2PMC_ACK_SR_ENTRY_MASK, ~DDRMC5E_UB_UB2PMC_ACK_SR_ENTRY_MASK);

		/* Poll UB2PMC_DONE register until sr_entry sets */
		Reg = BaseAddress + DDRMC5E_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC5E_UB_UB2PMC_DONE_SR_ENTRY_MASK, XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			/* Lock PCSR */
			XPm_LockPcsr(BaseAddress);
			goto done;
		}
		/* Clear DONE */
		XPm_RMW32(Reg, DDRMC5E_UB_UB2PMC_DONE_SR_ENTRY_MASK, ~DDRMC5E_UB_UB2PMC_DONE_SR_ENTRY_MASK);

		/* Lock PCSR */
		XPm_LockPcsr(BaseAddress);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);

	return Status;
}

/****************************************************************************/
/**
 * @brief Shutdown DDR device
 *
 * @param Device DDR device pointer
 *
 * @return XST_SUCCESS on success, error code otherwise
 *
 ****************************************************************************/
static XStatus ActionDDRShutdown(XPm_Device* const Device)
{
	XStatus Status = XST_SUCCESS;
	u16 DbgErr = XPM_INT_ERR_UNDEFINED;

	/* Power down the DDR power domain if present */
	if (NULL != Device->Power) {
		Status = Device->Power->HandleEvent(&Device->Power->Node,
						    (u32)XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			DbgErr = XPM_INT_ERR_PWRDN;
			goto done;
		}
		Device->Node.Flags &= (u8)(~NODE_IDLE_DONE);
	}

done:
	XPm_PrintDbgErr(Status, DbgErr);
	return Status;
}

static const XPmFsm_StateCap XPmDDRDeviceStates[] = {
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

static const XPmFsm_Tran XPmDDRDevTransitions[] = {
	{
		.Event = (u32)XPM_DEVEVENT_BRINGUP_ALL,
		.FromState = (u8)XPM_DEVSTATE_UNUSED,
		.ToState = (u8)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
		.Action = ActionDDRBringUp,
	}, {
		.Event = (u32)XPM_DEVEVENT_BRINGUP_ALL,
		.FromState = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.ToState = (u8)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
		.Action = ActionDDRExitSelfRefresh,
	}, {
		.Event = (u32)XPM_DEVEVENT_RUNTIME_SUSPEND,
		.FromState = (u8)XPM_DEVSTATE_RUNNING,
		.ToState = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.Latency = XPM_DEF_LATENCY,
		.Action = ActionDDREnterSelfRefresh,
	}, {
		.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
		.FromState = (u8)XPM_DEVSTATE_RUNNING,
		.ToState = (u8)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
		.Action = ActionDDRShutdown,
	}, {
		.Event = (u32)XPM_DEVEVENT_SHUTDOWN,
		.FromState = (u8)XPM_DEVSTATE_RUNTIME_SUSPEND,
		.ToState = (u8)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
		.Action = ActionDDRShutdown,
	},
};

/**
 * @def DEFINE_DEV_STATES
 * @brief Helper macro to define device states array and count
 * @param S States array
 */
#define DEFINE_DEV_STATES(S)	.States = (S), \
				.StatesCnt = ARRAY_SIZE(S)

/**
 * @def DEFINE_DEV_TRANS
 * @brief Helper macro to define device transitions array and count
 * @param T Transitions array
 */
#define DEFINE_DEV_TRANS(T)	.Trans = (T), \
				.TransCnt = ARRAY_SIZE(T)

const XPm_Fsm XPmDDRDeviceFsm = {
	DEFINE_DEV_STATES(XPmDDRDeviceStates),
	DEFINE_DEV_TRANS(XPmDDRDevTransitions),
};