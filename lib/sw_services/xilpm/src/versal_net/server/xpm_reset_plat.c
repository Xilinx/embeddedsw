/******************************************************************************
* Copyright (C) 2025, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_reset.h"

static XStatus AdmaResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	const XPm_Device *Device;

	Device = XPmDevice_GetById(PM_DEV_ADMA_0);
	if (NULL == Device) {
		goto done;
	}

#if (defined(XILPM_ZDMA_0) || defined(XILPM_ZDMA_1) || defined(XILPM_ZDMA_2) || \
	defined(XILPM_ZDMA_3) || defined(XILPM_ZDMA_4) || defined(XILPM_ZDMA_5) || \
	defined(XILPM_ZDMA_6) || defined(XILPM_ZDMA_7))
	(void)Rst;
	Status = NodeZdmaIdle(0U, Device->Node.BaseAddress);
#else
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;

	XPm_RMW32(ControlReg, Mask, Mask);

	Status = XST_SUCCESS;
#endif

done:
	return Status;
}

static XStatus AdmaResetPulse(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	const u32 Mask = BITNMASK(Rst->Shift, Rst->Width);
	const u32 ControlReg = Rst->Node.BaseAddress;

	Status = AdmaResetAssert(Rst);
	if (XST_SUCCESS != Status) {
		goto done;
	}

	XPm_RMW32(ControlReg, Mask, 0U);

done:
	return Status;
}

const void *GetResetCustomOps(u32 ResetId)
{
	u16 Idx;
	const struct ResetCustomOps *RstCustomStatus = NULL;
	static const struct ResetCustomOps Reset_Custom[] = {
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
	};

	for (Idx = 0U; Idx < ARRAY_SIZE(Reset_Custom); Idx++) {
		if (Reset_Custom[Idx].ResetIdx == NODEINDEX(ResetId)) {
			RstCustomStatus = &Reset_Custom[Idx];
			break;
		}
	}
	return RstCustomStatus;
}