/******************************************************************************
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_aib.h"
#include "lpd_slcr.h"
#include "sleep.h"

struct Aib {
	u32 ReqRegAddr;
	u32 StsRegAddr;
	u32 Mask;
};

static struct Aib AibList[XPFW_AIB_ID_MAX] = {
	[XPFW_AIB_RPU0_TO_LPD] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_RPUM0_MASK,
	},
	[XPFW_AIB_RPU1_TO_LPD] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_RPUM1_MASK,
	},
	[XPFW_AIB_LPD_TO_DDR] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_LPD_DDR_MASK,
	},
	[XPFW_AIB_LPD_TO_FPD] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_FPD_MAIN_MASK,
	},
	[XPFW_AIB_LPD_TO_RPU0] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_RPUS0_MASK,
	},
	[XPFW_AIB_LPD_TO_RPU1] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_RPUS1_MASK,
	},
	[XPFW_AIB_LPD_TO_USB0] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_USB0S_MASK,
	},
	[XPFW_AIB_LPD_TO_USB1] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_USB1S_MASK,
	},
	[XPFW_AIB_LPD_TO_OCM] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_OCMS_MASK,
	},
	[XPFW_AIB_LPD_TO_AFI_FS2] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_AFIFS2_MASK,
	},
	[XPFW_AIB_FPD_TO_LPD_NON_OCM] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_FPD_LPDIBS_MASK,
	},
	[XPFW_AIB_FPD_TO_LPD_OCM] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_FPD_OCM_MASK,
	},
	[XPFW_AIB_FPD_TO_AFI_FS0] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_AFIFS0_MASK,
	},
	[XPFW_AIB_FPD_TO_AFI_FS1] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAXI_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAXI_REQ_AFIFS1_MASK,
	},
	[XPFW_AIB_FPD_TO_GPU] = {
		.ReqRegAddr = LPD_SLCR_ISO_AIBAPB_REQ,
		.StsRegAddr = LPD_SLCR_ISO_AIBAXI_ACK,
		.Mask = LPD_SLCR_ISO_AIBAPB_REQ_GPU_MASK,
	},
};

/**
 * Enable AIB isolation
 * @param AibId is ID of the AIB instance that needs to be enabled
 */
void XPfw_AibEnable(enum XPfwAib AibId)
{
	u32 Status;
	u32 TimeOutCount = AIB_ACK_TIMEOUT;

	if (AibId >= XPFW_AIB_ID_MAX) {
		XPfw_Printf(DEBUG_DETAILED, "Err: invalid AIB#%d\r\n", AibId);
		goto Done;
	}
	XPfw_RMW32(AibList[AibId].ReqRegAddr, AibList[AibId].Mask,
		   AibList[AibId].Mask);

	/* Loop until the AIB isolation ack is not received or we timeout */
	do {
		Status = XPfw_Read32(AibList[AibId].StsRegAddr) & AibList[AibId].Mask;
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
	if (AibId >= XPFW_AIB_ID_MAX) {
		XPfw_Printf(DEBUG_DETAILED, "Err: invalid AIB#%d\r\n", AibId);
		goto Done;
	}
	XPfw_RMW32(AibList[AibId].ReqRegAddr, AibList[AibId].Mask, 0U);
Done:
	return;
}