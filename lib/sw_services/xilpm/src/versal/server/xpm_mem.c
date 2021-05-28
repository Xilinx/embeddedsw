/******************************************************************************
* Copyright (c) 2019 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_defs.h"
#include "xplmi_dma.h"
#include "xpm_regs.h"
#include "xpm_device.h"
#include "xpm_powerdomain.h"
#include "xpm_mem.h"
#include "xpm_rpucore.h"
#include "xpm_npdomain.h"
#include "xpm_debug.h"
#include "xpm_prot.h"

#define XPM_TCM_BASEADDRESS_MODE_OFFSET	0x80000U

#define XPM_NODEIDX_DEV_DDRMC_MIN	XPM_NODEIDX_DEV_DDRMC_0
#define XPM_NODEIDX_DEV_DDRMC_MAX	XPM_NODEIDX_DEV_DDRMC_3

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
 * @return XST_SUCCESS if DRAMs are in self-refresh mode or no DDRMC is
 *	   configured for the design, XST_FAILURE if DRAMs are not in
 *	   self-refresh mode
 *
 * @note   None
 *
 ****************************************************************************/
XStatus XPmDDRDevice_IsInSelfRefresh(void)
{
	XStatus Status = XST_SUCCESS;
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
		PmIn32((BaseAddress + NPI_PCSR_CONTROL_OFFSET), IsActive);
		if (DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK !=
		    (IsActive & DDRMC_UB_PCSR_CONTROL_PCOMPLETE_MASK)) {
			continue;
		}

		Reg = XPm_In32(BaseAddress + DDRMC_MAIN_UB_OFFSET + DDRMC_MAIN_REG_COM_4_OFFSET);
		if ((Reg & DDRMC_MAIN_DRAM_MODE_REPORT_MASK) != DDRMC_MAIN_SELF_REFRESH_MODE) {
			Status = XST_FAILURE;
			goto done;
		}
	}

done:
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
		Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
		XPm_Out32(Reg, NPI_PCSR_UNLOCK_VAL);

		/* Enable self-refresh */
		Reg = BaseAddress + DDRMC_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC_UB_PMC2UB_INTERRUPT_SPARE_0_MASK);
		Reg = BaseAddress + DDRMC_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_ACK_SPARE_0_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to enter self-refresh controller %x!\r\n",i);
			Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
			XPm_Out32(Reg, 0);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + DDRMC_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_DONE_SPARE_0_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to enter self-refresh controller %x!\r\n",i);
			Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
			XPm_Out32(Reg, 0);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
		XPm_Out32(Reg, 0);
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
		Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
		XPm_Out32(Reg, NPI_PCSR_UNLOCK_VAL);

		/* Disable self-refresh */
		Reg = BaseAddress + DDRMC_UB_PMC2UB_INTERRUPT_OFFSET;
		XPm_Out32(Reg, DDRMC_UB_PMC2UB_INTERRUPT_SR_EXIT_MASK);
		Reg = BaseAddress + DDRMC_UB_UB2PMC_ACK_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_ACK_SR_EXIT_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to exit self-refresh controller %x!\r\n",i);
			Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
			XPm_Out32(Reg, 0);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + DDRMC_UB_UB2PMC_DONE_OFFSET;
		Status = XPm_PollForMask(Reg, DDRMC_UB_UB2PMC_DONE_SR_EXIT_MASK,
					XPM_POLL_TIMEOUT);
		if (XST_SUCCESS != Status) {
			PmErr("Failed to exit self-refresh controller %x!\r\n",i);
			Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
			XPm_Out32(Reg, 0);
			goto done;
		}
		XPm_Out32(Reg, 0);

		Reg = BaseAddress + NPI_PCSR_LOCK_OFFSET;
		XPm_Out32(Reg, 0);
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
		} else {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
		} else {
			Status = XST_SUCCESS;
		}
		if ((u32)XPM_DEVSTATE_RUNTIME_SUSPEND == NextState) {
			Status = XPmDDRDevice_EnterSelfRefresh();
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

static const XPm_StateCap XPmMemDeviceStates[] = {
	{
		.State = (u8)XPM_DEVSTATE_UNUSED,
		.Cap = XPM_MIN_CAPABILITY,
	}, {
		.State = (u8)XPM_DEVSTATE_RUNNING,
		.Cap = PM_CAP_ACCESS | PM_CAP_CONTEXT,
	},
};

static const XPm_StateTran XPmMemDevTransitions[] = {
	{
		.FromState = (u32)XPM_DEVSTATE_RUNNING,
		.ToState = (u32)XPM_DEVSTATE_UNUSED,
		.Latency = XPM_DEF_LATENCY,
	}, {
		.FromState = (u32)XPM_DEVSTATE_UNUSED,
		.ToState = (u32)XPM_DEVSTATE_RUNNING,
		.Latency = XPM_DEF_LATENCY,
	},
};

static XStatus TcmProtControl(const XPm_Requirement *Reqm, u32 Enable)
{
	const XPm_MemDevice *Tcm = (XPm_MemDevice *)Reqm->Device;

	return XPmProt_PpuControl(Reqm, Tcm->StartAddress, Enable);
}

static void TcmEccInit(const XPm_MemDevice *Tcm, u32 Mode)
{
	u32 Size = Tcm->EndAddress - Tcm->StartAddress;
	u32 Id = Tcm->Device.Node.Id;
	u32 Base = Tcm->StartAddress;

	if ((PM_DEV_TCM_1_A == Id) || (PM_DEV_TCM_1_B == Id)) {
		if (XPM_RPU_MODE_LOCKSTEP == Mode) {
			Base -= XPM_TCM_BASEADDRESS_MODE_OFFSET;
		}
	}
	if (0U != Size) {
		s32 Status = XPlmi_EccInit(Base, Size);
		if (XST_SUCCESS != Status) {
			PmWarn("Error %d in EccInit of 0x%x\r\n", Status, Tcm->Device.Node.Id);
		}
	}
	return;
}

static XStatus HandleTcmDeviceState(XPm_Device* Device, u32 NextState)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Rpu0Device = XPmDevice_GetById(PM_DEV_RPU0_0);
	const XPm_Device *Rpu1Device = XPmDevice_GetById(PM_DEV_RPU0_1);
	u32 Id = Device->Node.Id;
	u32 Mode;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Request the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Request(Rpu0Device->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* TCM is only accessible when the RPU is powered on and out of reset and is in halted state
			 * so bring up RPU too when TCM is requested*/
			XPm_RpuGetOperMode(PM_DEV_RPU0_0, &Mode);
			if (XPM_RPU_MODE_SPLIT == Mode) {
				if (((PM_DEV_TCM_0_A == Id) ||
				     (PM_DEV_TCM_0_B == Id)) &&
				    ((u8)XPM_DEVSTATE_RUNNING !=
				     Rpu0Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu0Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
				if (((PM_DEV_TCM_1_A == Id) ||
				     (PM_DEV_TCM_1_B == Id)) &&
				    ((u8)XPM_DEVSTATE_RUNNING !=
				     Rpu1Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu1Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}
			if (XPM_RPU_MODE_LOCKSTEP == Mode)
			{
				if (((PM_DEV_TCM_0_A == Id) ||
				     (PM_DEV_TCM_0_B == Id) ||
				     (PM_DEV_TCM_1_A == Id) ||
				     (PM_DEV_TCM_1_B == Id)) &&
				     ((u8)XPM_DEVSTATE_RUNNING !=
				      Rpu0Device->Node.State)) {
					Status = XPmRpuCore_Halt(Rpu0Device);
					if (XST_SUCCESS != Status) {
						goto done;
					}
				}
			}
			/* Tcm should be ecc initialized */
			TcmEccInit((XPm_MemDevice *)Device, Mode);
		}
		Status = XST_SUCCESS;
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
			if (XST_SUCCESS != Status) {
				goto done;
			}

			/* Release the RPU clocks. Here both core having same RPU clock */
			Status = XPmClock_Release(Rpu0Device->ClkHandles);
			if (XST_SUCCESS != Status) {
				goto done;
			}
		}
		Status = XST_SUCCESS;
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

done:
	return Status;
}

static const XPm_DeviceFsm XPmTcmDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleTcmDeviceState,
};

static XStatus HandleMemDeviceState(XPm_Device* const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			Status = XPmDevice_BringUp(Device);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			Status = Device->HandleEvent(&Device->Node,
						     XPM_DEVEVENT_SHUTDOWN);
		} else {
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmMemDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleMemDeviceState,
};

static XStatus HandleMemRegnDeviceState(XPm_Device * const Device, const u32 NextState)
{
	XStatus Status = XST_FAILURE;

	/**
	 * FIXME:
	 *   1. This handler is a temporary solution to manage FSM
	 *      for memory region nodes. Since, memory nodes are abstract
	 *      devices, their parent dependency to physical memory device
	 *      nodes should either come from CDOs or internally be resolved
	 *      using comparisons from memory address ranges.
	 *   2. FSM to handle power up/down routines for these abstract
	 *      devices.
	 */
	switch (Device->Node.State) {
	case (u8)XPM_DEVSTATE_UNUSED:
		if ((u32)XPM_DEVSTATE_RUNNING == NextState) {
			/** HACK **/
			Device->Node.State = (u8)XPM_DEVSTATE_RUNNING;
			Status = XST_SUCCESS;
		}
		break;
	case (u8)XPM_DEVSTATE_RUNNING:
		if ((u32)XPM_DEVSTATE_UNUSED == NextState) {
			/** HACK */
			Device->Node.State = (u8)XPM_DEVSTATE_UNUSED;
			Status = XST_SUCCESS;
		}
		break;
	default:
		Status = XST_FAILURE;
		break;
	}

	return Status;
}

static const XPm_DeviceFsm XPmMemRegnDeviceFsm = {
	DEFINE_DEV_STATES(XPmMemDeviceStates),
	DEFINE_DEV_TRANS(XPmMemDevTransitions),
	.EnterState = HandleMemRegnDeviceState,
};

XStatus XPmMemDevice_Init(XPm_MemDevice *MemDevice,
		u32 Id,
		u32 BaseAddress,
		XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset,
		u32 MemStartAddress, u32 MemEndAddress)
{
	XStatus Status = XST_FAILURE;
	u32 Type = NODETYPE(Id);

	Status = XPmDevice_Init(&MemDevice->Device, Id, BaseAddress, Power, Clock,
				Reset);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	MemDevice->StartAddress = MemStartAddress;
	MemDevice->EndAddress = MemEndAddress;

	switch (Type) {
	case (u32)XPM_NODETYPE_DEV_DDR:
		MemDevice->Device.DeviceFsm = &XPmDDRDeviceFsm;
		break;
	case (u32)XPM_NODETYPE_DEV_TCM:
		MemDevice->Device.DeviceFsm = &XPmTcmDeviceFsm;
		/* TCM is protected by XPPU */
		MemDevice->Device.HandleProtection = &TcmProtControl;
		break;
	case (u32)XPM_NODETYPE_DEV_OCM_REGN:
	case (u32)XPM_NODETYPE_DEV_DDR_REGN:
		/**
		 * TODO: Device FSM handler for memory regions.
		 *
		 * Implement a device FSM handler for memory regions
		 * with device parent dependency resolved.
		 * For now, assume all the parents are on
		 * and will never be turned off.
		 */
		MemDevice->Device.DeviceFsm = &XPmMemRegnDeviceFsm;
		/* XMPU handler for memory regions */
		MemDevice->Device.HandleProtection = &XPmProt_MpuControl;
		break;
	default:
		MemDevice->Device.DeviceFsm = &XPmMemDeviceFsm;
		break;
	}

done:
	return Status;
}
