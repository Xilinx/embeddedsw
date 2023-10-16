/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xpm_requirement.h"
#include "xpm_device.h"
#include "xpm_pmc.h"

#define SD_DLL_DIV_MAP_RESET_VAL	(0x50505050U)

static XStatus ResetSdDllRegs(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	u32 Value;
	u32 BaseAddress;
	const XPm_Pmc *Pmc = (XPm_Pmc *)XPmDevice_GetById(PM_DEV_PMC_PROC);
	if (NULL == Pmc) {
		Status = XPM_INVALID_DEVICEID;
		goto done;
	}

	BaseAddress = Pmc->PmcIouSlcrBaseAddr;
	if (PM_DEV_SDIO_0 == Device->Node.Id) {
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP0_OFFSET,
			Value);
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD0_DLL_DIV_MAP1_OFFSET,
			Value);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmDevice_SdResetWorkaround(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	/*
	 * As per EDT-1054057 SD/eMMC DLL modes are failing after
	 * SD controller reset. Reset SD_DLL_MAP registers after
	 * reset release as a workaround.
	 */
	/* SDIO1 is EMMC and there is no DLL_RESET for SDIO1 */
	if ((PM_DEV_SDIO_0 == Device->Node.Id)) {
		Status = ResetSdDllRegs(Device);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}
