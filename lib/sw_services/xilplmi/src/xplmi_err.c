/******************************************************************************
* Copyright (C) 2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplmi_err.c
*
* This file contains error management for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ====  ==== ======== ======================================================-
* 1.00  kc   02/12/2019 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_err.h"
#include "xplmi.h"
#include "xpm_node.h"
#include "xplmi_sysmon.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define	XPLMI_ERR_REG_MASK(ErrorId)	(0x1U << (NODEINDEX(ErrorId)%32))
/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * This function is called in PLM error cases.
 *
 * @param ErrorStatus is the error code which is written to the
 *		  error status register
 *
 * @return none
 *
 *****************************************************************************/
void XPlmi_ErrMgr(int Status)
{
	/* Print the PMCFW error */
	XPlmi_Printf(DEBUG_GENERAL, "PLM Error Status: 0x%08lx\n\r",
			Status);
	XPlmi_Out32(PMC_GLOBAL_PMC_FW_ERR, Status);

	/**
	 * Fallback if boot PDI is not done
	 * else just return, so that we receive next requests
	 */
	if (XPlmi_IsLoadBootPdiDone() == FALSE)
	{
		XPlmi_DumpRegisters();
		/**
		 * TODO
		 * Add fallback code here.
		 */
		while(1);
	}
}

/*
 * Structure to define error action type and handler if action type
 * is XPLMI_EM_ACTION_CUSTOM for each error.
 */
struct XPlmi_Error_t ErrorTable[] = {
	[XPM_NODEIDX_ERROR_BOOT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_BOOT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, },
	[XPM_NODEIDX_ERROR_FW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, },
	[XPM_NODEIDX_ERROR_GSW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_GSW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CFU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CFRAME] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMC_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMC_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_DDRMB_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_DDRMB_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_NOCTYPE1_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_NOCTYPE1_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_NOCUSER] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_MMCM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_ME_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_ME_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_DDRMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_DDRMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_GT_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_GT_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PLSMON_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PLSMON_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PL2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PL3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_NPIROOT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCAPB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCROM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, },
	[XPM_NODEIDX_ERROR_MB_FATAL0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, },
	[XPM_NODEIDX_ERROR_MB_FATAL1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, },
	[XPM_NODEIDX_ERROR_PMCPAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMC_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMC_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_ERROUT, },
	[XPM_NODEIDX_ERROR_PMCSMON0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON6] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON7] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCSMON8] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, },
	[XPM_NODEIDX_ERROR_PMCSMON9] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CFI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SEUCRC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SEUECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_RTCALARM] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, },
	[XPM_NODEIDX_ERROR_NPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PPLL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CLKMON] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCTO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCXMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PMCXPPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_SSIT2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PS_SW_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PS_SW_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PSM_B_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PSM_B_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_MB_FATAL] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_SRST, },
	[XPM_NODEIDX_ERROR_PSM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PSM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_OCM_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_L2_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_RPU_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_RPU_LS] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_RPU_CCF] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_GIC_AXI] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_GIC_ECC] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_APLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_RPLL_LOCK] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CPM_CR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_CPM_NCR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_APB] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_IOU_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PSM_PAR] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_PSM_TO] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SWDT] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON6] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_SMON7] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON0] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON1] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON2] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON3] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON4] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON5] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON6] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_SMON7] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_XMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_LPD_XPPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
	[XPM_NODEIDX_ERROR_FPD_XMPU] =
	{ .Handler = NULL, .Action = XPLMI_EM_ACTION_NONE, },
};

/*****************************************************************************/
/**
 * @brief This function disables the responses for the given error ID.
 * @param ErrorId is the error identifier
 * @return Success / failure
 *****************************************************************************/
int XPlmi_EmDisable(u32 ErrorId)
{
	int Status;
	u32 RegMask;

	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		/* Invalid Error ID */
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPLMI_ERR_REG_MASK(ErrorId);

	switch (NODETYPE(ErrorId)) {
	case XPM_NODETYPE_EVENT_PMC_ERR1:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPlmi_Out32(PMC_GLOBAL_PMC_POR1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PMC_ERR2:
		/* Disable POR, SRST, Interrupt and PS Error Out */
		XPlmi_Out32(PMC_GLOBAL_PMC_POR2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ2_DIS, RegMask);
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST2_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR1:
		/* Disable CR / NCR to PMC,  SRST, Interrupt */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_SRST1_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR2:
		/* Disable CR / NCR to PMC,  SRST, Interrupt */
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR2_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR2_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ2_DIS, RegMask);
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_SRST2_DIS, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Error Type */
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function enables the POR response for the given Error ID.
 * @param ErrorId is the error identifier
 * @return Success / failure
 *****************************************************************************/
static int XPlmi_EmEnablePOR(u32 ErrorId)
{
	int Status;
	u32 RegMask;

	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPLMI_ERR_REG_MASK(ErrorId);

	switch (NODETYPE(ErrorId)) {
	case XPM_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_POR1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_POR2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function enables the SRST response for the given Error ID.
 * @param ErrorId is the error identifier
 * @return Success / failure
 *****************************************************************************/
static int XPlmi_EmEnableSRST(u32 ErrorId)
{
	int Status;
	u32 RegMask;

	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPLMI_ERR_REG_MASK(ErrorId);

	switch (NODETYPE(ErrorId)) {
	case XPM_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_SRST2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_SRST1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_SRST2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function enables the ERR OUT response for the given Error ID.
 * @param ErrorId is the error identifier
 * @return Success / failure
 *****************************************************************************/
static int XPlmi_EmEnablePSError(u32 ErrorId)
{
	int Status;
	u32 RegMask;

	/* If Error ID is not in range, fail */
	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPLMI_ERR_REG_MASK(ErrorId);

	/* Enable the specified Error to propagate to ERROUT pin	*/
	switch (NODETYPE(ErrorId)) {
	case XPM_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function enables the interrupt to PMC for the given Error ID.
 * @param ErrorId is the error identifier
 * @return Success / failure
 *****************************************************************************/
static int XPlmi_EmEnableInt(u32 ErrorId)
{
	int Status;
	u32 RegMask;

	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		/* Invalid Error Id */
		Status = XPLMI_INVALID_ERROR_ID;
		goto END;
	}

	RegMask = XPLMI_ERR_REG_MASK(ErrorId);

	switch (NODETYPE(ErrorId)) {
	case XPM_NODETYPE_EVENT_PMC_ERR1:
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PMC_ERR2:
		XPlmi_Out32(PMC_GLOBAL_PMC_IRQ2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR1:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	case XPM_NODETYPE_EVENT_PSM_ERR2:
		XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ2_EN, RegMask);
		Status = XST_SUCCESS;
		break;
	default:
		/* Invalid Err Type */
		Status = XPLMI_INVALID_ERROR_TYPE;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid ErrType for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function sets the Action specified for a given Error ID.
 * @param ErrorId is the error identifier
 * @param ActionId is the action that need to be set for ErrorID. Action
 * can be SRST/POR/ERR OUT/INT
 * @param ErrorHandler If INT is defined as response, handler should be
 * defined.
 * @return Success / failure
 *****************************************************************************/
int XPlmi_EmSetAction(u32 ErrorId, u8 ActionId,
		XPlmi_ErrorHandler_t ErrorHandler)
{
	int Status;

	/* Check for Valid Error ID */
	if (NODEINDEX(ErrorId) >= XPM_NODEIDX_ERROR_PSMERR2_MAX) {
		/* Invalid Error Id */
		Status = XPLMI_INVALID_ERROR_ID;
		XPlmi_Printf(DEBUG_GENERAL,
			"Invalid Error Id:0x%0x\n\r", ErrorId);
		goto END;
	}

	if((XPLMI_EM_ACTION_CUSTOM == ActionId) && (NULL == ErrorHandler)) {
		/* Null handler */
		Status = XPLMI_INVALID_ERROR_HANDLER;
		XPlmi_Printf(DEBUG_GENERAL, "Invalid Error Handler \n\r");
		goto END;
	}

	if((ActionId > XPLMI_EM_ACTION_NONE) && (ActionId < XPLMI_EM_ACTION_MAX)) {
		/* Disable the error actions for Error ID for configuring
		 * the requested error action */
		Status = XPlmi_EmDisable(ErrorId);
		if (XST_SUCCESS != Status) {
			/* Error action disabling failure */
			goto END;
		}
	}

	switch (ActionId) {

	case XPLMI_EM_ACTION_NONE:
		/* No Action */
		ErrorTable[NODEINDEX(ErrorId)].Action = ActionId;
		Status = XST_SUCCESS;
		break;

	case XPLMI_EM_ACTION_POR:
		/* Set the error action and enable it */
		ErrorTable[NODEINDEX(ErrorId)].Action = ActionId;
		Status = XPlmi_EmEnablePOR(ErrorId);
		break;

	case XPLMI_EM_ACTION_SRST:
		/* Set error action SRST for the errorId */
		ErrorTable[NODEINDEX(ErrorId)].Action = ActionId;
		Status = XPlmi_EmEnableSRST(ErrorId);
		break;

	case XPLMI_EM_ACTION_CUSTOM:
		/* Set custom handler as error action for the errorId */
		ErrorTable[NODEINDEX(ErrorId)].Action = ActionId;
		ErrorTable[NODEINDEX(ErrorId)].Handler = ErrorHandler;
		Status = XPlmi_EmEnableInt(ErrorId);
		break;

	case XPLMI_EM_ACTION_ERROUT:
		ErrorTable[NODEINDEX(ErrorId)].Action = ActionId;
		/* Set error action ERROUT signal for the errorId */
		Status = XPlmi_EmEnablePSError(ErrorId);
		break;

	default:
		/* Invalid Action Id */
		Status = XPLMI_INVALID_ERROR_ACTION;
		XPlmi_Printf(DEBUG_GENERAL,
		"Invalid ActionId for ErrId: 0x%0x\n\r", ErrorId);
		break;
	}
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function initializes the error module. Disables all the
 * error actions and registers default action
 * @param None
 * @return None
 *****************************************************************************/
void XPlmi_EmInit(void)
{
	u32 Index;

	/* Register Error module commands */
	XPlmi_ErrModuleInit();

	/* Disable all the Error Actions */
	XPlmi_Out32(PMC_GLOBAL_PMC_POR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR_OUT1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_IRQ1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_SRST1_DIS, MASK32_ALL_HIGH);

	/* Clear the error status registers */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Detect if we are in over-temperature condition */
	XPlmi_SysMonOTDetect();

	/* Set the default actions as defined in the Error table */
	for (Index = XPM_NODEIDX_ERROR_BOOT_CR;
	       Index < XPM_NODEIDX_ERROR_PMCERR1_MAX; Index++) {
		if (XPlmi_EmSetAction(NODEID(XPM_NODECLASS_EVENT,
		     XPM_NODESUBCL_EVENT_ERROR, XPM_NODETYPE_EVENT_PMC_ERR1,
		     Index), ErrorTable[Index].Action,
		      ErrorTable[Index].Handler) != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
			     "Warning: XPlmi_EmInit: Failed to "
			     "set action for PMC ERR1: %d\r\n", Index)
		}
	}

	for (Index = XPM_NODEIDX_ERROR_PMCAPB;
	          Index < XPM_NODEIDX_ERROR_PMCERR2_MAX; Index++) {
		if (XPlmi_EmSetAction(NODEID(XPM_NODECLASS_EVENT,
		     XPM_NODESUBCL_EVENT_ERROR, XPM_NODETYPE_EVENT_PMC_ERR2,
		     Index), ErrorTable[Index].Action,
		      ErrorTable[Index].Handler) != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
			     "Warning: XPlmi_EmInit: Failed to "
			     "set action for PMC ERR2: %d\r\n", Index)
		}
	}

}

/*****************************************************************************/
/**
 * @brief This function initializes the PSM error actions. Disables all the
 * PSM error actions and registers default action
 * @param None
 * @return None
*****************************************************************************/
int XPlmi_PsEmInit(void)
{
	u32 Index;

	/* Disable all the Error Actions */
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_CR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_NCR_ERR1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_IRQ1_DIS, MASK32_ALL_HIGH);
	XPlmi_Out32(PSM_GLOBAL_REG_PSM_SRST1_DIS, MASK32_ALL_HIGH);

	/* Clear the error status registers */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, MASK32_ALL_HIGH);
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, MASK32_ALL_HIGH);

	/* Set the default actions as defined in the Error table */
	for (Index = XPM_NODEIDX_ERROR_PS_SW_CR;
	           Index < XPM_NODEIDX_ERROR_PSMERR1_MAX; Index++) {
		if (XPlmi_EmSetAction(NODEID(XPM_NODECLASS_EVENT,
		     XPM_NODESUBCL_EVENT_ERROR, XPM_NODETYPE_EVENT_PSM_ERR1,
		     Index), ErrorTable[Index].Action,
		      ErrorTable[Index].Handler) != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
			     "Warning: XPlmi_PsEmInit: Failed to "
			     "set action for PSM ERR1: %d\r\n", Index)
		}
	}

	for (Index = XPM_NODEIDX_ERROR_LPD_SWDT;
	          Index < XPM_NODEIDX_ERROR_PSMERR2_MAX; Index++) {
		if (XPlmi_EmSetAction(NODEID(XPM_NODECLASS_EVENT,
		     XPM_NODESUBCL_EVENT_ERROR, XPM_NODETYPE_EVENT_PSM_ERR2,
		     Index), ErrorTable[Index].Action,
		      ErrorTable[Index].Handler) != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_GENERAL,
			     "Warning: XPlmi_PsEmInit: Failed to "
			     "set action for PSM ERR2: %d\r\n", Index)
		}
	}
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
 * This function dumps the registers which can help debugging
 *
 * @param
 *
 * @return none
 *
 * @note none
 *****************************************************************************/
void XPlmi_DumpRegisters()
{

	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");

	XPlmi_Printf(DEBUG_GENERAL, "IDCODE: 0x%08x\n\r",
		      XPlmi_In32(PMC_TAP_IDCODE));
	XPlmi_Printf(DEBUG_GENERAL, "Version: 0x%08x\n\r",
		      XPlmi_In32(PMC_TAP_VERSION));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode User: 0x%08x\n\r",
		      XPlmi_In32(CRP_BOOT_MODE_USER));
	XPlmi_Printf(DEBUG_GENERAL, "Bootmode POR: 0x%08x\n\r",
		      XPlmi_In32(CRP_BOOT_MODE_POR));
	XPlmi_Printf(DEBUG_GENERAL, "Reset Reason: 0x%08x\n\r",
		      XPlmi_In32(CRP_RESET_REASON));
	XPlmi_Printf(DEBUG_GENERAL, "Multiboot: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_MULTI_BOOT));
	XPlmi_Printf(DEBUG_GENERAL, "PMC PWR Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PWR_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC GSW Err: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_GSW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC FW Error: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_FW_ERR));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT1 Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "PMC ERR OUT2 Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP0 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP0_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP1 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP1_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP2 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP2_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP3 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP3_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP4 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP4_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP5 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP5_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP6 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP6_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICP7 IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP7_IRQ_STATUS));
	XPlmi_Printf(DEBUG_GENERAL, "GICPPMC IRQ Status: 0x%08x\n\r",
		      XPlmi_In32(PMC_GLOBAL_GICP_PMC_IRQ_STATUS));

	XPlmi_Printf(DEBUG_GENERAL, "====Register Dump============\n\r");
}
