/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserve.
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

/* Device security bit in register */
#define DEV_NONSECURE			(1U)
#define DEV_SECURE			(0U)

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

static XStatus SetSecurityAttr(XPm_Requirement *Reqm, u32 ReqCaps, u32 PrevState)
{
	XStatus Status = XST_FAILURE;
	u32 CurrSecState;
	u32 Is_CapSecure = 0U;
	u32 BaseAddr, Offset1, Mask1, Offset2, Mask2;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;
	const XPm_PsLpDomain *Lpd = (XPm_PsLpDomain *)XPmPower_GetById(PM_POWER_LPD);
	const XPm_PsFpDomain *Fpd = (XPm_PsFpDomain *)XPmPower_GetById(PM_POWER_FPD);

	/**
	 * Skip if device does not have any security attributes.
	 * Security[1] attribute is optional.
	 */
	if ((NULL == DevAttr) || (0U == DevAttr->Security[0].Mask) ||
	    (0U == DevAttr->SecurityBaseAddr)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((NULL == Lpd) || (NULL == Fpd)) {
		Status = XST_FAILURE;
		goto done;
	}

	BaseAddr = DevAttr->SecurityBaseAddr;
	Offset1 = DevAttr->Security[0].Offset;
	Mask1 = DevAttr->Security[0].Mask;
	Offset2 = DevAttr->Security[1].Offset;
	Mask2 = DevAttr->Security[1].Mask;

	if (0U != (ReqCaps & (u32)(PM_CAP_SECURE))) {
		Is_CapSecure = 1U;
	}

	/**
	 * Do not touch the LPD/FPD registers if domain is not ON.
	 * This is the generalized solution but applicable during the last
	 * device release of the domain.
	 */
	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Lpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		if ((u8)XPM_POWER_STATE_ON != Fpd->Domain.Power.Node.State) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Required due to MISRA */
	}

	/* Here 1 value in bit corresponds to non-secure config */
	CurrSecState = XPm_In32(BaseAddr + Offset1) & Mask1;

	/* Do nothing if device is still ON after release */
	if ((0U == Reqm->Allocated) &&
	    ((u8)XPM_DEVSTATE_RUNNING == Reqm->Device->Node.State)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if ((1U == Reqm->Allocated) &&
	    ((u32)XPM_DEVSTATE_RUNNING == PrevState)) {
		/**
		 * Return error if current device state does not match
		 * with the received request.
		 */
		if (((1U == Is_CapSecure) && (DEV_NONSECURE == CurrSecState)) ||
		    ((0U == Is_CapSecure) && (DEV_SECURE == CurrSecState))) {
			Status = XPM_PM_NO_ACCESS;
			goto done;
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x0U);
	} else {
		/* Required due to MISRA */
	}

	if (1U == Is_CapSecure) {
		PmRmw32(BaseAddr + Offset1, Mask1, 0);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, 0);
		}
		Reqm->AttrCaps |= (u8)(PM_CAP_SECURE);
	} else {
		PmRmw32(BaseAddr + Offset1, Mask1, Mask1);
		if (0U != Mask2) {
			PmRmw32(BaseAddr + Offset2, Mask2, Mask2);
		}
	}

	if (BaseAddr == Lpd->LpdSlcrSecureBaseAddr) {
		XPm_Out32(Lpd->LpdSlcrSecureBaseAddr + LPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else if (BaseAddr == Fpd->FpdSlcrSecureBaseAddr) {
		XPm_Out32(Fpd->FpdSlcrSecureBaseAddr + FPD_SLCR_SECURE_WPROT0_OFFSET, 0x1U);
	} else {
		/* Required due to MISRA */
	}
	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus SetDevCohVirtAttr(XPm_Requirement *Reqm, u32 ReqCaps,
				 u8 Capability, u32 Enable)
{
	XStatus Status = XST_FAILURE;
	u8 CurrCaps = 0U;
	u32 BaseAddr, Offset, Mask;
	XPm_Power *FpdPower = NULL;
	const XPm_Requirement *DevReqm = NULL;
	const XPm_DeviceAttr *DevAttr = Reqm->Device->DevAttr;

	/* Skip if device does not have any attributes */
	if (NULL == DevAttr) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Get base address, mask and offset based on capability */
	BaseAddr = DevAttr->CohVirtBaseAddr;
	if ((u8)(PM_CAP_COHERENT) == Capability) {
		Offset = DevAttr->Coherency.Offset;
		Mask = DevAttr->Coherency.Mask;
	} else {
		Offset = DevAttr->Virtualization.Offset;
		Mask = DevAttr->Virtualization.Mask;
	}

	/* Do nothing if required attributes are not present */
	if ((0U == Mask) || (0U == BaseAddr)) {
		Status = XST_SUCCESS;
                goto done;
	}

	if (1U == Enable) {
		/* Do nothing if capability is not requested */
		if (0U == (ReqCaps & Capability)) {
			Status = XST_SUCCESS;
			goto done;
		}
	} else {
		/* Update AttrCaps flag of requirement */
		if (0U != (Reqm->AttrCaps & Capability)) {
			Reqm->AttrCaps &= (u8)(~Capability);
		} else {
			Status = XST_SUCCESS;
			goto done;
		}
	}

	FpdPower = XPmPower_GetById(PM_POWER_FPD);
	if (NULL == FpdPower) {
		goto done;
	}

	DevReqm = Reqm->Device->Requirements;
	while (NULL != DevReqm) {
		if (1U == DevReqm->Allocated) {
			CurrCaps |= DevReqm->AttrCaps;
		}
		DevReqm = DevReqm->NextSubsystem;
	}
	/* Do nothing if attribute is already set */
	if (0U != (CurrCaps & Capability)) {
		Status = XST_SUCCESS;
		goto done;
	}

	if (1U == Enable) {
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_UP);
		if (XST_SUCCESS != Status) {
			goto done;
		}
		PmRmw32(BaseAddr + Offset, Mask, Mask);
		Reqm->AttrCaps |= Capability;
	} else {
		PmRmw32(BaseAddr + Offset, Mask, 0U);
		Status = FpdPower->HandleEvent(&FpdPower->Node,
						XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

done:
	return Status;
}

XStatus HandleDeviceAttr(struct XPm_Reqm *Reqm, u32 ReqCaps,
				u32 PrevState, u32 Enable)
{
	XStatus Status = XST_FAILURE;

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_COHERENT, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetDevCohVirtAttr(Reqm, ReqCaps, (u8)PM_CAP_VIRTUALIZED, Enable);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = SetSecurityAttr(Reqm, ReqCaps, PrevState);
	if (XST_SUCCESS != Status) {
		goto done;
	}

done:
	return Status;
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

/****************************************************************************/
/**
 * @brief  Add device security, virtualization and coherency attributes
 *
 * @param  Args		CDO command arguments
 * @param  NumArgs	Total number of arguments
 *
 * @return Status of the operation.
 *
 ****************************************************************************/
XStatus AddDevAttributes(const u32 *Args, const u32 NumArgs)
{
	XStatus Status = XST_FAILURE;
	XPm_DeviceAttr *DevAttr = NULL;
	XPm_Device *Dev = XPmDevice_GetById(Args[0]);

	/* Check for device presence and sufficient arguments */
	if ((NULL == Dev) || (NumArgs < 9U)) {
		Status = XST_INVALID_PARAM;
		goto done;
	}

	DevAttr = (XPm_DeviceAttr *)XPm_AllocBytes(sizeof(XPm_DeviceAttr));
	if (NULL == DevAttr) {
		Status = XST_BUFFER_TOO_SMALL;
		goto done;
	}

	/* Store the security attributes */
	DevAttr->SecurityBaseAddr = Args[6];
	DevAttr->Security[0].Offset = (u16)((Args[7] >> 16U) & 0xFFFFU);
	DevAttr->Security[0].Mask = (u16)(Args[7] & 0xFFFFU);
	DevAttr->Security[1].Offset = (u16)((Args[8] >> 16U) & 0xFFFFU);
	DevAttr->Security[1].Mask = (u16)(Args[8] & 0xFFFFU);

	/* Check for the coherency and virtualization attributes */
	if (NumArgs > 9U) {
		if (NumArgs < 12U) {
			Status = XST_INVALID_PARAM;
			goto done;
		}

		DevAttr->CohVirtBaseAddr = Args[9];
		DevAttr->Coherency.Offset = (u16)((Args[10] >> 16U) & 0xFFFFU);
		DevAttr->Coherency.Mask = (u16)(Args[10] & 0xFFFFU);
		DevAttr->Virtualization.Offset = (u16)((Args[11] >> 16U) & 0xFFFFU);
		DevAttr->Virtualization.Mask = (u16)(Args[11] & 0xFFFFU);
	}

	Dev->DevAttr = DevAttr;
	Status = XST_SUCCESS;

done:
	return Status;
}
