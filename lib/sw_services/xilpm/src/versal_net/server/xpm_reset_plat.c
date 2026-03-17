/******************************************************************************
* Copyright (C) 2025 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

#include "xpm_common.h"
#include "xpm_device.h"
#include "xpm_device_idle.h"
#include "xpm_reset.h"
#include "xzdma_hw.h"

#define XPM_MAX_TIMEOUT			(0x1FFFFFFFU)
#define XILPM_ZDMA_CH_ISR_OFFSET	(0x100U)	/* Channel ISR offset */
#define XILPM_ZDMA_CH_IDS_OFFSET	(0x10CU)	/* Channel IDS offset */

static XStatus AdmaResetAssert(const XPm_ResetNode *Rst)
{
	XStatus Status = XST_FAILURE;
	u32 RegVal = 0U, LocalTimeout;
	u32 BaseAddress;

	BaseAddress = Rst->Node.BaseAddress;

	/* Disable/stop the Channel */
	XZDma_WriteReg(BaseAddress, (XZDMA_CH_CTRL2_OFFSET), (XZDMA_CH_CTRL2_DIS_MASK));

	/* Wait till transfers are not completed or halted */
	LocalTimeout = XPM_MAX_TIMEOUT;
	do {
		RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_STS_OFFSET) & XZDMA_STS_BUSY_MASK;
		LocalTimeout--;
	} while ((0U != RegVal) && (LocalTimeout > 0U));

	if (0U != RegVal) {
		goto done;
	}

	/* Disable and clear all interrupts */
	XZDma_WriteReg(BaseAddress, XILPM_ZDMA_CH_IDS_OFFSET, XZDMA_IXR_ALL_INTR_MASK);

	RegVal = XZDma_ReadReg(BaseAddress, XILPM_ZDMA_CH_ISR_OFFSET);
	XZDma_WriteReg(BaseAddress, XILPM_ZDMA_CH_ISR_OFFSET, (RegVal & XZDMA_IXR_ALL_INTR_MASK));

	/* Reset all the configurations */
	XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL0_OFFSET, XZDMA_CTRL0_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_CTRL1_OFFSET, XZDMA_CTRL1_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DATA_ATTR_OFFSET, XZDMA_DATA_ATTR_RESET_VALUE);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, XZDMA_DSCR_ATTR_RESET_VALUE);

	/* Clears total byte transferred */
	RegVal = XZDma_ReadReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_TOTAL_BYTE_OFFSET, RegVal);

	/* Read interrupt counts to clear it on both source and destination Channels*/
	(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_SRC_ACCT_OFFSET);
	(void)XZDma_ReadReg(BaseAddress, XZDMA_CH_IRQ_DST_ACCT_OFFSET);

	/* Reset the channel's coherent attributes. */
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DSCR_ATTR_OFFSET, 0x0);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_SRC_DSCR_WORD3_OFFSET, 0x0);
	XZDma_WriteReg(BaseAddress, XZDMA_CH_DST_DSCR_WORD3_OFFSET, 0x0);

	Status = XST_SUCCESS;
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
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_0,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_1,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_2,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_3,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_4,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_5,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_6,
			.ActionAssert = &AdmaResetAssert,
			.ActionPulse = &AdmaResetPulse,
		},
		{
			.ResetIdx = (u32)XPM_NODEIDX_RST_ADMA_7,
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
