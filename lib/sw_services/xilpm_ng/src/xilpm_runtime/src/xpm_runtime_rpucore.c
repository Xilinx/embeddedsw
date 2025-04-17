/******************************************************************************
* Copyright (c) 2024 - 2025 Advanced Micro Devices, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_rpucore.h"
#include "xpm_runtime_rpucore.h"
#include "xil_sutil.h"

static XStatus SetBootAddr(const struct XPm_RpuCore *RpuCore, const u32 BootAddr)
{
	if (0U == BootAddr) {
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK,
			XPM_RPU_TCMBOOT_MASK);
	} else {
		PmOut32(RpuCore->RpuBaseAddr + XPM_CORE_VECTABLE_OFFSET, BootAddr);
		PmRmw32(RpuCore->RpuBaseAddr + XPM_CORE_CFG0_OFFSET, XPM_RPU_TCMBOOT_MASK, 0U);
	}

	return XST_SUCCESS;
}

XStatus XPm_RpuBootAddrConfig(const u32 DeviceId, const u32 BootAddr)
{
	volatile XStatus Status = XST_FAILURE;
	volatile XStatus StatusTmp = XST_FAILURE;
	const XPm_RpuCore *RpuCore = (XPm_RpuCore *)XPmDevice_GetById(DeviceId);
	if (NULL == RpuCore) {
		PmErr("Unable to get RPU Core for Id: 0x%x\n\r", DeviceId);
		goto done;
	}
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, SetBootAddr, RpuCore, BootAddr);
	if ((XST_SUCCESS != Status) || (XST_SUCCESS != StatusTmp)) {
		Status |= StatusTmp;
		PmErr("Error while setting boot address\n");
	}
done:
	return Status;
}
