/******************************************************************************
* Copyright (c) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


#include "xpfw_default.h"
#include "xpfw_aib.h"
#include "lpd_slcr.h"
#include "sleep.h"

static u32 AibMask[XPFW_AIB_ID_MAX] = {
	LPD_SLCR_ISO_AIBAXI_REQ_RPUM0_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_RPUM1_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_LPD_DDR_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_FPD_MAIN_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_RPUS0_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_RPUS1_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_USB0S_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_USB1S_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_OCMS_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_AFIFS2_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_FPD_LPDIBS_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_FPD_OCM_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_AFIFS0_MASK,
	LPD_SLCR_ISO_AIBAXI_REQ_AFIFS1_MASK,
	LPD_SLCR_ISO_AIBAPB_REQ_GPU_MASK
};

/**
 * Enable AIB isolation
 * @param AibId is ID of the AIB instance that needs to be enabled
 */
void XPfw_AibEnable(enum XPfwAib AibId)
{
	u32 Status;
	u32 TimeOutCount = AIB_ACK_TIMEOUT;
	u32 AibReqRegAddr;

	if (AibId >= XPFW_AIB_ID_MAX) {
		XPfw_Printf(DEBUG_DETAILED, "Err: invalid AIB#%d\r\n", AibId);
		goto Done;
	}

	if (AibId == XPFW_AIB_FPD_TO_GPU) {
		AibReqRegAddr = LPD_SLCR_ISO_AIBAPB_REQ;
	} else {
		AibReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ;
	}
	XPfw_RMW32(AibReqRegAddr, AibMask[AibId], AibMask[AibId]);

	/* Loop until the AIB isolation ack is not received or we timeout */
	do {
		Status = XPfw_Read32(LPD_SLCR_ISO_AIBAXI_ACK) & AibMask[AibId];
		if (Status != 0U) {
			break;
		}
		--TimeOutCount;
		usleep(10U);
	} while (TimeOutCount > 0U);

	if (Status == 0U) {
		XPfw_Printf(DEBUG_DETAILED, "No AIB#%d ack\r\n", AibId);
	}

Done:
	return;
}

/**
 * Disable AIB isolation
 * @param AibId is ID of the AIB instance that needs to be disabled
 */
void XPfw_AibDisable(enum XPfwAib AibId)
{
	u32 AibReqRegAddr;

	if (AibId >= XPFW_AIB_ID_MAX) {
		XPfw_Printf(DEBUG_DETAILED, "Err: invalid AIB#%d\r\n", AibId);
		goto Done;
	}
	if (AibId == XPFW_AIB_FPD_TO_GPU) {
		AibReqRegAddr = LPD_SLCR_ISO_AIBAPB_REQ;
	} else {
		AibReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ;
	}
	XPfw_RMW32(AibReqRegAddr, AibMask[AibId], 0U);
Done:
	return;
}
