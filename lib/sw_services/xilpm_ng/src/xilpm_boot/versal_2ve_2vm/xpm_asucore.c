/******************************************************************************
* Copyright (C) 2024-2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xplmi.h"
#include "xil_io.h"
#include "xpm_asucore.h"
#include "xpm_api.h"
#include "xpm_regs.h"

XStatus XPmAsuCore_WakeUp(XPm_Core *Core, u32 SetAddress, u64 Address)
{
	XStatus Status = XST_FAILURE;
	/* Handoff address is not able to adjust; hence ignore them */
	(void)SetAddress;
	(void)Address;

	/* ASU is special, we don't need to do direct power on. Because it is never power off until POR */
	if (TRUE == Core->isCoreUp) {
		/* This should never happen but just in case */
		PmWarn("ASU core is already up.\r\n");
		Status = XST_SUCCESS;
		goto done;
	}

	/* Set Wake up on reset for ASU processor */
	XPm_Out32(PSX_CRL_BASEADDR + CRL_ASU_MB_RST_MODE_OFFSET,CRL_ASU_MB_RST_WAKEUP_MASK);
	/* Bring ASU out of reset by toggling the soft rst */
	XPm_Out32(ASU_GLOBAL_ASU_MB_SOFT_RST, ASU_GLOBAL_ASU_MB_SOFT_RST_VAL_MASK);
	/*.. deassert the reset bit*/
	XPm_Out32(ASU_GLOBAL_ASU_MB_SOFT_RST, XPM_ASU_SOFT_RST_DEASSERT);

	Core->isCoreUp = TRUE;
	Status = XST_SUCCESS;
done:
	PmInfo("ASU Core Wakeup Status: 0x%x\r\n", Status);
	return Status;
}

XStatus XPmAsuCore_PwrDwn(XPm_Core *Core)
{
	/* ASU cores don't have a power down implementation */
	(void)Core;
	return XST_SUCCESS;
}

XStatus XPmAsuCore_Init(struct XPm_AsuCore *AsuCore, u32 Id, u32 Ipi, const u32 *BaseAddress,
			XPm_Power *Power, XPm_ClockNode *Clock,
			XPm_ResetNode *Reset)
{
	XStatus Status = XST_FAILURE;

	Status = XPmCore_Init(&AsuCore->Core, Id, Power, Clock, Reset, (u8)Ipi);
	if (XST_SUCCESS != Status) {
		PmErr("Status: 0x%x\r\n", Status);
		goto done;
	}

	AsuCore->AsuBaseAddr = BaseAddress[XPM_ASU_BASEADDR_IDX];

done:
	return Status;
}
