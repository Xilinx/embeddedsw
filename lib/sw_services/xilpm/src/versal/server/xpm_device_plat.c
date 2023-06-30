/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi_dma.h"
#include "xplmi.h"
#include "xpm_device.h"
#include "xpm_core.h"
#include "xpm_regs.h"
#include "xpm_rpucore.h"
#include "xpm_notifier.h"
#include "xpm_api.h"
#include "xpm_pmc.h"
#include "xpm_mem.h"
#include "xpm_pslpdomain.h"
#include "xpm_psfpdomain.h"
#include "xpm_requirement.h"
#include "xpm_debug.h"
#include "xpm_pldevice.h"
#include "xpm_aiedevice.h"

#define SD_DLL_DIV_MAP_RESET_VAL	(0x50505050U)

struct XPm_Reqm *XPmDevice_GetAieReqm(XPm_Device *Device, XPm_Subsystem *Subsystem)
{
	XPm_Requirement *Reqm = NULL;

	if (IS_DEV_AIE(Device->Node.Id)) {
		(void)XPmRequirement_Add(Subsystem, Device,
					    (u32)REQUIREMENT_FLAGS(0U,
					    (u32)REQ_ACCESS_SECURE_NONSECURE,
					    (u32)REQ_TIME_SHARED),
					    0U, XPM_DEF_QOS);

		/* Get requirement */
		Reqm = XPmDevice_FindRequirement(Device->Node.Id, Subsystem->Id);
	}

	return Reqm;
}

void PlatDevRequest(const XPm_Device *Device, const XPm_Subsystem *Subsystem, const u32 QoS, XStatus *Status)
{
	if (IS_DEV_AIE(Device->Node.Id)) {
		*Status = XPmAieDevice_UpdateClockDiv(Device, Subsystem, QoS);
	}
}

XStatus XPmDevice_ConfigureADMA(const u32 Id)
{
	XStatus Status = XST_FAILURE;

	const XPm_PsLpDomain *PsLpd;
	PsLpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	if (NULL == PsLpd) {
		Status = XST_FAILURE;
		goto done;
	}

	/*
	 * Configure ADMA as non-secure so Linux
	 * can use it.
	 * TODO: Remove this when security config
	 * support is added through CDO
	 */
	if ((PM_DEV_ADMA_0 <= Id) && (PM_DEV_ADMA_7 >= Id)) {
		XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
			  LPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
		XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
			  LPD_SLCR_SECURE_ADMA_0_OFFSET +
			  ((Id - PM_DEV_ADMA_0) * 4U), 0x1U);
		XPm_Out32(PsLpd->LpdSlcrSecureBaseAddr +
			  LPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

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
	} else if (PM_DEV_SDIO_1 == Device->Node.Id) {
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP0_OFFSET,
			Value);
		PmIn32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
		       Value);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
			SD_DLL_DIV_MAP_RESET_VAL);
		PmOut32(BaseAddress + PMC_IOU_SLCR_SD1_DLL_DIV_MAP1_OFFSET,
			Value);
	} else {
		/* Required by MISRA */
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmDevice_SdResetWorkaround(const XPm_Device *Device)
{
	XStatus Status = XST_FAILURE;
	/*
	 * As per EDT-997700 SD/eMMC DLL modes are failing after
	 * SD controller reset. Reset SD_DLL_MAP registers after
	 * reset release as a workaround.
	 */
	if ((PM_DEV_SDIO_0 == Device->Node.Id) || (PM_DEV_SDIO_1 == Device->Node.Id)) {
		Status = ResetSdDllRegs(Device);
	} else {
		Status = XST_SUCCESS;
	}

	return Status;
}

XStatus XPmDevice_PlatAddParent(const u32 Id, const u32 ParentId)
{
	XStatus Status = XST_FAILURE;
	XPm_Device *DevPtr = XPmDevice_GetById(Id);

	if (((u32)XPM_NODECLASS_DEVICE == NODECLASS(Id)) &&
		((u32)XPM_NODESUBCL_DEV_AIE == NODESUBCLASS(Id))) {
		XPm_AieDevice *AieDevice = (XPm_AieDevice *)DevPtr;
		XPm_PlDevice *Parent = (XPm_PlDevice *)XPmDevice_GetById(ParentId);
		/*
		 * Along with checking validity of parent, check if parent has
		 * a parent with exception being PLD_0. This is to prevent
		 * broken trees
		 */
		Status = XPmPlDevice_IsValidPld(Parent);
		if (XST_SUCCESS != Status) {
			goto done;
		}

		AieDevice->Parent = Parent;
		Parent->AieDevice = AieDevice;
		Status = XST_SUCCESS;
	} else {
		Status = XPM_INVALID_DEVICEID;
	}

done:
	return Status;
}
