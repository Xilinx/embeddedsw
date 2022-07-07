/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
#include "xplmi_hw.h"
#include "xpm_psm.h"
#include "xil_types.h"
#include "xpm_common.h"
#include "xpm_regs.h"
#include "xplmi_generic.h"

#define GLOBAL_CNTRL(BASE)	((BASE) + PSMX_GLOBAL_CNTRL)
#define PROC_LOCATION_ADDRESS	(0xEBC26000U)
#define PROC_LOCATION_LENGTH	(0x2000U)

static struct PsmToPlmEvent_t PsmToPlmEvent_bkp = {0};
static u32 Is_PsmPoweredDown = 0U;

static XStatus XPmPsm_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	const XPm_Psm *Psm = (XPm_Psm *)Core;
	u32 CRLBaseAddress = Psm->CrlBaseAddr;
	const u32 CopySize = sizeof(PsmToPlmEvent_bkp);

	if (1U == Core->isCoreUp) {
		Status = XPM_ERR_WAKEUP;
		goto done;
	}

	/* Set reset address */
	if (1U == SetAddress) {
		if (0U != Address) {
			PmWarn("Handoff address is not used for PSM.\r\n");
		}
		Status = XST_INVALID_PARAM;
		goto done;
	}

	/* Assert wakeup bit to Wakeup PSM */
	PmRmw32(CRLBaseAddress + CRL_PSM_RST_MODE_OFFSET, XPM_PSM_WAKEUP_MASK, XPM_PSM_WAKEUP_MASK);

	/* Wait for PSMFW to initialize */
	Status = XPm_PollForMask(GLOBAL_CNTRL(Psm->PsmGlobalBaseAddr),
				 PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK,
				 XPM_MAX_POLL_TIMEOUT);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XPm_GetPsmToPlmEventAddr();
	if (XST_SUCCESS != Status) {
		goto done;
	}

	if (1U == Is_PsmPoweredDown) {
		/* Restore the context of reserved PSM RAM memory */
		Status = Xil_SMemCpy((void *)PsmToPlmEvent, CopySize, &PsmToPlmEvent_bkp, CopySize, CopySize);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	/* Check for the version of the PsmToPlmEvent structure */
	if (PsmToPlmEvent->Version != PSM_TO_PLM_EVENT_VERSION) {
		PmErr("PSM-PLM are out of sync. Can't process PSM event\n\r");
		goto done;
	} else {
		Status = XST_SUCCESS;
		Core->isCoreUp = 1;
	}

	/*
	 * PSM toggles between running and sleeping too frequently. Clear PSM
	 * wakeup bit to put it into sleep state while idle.
	 */
	PmRmw32(CRLBaseAddress + CRL_PSM_RST_MODE_OFFSET, XPM_PSM_WAKEUP_MASK, 0U);


	Status = XPlmi_SetProcList(PROC_LOCATION_ADDRESS, PROC_LOCATION_LENGTH);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	Status = XST_SUCCESS;

done:
	return Status;
}

static XStatus XPmPsm_PowerDown(XPm_Core *Core)
{
	XStatus Status = XST_FAILURE;
	XPm_Power *PwrNode;
	(void)Core;
	const u32 CopySize = sizeof(PsmToPlmEvent_bkp);

	if ((u8)XPM_DEVSTATE_UNUSED == Core->Device.Node.State) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Store the context of reserved PSM RAM memory */
	Status = Xil_SMemCpy(&PsmToPlmEvent_bkp, CopySize, (void *)PsmToPlmEvent, CopySize, CopySize);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	/* Add PSM specific power down sequence if any */

	if (NULL != Core->Device.Power) {
		PwrNode = Core->Device.Power;
		Status = PwrNode->HandleEvent(&PwrNode->Node, XPM_POWER_EVENT_PWR_DOWN);
		if (XST_SUCCESS != Status) {
			goto done;
		}
	}

	Is_PsmPoweredDown = 1U;

	Core->Device.Node.State = (u8)XPM_DEVSTATE_UNUSED;
	Core->isCoreUp = 0;
	Status = XST_SUCCESS;

done:
	return Status;
}

static struct XPm_CoreOps PsmOps = {
	.RequestWakeup = XPmPsm_WakeUp,
	.PowerDown = XPmPsm_PowerDown,
};

XStatus XPmPsm_SendPowerUpReq(XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	const XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		PmErr("PSMFW is not present\r\n");
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* Check if already powered up */
	PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	if (Power->PwrStatMask == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/**
	 * TODO: Uncomment below lines when request power up/down interrupt
	 * handling supported in PSMFW
	 */
	/*PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrUpEnOffset + REQ_PWRUP_INT_TRIG_OFFSET, Power->PwrUpMask);
	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrUpEnOffset, Power->PwrUpMask);
	do {
		PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	} while ((Reg & Power->PwrStatMask) != Power->PwrStatMask);*/

	Status = XST_SUCCESS;

done:
	return Status;
}

XStatus XPmPsm_SendPowerDownReq(XPm_Power *Power)
{
	XStatus Status = XST_FAILURE;
	u32 Reg;
	const XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	if (1U != XPmPsm_FwIsPresent()) {
		Status = XST_NOT_ENABLED;
		goto done;
	}

	/* Skip power down for power islands since power down not supported on SPP */
	if (XPLMI_PLATFORM == PMC_TAP_VERSION_SPP) {
		Status = XST_SUCCESS;
		goto done;
	}

	/* Check if already powered down */
	PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	if (0U == (Reg & Power->PwrStatMask)) {
		Status = XST_SUCCESS;
		goto done;
	}

	/**
	 * TODO: Uncomment below lines when request power up/down interrupt
	 * handling supported in PSMFW
	 */
	/*PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrDwnEnOffset + REQ_PWRDWN_INT_TRIG_OFFSET, Power->PwrDwnMask);
	PmOut32(Psm->PsmGlobalBaseAddr + Power->PwrDwnEnOffset, Power->PwrDwnMask);
	do {
		PmIn32(Psm->PsmGlobalBaseAddr + Power->PwrStatOffset, Reg);
	} while (0U != (Reg & Power->PwrStatMask));*/

	Status = XST_SUCCESS;

done:
	return Status;
}

u32 XPmPsm_FwIsPresent(void)
{
	u32 Reg = 0U;
	//changed to support minimum boot time xilpm
	PmIn32(GLOBAL_CNTRL(PSM_GLOBAL_REG_BASEADDR), Reg);//read PSM_GLOBAL_CNTRL data
	if (PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK ==
		(Reg & PSM_GLOBAL_REG_GLOBAL_CNTRL_FW_IS_PRESENT_MASK)) {
		Reg = 1U;
	}
	PmDbg("Reg %d\n",Reg);
	return Reg;
}

XStatus XPmPsm_Init(XPm_Psm *Psm,
	u32 Ipi,
	const u32 *BaseAddress,
	XPm_Power *Power, XPm_ClockNode *Clock, XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&Psm->Core, PM_DEV_PSM_PROC, Power, Clock, Reset,
			      (u8)Ipi, &PsmOps);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	Psm->PsmGlobalBaseAddr = BaseAddress[0];
	Psm->CrlBaseAddr = BaseAddress[1];
done:
	return Status;
}

void XPmPsm_RegWrite(const u32 Offset, const u32 Value)
{
	const XPm_Psm *Psm;

	Psm = (XPm_Psm *)XPmDevice_GetById(PM_DEV_PSM_PROC);
	if (NULL == Psm) {
		goto done;
	}

	PmOut32(Psm->PsmGlobalBaseAddr + Offset, Value);

done:
	return;
}
