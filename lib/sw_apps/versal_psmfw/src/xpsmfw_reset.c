/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
/*****************************************************************************/
/**
*
* @file xpsmfw_reset.c
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who	Date		Changes
* ---- ---- -------- ------------------------------
* 1.00	rp	07/20/2018 	Initial release
*
* </pre>
*
* @note
*
******************************************************************************/
#include "xpsmfw_default.h"
#include "xpsmfw_reset.h"
#include "psm_global.h"
#include "pmc_global.h"
#include "crl.h"
#include "crf.h"

#define CHECK_BIT(reg, mask)	((reg & mask) == mask)

/**
 * XPsmFw_ResetLsR5() - Reset RPU
 *
 * @return    XST_SUCCESS or error code
 */
XStatus XPsmFw_ResetLsR5(void)
{
	/* Block R5 master interfaces using AIB */
	XPsmFw_RMW32(INTLPD_CONFIG_RPU0_LPD_AXI,
			INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEREQ_MASK,
			INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEREQ_MASK);
	XPsmFw_RMW32(INTLPD_CONFIG_RPU1_LPD_AXI,
			INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEREQ_MASK,
			INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEREQ_MASK);

	/* Wait for AIB ack. Timeout and continue if ack is not received */
	if (XST_SUCCESS != XPsmFw_UtilPollForMask(INTLPD_CONFIG_RPU0_LPD_AXI,
				INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEACK_MASK,
				RPU0_LPD_AXI_ISOLATION_TIMEOUT)) {
		xil_printf("Warning: XPsmFw_ResetLsR5: Failed to block r5_0 master interface\r\n");
	}
	if (XST_SUCCESS != XPsmFw_UtilPollForMask(INTLPD_CONFIG_RPU1_LPD_AXI,
				INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEACK_MASK,
				RPU1_LPD_AXI_ISOLATION_TIMEOUT)) {
		xil_printf("Warning: XPsmFw_ResetLsR5: Failed to block r5_1 master interface\r\n");
	}

	/* Block R5 slave interfaces using AIB */
	XPsmFw_RMW32(INTLPD_CONFIG_INTLPD_RPU0_AXI,
			INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEREQ_MASK,
			INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEREQ_MASK);
	XPsmFw_RMW32(INTLPD_CONFIG_INTLPD_RPU1_AXI,
			INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEREQ_MASK,
			INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEREQ_MASK);

	/* Wait for AIB ack. Timeout and continue if ack is not received */
	if (XST_SUCCESS != XPsmFw_UtilPollForMask(INTLPD_CONFIG_INTLPD_RPU0_AXI,
				INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEACK_MASK,
				LPD_RPU0_AXI_ISOLATION_TIMEOUT)) {
		xil_printf("Warning: XPsmFw_ResetLsR5: Failed to block r5_0 slave interface\r\n");
	}
	if (XST_SUCCESS != XPsmFw_UtilPollForMask(INTLPD_CONFIG_INTLPD_RPU1_AXI,
				INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEACK_MASK,
				LPD_RPU1_AXI_ISOLATION_TIMEOUT)) {
		xil_printf("Warning: XPsmFw_ResetLsR5: Failed to block r5_1 slave interface\r\n");
	}

	/* Unblock R5 master interfaces */
	XPsmFw_RMW32(INTLPD_CONFIG_RPU0_LPD_AXI,
			INTLPD_CONFIG_RPU0_LPD_AXI_POWER_IDLEREQ_MASK, 0);
	XPsmFw_RMW32(INTLPD_CONFIG_RPU1_LPD_AXI,
			INTLPD_CONFIG_RPU1_LPD_AXI_POWER_IDLEREQ_MASK, 0);

	/* Assert R5 reset by writing to PSM Global Reset register */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_PGE_MASK, CRL_RST_CPU_R5_RESET_PGE_MASK);
	XPsmFw_UtilWait(XPSMFW_RST_RPU_SEQ_PROP_TIME);

	/* De-assert R5 reset which will trigger R5 reboot */
	XPsmFw_RMW32(CRL_RST_CPU_R5, CRL_RST_CPU_R5_RESET_PGE_MASK, 0);
	XPsmFw_UtilWait(XPSMFW_RST_RPU_COMB_PROP_TIME);

	/* Unblock R5 slave interfaces */
	XPsmFw_RMW32(INTLPD_CONFIG_INTLPD_RPU0_AXI,
			INTLPD_CONFIG_INTLPD_RPU0_AXI_POWER_IDLEREQ_MASK, 0);
	XPsmFw_RMW32(INTLPD_CONFIG_INTLPD_RPU1_AXI,
			INTLPD_CONFIG_INTLPD_RPU1_AXI_POWER_IDLEREQ_MASK, 0);

	return XST_SUCCESS;
}

/**
 * XPsmFw_ResetFpd() - Reset FPD
 *
 * @return    XST_SUCCESS or error code
 */
XStatus XPsmFw_ResetFpd(void)
{
	/* Block FPD-LPD interfaces */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			(PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK | PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK),
			(PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK | PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK));

	/* Block FPD-PL interfaces */
	XPsmFw_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
			PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK,
			PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK);

	/* Block FPD-NoC interfaces */
	XPsmFw_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL,
			PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK,
			PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_SOC_MASK);

	/* Assert FPD reset by writing to PSM Global Reset register */
	XPsmFw_RMW32(CRL_RST_FPD, CRL_RST_FPD_SRST_MASK, CRL_RST_FPD_SRST_MASK);
	XPsmFw_UtilWait(XPSMFW_RST_FPD_SEQ_PROP_TIME);

	/* Unblock FPD-LPD interfaces */
	XPsmFw_RMW32(PSM_LOCAL_DOMAIN_ISO_CNTRL,
			(PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_DFX_MASK | PSM_LOCAL_DOMAIN_ISO_CNTRL_LPD_FPD_MASK), 0);

	/* Unblock FPD-PL interfaces */
	XPsmFw_RMW32(PMC_GLOBAL_DOMAIN_ISO_CNTRL, PMC_GLOBAL_DOMAIN_ISO_CNTRL_FPD_PL_MASK, 0);

	/* De-assert FPD reset (incl. CCI), which will enable LPD requests to go to FPD */
	XPsmFw_RMW32(CRL_RST_FPD, CRL_RST_FPD_SRST_MASK, 0);
	XPsmFw_UtilWait(XPSMFW_RST_FPD_COMB_PROP_TIME);

	/* De-assert APU L2/CPU resets, which will result in APU re-boot */
	XPsmFw_RMW32(CRF_RST_APU, (CRF_RST_APU_L2_RESET_MASK | CRF_RST_APU_ACPU0_MASK | CRF_RST_APU_ACPU1_MASK), 0);

	return XST_SUCCESS;
}

static struct SwResetHandlerTable_t ResetHandlerTable[] = {
	{PSM_GLOBAL_REG_REQ_SWRST_STATUS_FP_MASK, XPsmFw_ResetFpd},
	{PSM_GLOBAL_REG_REQ_SWRST_STATUS_LS_R5_MASK, XPsmFw_ResetLsR5},
};

/**
 * XPsmFw_DispatchSwRstHandler() - Software reset interrupt handler
 *
 * @SwRstStatus   Reset request status register value
 * @SwRstIntMask  Reset request interrupt mask register value
 *
 * @return         XST_SUCCESS or error code
 */
XStatus XPsmFw_DispatchSwRstHandler(u32 SwRstStatus, u32 SwRstIntMask)
{
	XStatus Status = XST_SUCCESS;
	u32 Idx;

	for (Idx = 0U; Idx < ARRAYSIZE(ResetHandlerTable); Idx++) {
		if ( (CHECK_BIT(SwRstStatus, ResetHandlerTable[Idx].Mask)) &
		     (!CHECK_BIT(SwRstIntMask, ResetHandlerTable[Idx].Mask)) ) {
			/* Call sw reset handler */
			Status = ResetHandlerTable[Idx].Handler();
		}

		/* Ack the service */
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_STATUS, ResetHandlerTable[Idx].Mask);
		XPsmFw_Write32(PSM_GLOBAL_REG_REQ_SWRST_INT_DIS, ResetHandlerTable[Idx].Mask);
	}

	return Status;
}
