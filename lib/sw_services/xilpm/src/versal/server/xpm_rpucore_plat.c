/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_rpucore.h"
#include "xpm_regs.h"

void XPmRpuCore_AssignRegAddr(struct XPm_RpuCore *RpuCore, const u32 Id, const u32 *BaseAddress)
{
	RpuCore->RpuBaseAddr = BaseAddress[0];
	if (PM_DEV_RPU0_0 == Id) {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_0_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_0_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_0_CPUPWRDWNREQ_MASK;
	} else {
		RpuCore->ResumeCfg = RpuCore->RpuBaseAddr + RPU_1_CFG_OFFSET;
		RpuCore->Core.SleepMask = XPM_RPU0_1_PWR_CTRL_MASK;
		RpuCore->Core.PwrDwnMask = XPM_RPU_1_CPUPWRDWNREQ_MASK;
	}
}

XStatus XPm_RpuTcmCombConfig(const u32 DeviceId, const u32 Config)
{
	XStatus Status = XST_FAILURE;
	u32 Address;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	Address = RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET;
	if (Config == XPM_RPU_TCM_SPLIT) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			~XPM_RPU_TCM_COMB_MASK);
		Status = XST_SUCCESS;
	} else if (Config == XPM_RPU_TCM_COMB) {
		PmRmw32(Address, XPM_RPU_TCM_COMB_MASK,
			XPM_RPU_TCM_COMB_MASK);
		Status = XST_SUCCESS;
	} else {
		Status = XST_INVALID_PARAM;
	}

	return Status;
}

XStatus XPm_RpuRstComparators(const u32 DeviceId)
{
	XStatus Status = XST_FAILURE;
	const XPm_RpuCore *RpuCore = NULL;

	RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);

	if(RpuCore == NULL) {
		PmInfo("Invalid Device Id: 0x%x\n\r", DeviceId);
		goto done;
	}

	PmOut32(RpuCore->RpuBaseAddr + RPU_ERR_INJ_OFFSET, 0x0);
	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPm_PlatRpuSetOperMode(const XPm_RpuCore *RpuCore, const u32 Mode, u32 *Val)
{
	if (Mode == XPM_RPU_MODE_SPLIT) {
		*Val |= XPM_RPU_SLSPLIT_MASK;
		*Val &= ~XPM_RPU_TCM_COMB_MASK;
		*Val &= ~XPM_RPU_SLCLAMP_MASK;
	} else if (Mode == XPM_RPU_MODE_LOCKSTEP) {
		*Val &= ~XPM_RPU_SLSPLIT_MASK;
		*Val |= XPM_RPU_TCM_COMB_MASK;
		*Val |= XPM_RPU_SLCLAMP_MASK;
	} else {
		/* Required by MISRA */
	}
	PmOut32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, *Val);

	return XST_SUCCESS;
}

XStatus XPm_PlatRpuBootAddrConfig(const XPm_RpuCore *RpuCore, const u32 BootAddr)
{
	XStatus Status = XST_FAILURE;

	/* CFG_VINITHI_MASK mask is common for both processors */
	if (XPM_RPU_BOOTMEM_LOVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
				~XPM_RPU_VINITHI_MASK);
		Status = XST_SUCCESS;
	} else if (XPM_RPU_BOOTMEM_HIVEC == BootAddr) {
		PmRmw32(RpuCore->ResumeCfg, XPM_RPU_VINITHI_MASK,
				XPM_RPU_VINITHI_MASK);
		Status = XST_SUCCESS;
	} else {
		/* Required for MISRA */
	}

	return Status;
}

u32 XPm_PlatRpuGetOperMode(const XPm_RpuCore *RpuCore)
{
	u32 Val;

	PmIn32(RpuCore->RpuBaseAddr + RPU_GLBL_CNTL_OFFSET, Val);

	return Val;
}

void XPm_GetCoreId(u32 *Rpu0, u32 *Rpu1, const u32 DeviceId)
{
	(void)DeviceId;

	*Rpu0 = PM_DEV_RPU0_0;
	*Rpu1 = PM_DEV_RPU0_1;
}

XStatus XPm_PlatRpucoreHalt(XPm_Core *Core){
	return XPmRpuCore_Halt((XPm_Device *)Core);
}
