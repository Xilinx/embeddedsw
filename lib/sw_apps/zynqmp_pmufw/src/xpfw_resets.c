/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
*
******************************************************************************/

#include "xpfw_default.h"
#include "xpfw_util.h"
#include "xpfw_resets.h"



/**
 * Delays or Wait times for HW related actions
 */
#define XPFW_TO_AIB_PS_PL	(0xFU)
#define XPFW_RST_PROP_DELAY (0xFU)

/**
 * Macros used for convenience
 * Only in this file
 */

#define AIB_CNTRL_MASK	(PMU_GLOBAL_AIB_CNTRL_LPD_AFI_FM_MASK |\
			PMU_GLOBAL_AIB_CNTRL_LPD_AFI_FS_MASK |\
			PMU_GLOBAL_AIB_CNTRL_FPD_AFI_FM_MASK |\
			PMU_GLOBAL_AIB_CNTRL_FPD_AFI_FS_MASK)

#define AIB_STATUS_MASK	(PMU_GLOBAL_AIB_STATUS_LPD_AFI_FM_MASK |\
			PMU_GLOBAL_AIB_STATUS_LPD_AFI_FS_MASK |\
			PMU_GLOBAL_AIB_STATUS_FPD_AFI_FM_MASK |\
			PMU_GLOBAL_AIB_STATUS_FPD_AFI_FS_MASK)

#define RPU_AIB_MASTER_MASK	(LPD_SLCR_ISO_AIBAXI_ACK_RPUM0_MASK |\
			LPD_SLCR_ISO_AIBAXI_ACK_RPUM1_MASK )
#define RPU_AIB_SLAVE_MASK	(LPD_SLCR_ISO_AIBAXI_ACK_RPUS0_MASK |\
			LPD_SLCR_ISO_AIBAXI_ACK_RPUS1_MASK )

/**
 * This PS only reset is to gracefully reset PS while PL remains active.
 * This reset can be triggered by hardware error signal(s) or
 * by software register write. This reset is a subset of
 * �System� reset (excluding PL reset). If this PS reset is triggered by
 * error signal, then error is transmitted to PL also.
 * The sequencing of this reset is described below:
*- [ErrorLogic]Error interrupt is asserted whose action requires PS-only reset
 *- This request is sent to PMU as interrupt
 *- [PMU-FW] Set PMU Error (=>PS-only reset) to indicate to PL.
 *- [PMU-FW] Block FPD=>PL and LPD=>PL interfaces with the help of AIB (in PS).
 *- If AIB ack is not received, then PMU should timeout and continue.
 *- [PMU-FW] (Optional) Block PL=>FPD (and PL=>LPD if any) interfaces with the
 *  help of AIB (in PL wrapper).
 *-If AIB ack is not received, then PMU should timeout and continue.
 *@note It requires PMU to PL-AIB req/ack interface.
 *@note PMU to PL-AIB req signals should not be reset by this PS-only reset.
 *- [PMU-FW] Initiate PS-only reset by writing to PMU-local register
 *- [RstCtrl] Assert PS-only reset
 *- [RstCtrl] After some wait, de-assert reset which will result in
 *  PS-only re-boot
 *- [FSBL] Unblock PL => FPD (and PL=>LPD if any) AXI interfaces
 * @note: You may never return from this function, since PS resets
 *
 */
void XPfw_ResetPsOnly(void)
{
	XStatus l_Status;

	/**
	 *TODO: Set PMU Error to indicate to PL
	 *
	 */

	fw_printf("Isolating Interfaces.....");
	/*
	 * Block FPD=>PL and LPD=>PL interfaces with the help of AIB (in PS)
	 */

	/**
	 * AIB CNTRL register has only these four interface bits
	 * And we will be asserting all of them
	 * So there is no need to do RMW
	 */

	Xil_Out32(PMU_GLOBAL_AIB_CNTRL,AIB_CNTRL_MASK);


	/**
	 * Wait till we get the AIB ack or Timeout
	 */
	l_Status = XPfw_UtilPollForMask(PMU_GLOBAL_AIB_STATUS,
			AIB_STATUS_MASK,XPFW_TO_AIB_PS_PL);

	if(l_Status == XST_SUCCESS){
		fw_printf("Done\r\n");
	}
	else
	{
		fw_printf("Time Out\r\n");
	}

	/**
	 *  Initiate PS-only reset by writing to PMU-local register
	 */
	fw_printf("Asserting Reset\r\n");
	XPfw_UtilRMW(PMU_GLOBAL_GLOBAL_RESET,
			PMU_GLOBAL_GLOBAL_RESET_PS_ONLY_RST_MASK,
			PMU_GLOBAL_GLOBAL_RESET_PS_ONLY_RST_MASK);
	/**
	 * Done
	 */

}



XStatus XPfw_ResetFpd(void)
{
	/*
	 * Block FPD=>LPD interfaces with the help of AIB (in FPS).
	 */
	Xil_Out32(PMU_LOCAL_DOMAIN_ISO_CNTRL,
			(PMU_LOCAL_DOMAIN_ISO_CNTRL_LP_FP_1_MASK |
					PMU_LOCAL_DOMAIN_ISO_CNTRL_FP_PL_MASK));

	/**
	 * FIXME: Update with correct delay here for AIB
	 */
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);
	/**
	 *  Initiate FPD reset by writing to PMU-local register
	 */
	XPfw_UtilRMW(PMU_GLOBAL_GLOBAL_RESET,
			PMU_GLOBAL_GLOBAL_RESET_FPD_RST_MASK,
			PMU_GLOBAL_GLOBAL_RESET_FPD_RST_MASK);
	/**
	 * Hold in Reset and wait till it propagates
	 */
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);
	/**
	 * Remove Isolation
	 */
	Xil_Out32(PMU_LOCAL_DOMAIN_ISO_CNTRL,0U);
	/**
	 * FIXME: Update with correct delay here
	 */
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);
	/**
	 * Release from Reset and wait till it propagates
	 */
	XPfw_UtilRMW(PMU_GLOBAL_GLOBAL_RESET,
				PMU_GLOBAL_GLOBAL_RESET_FPD_RST_MASK,
				0);
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);

	/**
	 * FIXME: Is there something that we can poll to
	 * check if we failed somewhere?
	 */
	return XST_SUCCESS;

}


XStatus XPfw_ResetRpu(void)
{

	XStatus l_Status;

	/**
	 * Reference: ZynqMP Arch Spec 2.02
	 *
	 */

	/**
	 * - Block R5 Master Interfaces using AIB
	 */
	XPfw_UtilRMW(LPD_SLCR_ISO_AIBAXI_REQ, RPU_AIB_MASTER_MASK,
			RPU_AIB_MASTER_MASK);

	/**
	 * - Wait for AIB ack, TimeOut If not received
	 */
	fw_printf("Waiting for AIB Ack (M).....");
	l_Status = XPfw_UtilPollForMask(LPD_SLCR_ISO_AIBAXI_ACK,
					RPU_AIB_MASTER_MASK,
					XPFW_TO_AIB_PS_PL);

	if (l_Status == XST_SUCCESS) {
		fw_printf("Done\r\n");
	} else {
		fw_printf("Time Out\r\n");
	}

	/**
	 * - Block R5 Slave Interfaces using AIB
	 *
	 */
	XPfw_UtilRMW(LPD_SLCR_ISO_AIBAXI_REQ,
			RPU_AIB_SLAVE_MASK, RPU_AIB_SLAVE_MASK);
	/**
	 * - Wait for AIB ack, Timeout if not received
	 */
	fw_printf("Waiting for AIB Ack (S).....");
	l_Status = XPfw_UtilPollForMask(LPD_SLCR_ISO_AIBAXI_ACK, RPU_AIB_SLAVE_MASK,
					XPFW_TO_AIB_PS_PL);
	if (l_Status == XST_SUCCESS) {
		fw_printf("Done\r\n");
	} else {
		fw_printf("Time Out\r\n");
	}

	/**
	 * Unblock Master Interfaces
	 */
	XPfw_UtilRMW(LPD_SLCR_ISO_AIBAXI_REQ,
				RPU_AIB_MASTER_MASK, 0U);

	/**
	 *  Initiate R5 LockStep reset by writing to PMU-local register
	 */
	XPfw_UtilRMW(PMU_GLOBAL_GLOBAL_RESET,
			PMU_GLOBAL_GLOBAL_RESET_RPU_LS_RST_MASK,
			PMU_GLOBAL_GLOBAL_RESET_RPU_LS_RST_MASK);
	/**
	 * FIXME: Update with correct delay here
	 */
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);

	fw_printf("Releasing RPU out of Reset\r\n");

	/**
	 * Release from Reset and wait till it propagates
	 */
	XPfw_UtilRMW(PMU_GLOBAL_GLOBAL_RESET,
			PMU_GLOBAL_GLOBAL_RESET_RPU_LS_RST_MASK, 0U);
	XPfw_UtilWait(XPFW_RST_PROP_DELAY);

	/**
	 * Unblock SLave Interfaces
	 */
	XPfw_UtilRMW(LPD_SLCR_ISO_AIBAXI_REQ,
			RPU_AIB_SLAVE_MASK, 0U);
	return XST_SUCCESS;
}
