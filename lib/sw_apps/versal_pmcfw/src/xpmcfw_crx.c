/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmcfw_crx.c
*
* This file which contains the code related to clock and resets.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/


/***************************** Include Files *********************************/
#include "xpmcfw_crx.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/

/*****************************************************************************/
/**
 * This function releases the reset of APU1
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XCrx_RelApu1(XCrx *InstancePtr)
{
	u32 RvbarHigh;
	u32 RvbarLow;
	u32 RegVal;

	XPmcFw_Printf(DEBUG_INFO, "Taking A72-1 out of reset\r\n");

	RvbarLow = (u32)(InstancePtr->RvbarAddr & 0xFFFFFFFFU);
	RvbarHigh = (u32)((InstancePtr->RvbarAddr>>32) & 0xFFFFFFFFU);

	XPmcFw_Printf(DEBUG_INFO, "RVBARL: 0x%08x, RVBARH: 0x%08x\r\n",
		      RvbarLow, RvbarHigh);

	/* configure the reset vector location */
	Xil_Out32(FPD_APU_RVBARADDR1L, RvbarLow);
	Xil_Out32(FPD_APU_RVBARADDR1H, RvbarHigh);

	/* Read the APU config */
	RegVal = Xil_In32(FPD_APU_CONFIG_0);

	/* Set AArch state 64 Vs 32 bit */
	if (InstancePtr->AA64nAA32 == XCRX_APU_AA64)
	{
		RegVal |=  FPD_APU_CONFIG_0_AA64N32_MASK_CPU1;
	} else {
		RegVal &= ~(FPD_APU_CONFIG_0_AA64N32_MASK_CPU1);
	}

	/* Set Vector location for 32 bit */
	if (InstancePtr->VInitHi == XCRX_CPU_VINITHI_HIVEC)
	{
		RegVal |=  FPD_APU_CONFIG_0_VINITHI_MASK_CPU1;
	} else {
		RegVal &= ~(FPD_APU_CONFIG_0_VINITHI_MASK_CPU1);
	}

	/* Update the APU configuration */
	Xil_Out32(FPD_APU_CONFIG_0, RegVal);
#if 0
	/*TODO remove the hack to loop A72 */
	Xil_Out32(RvbarLow, 0x14000000);
#endif

	/* Release reset */
	RegVal = Xil_In32(CRF_RST_APU);
	RegVal &= ~(CRF_RST_APU_ACPU1_PWRON_MASK |
		    CRF_RST_APU_ACPU_L2_RESET_MASK |
		    CRF_RST_APU_ACPU1_MASK);
	Xil_Out32(CRF_RST_APU, RegVal);
}

/*****************************************************************************/
/**
 * This function releases the reset of APU
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XCrx_RelApu0(XCrx *InstancePtr)
{
	u32 RvbarHigh;
	u32 RvbarLow;
	u32 RegVal;

	XPmcFw_Printf(DEBUG_INFO, "Taking A72-0 out of reset\r\n");

	RvbarLow = (u32)(InstancePtr->RvbarAddr & 0xFFFFFFFFU);
	RvbarHigh = (u32)((InstancePtr->RvbarAddr>>32) & 0xFFFFFFFFU);

	XPmcFw_Printf(DEBUG_INFO, "RVBARL: 0x%08x, RVBARH: 0x%08x\r\n",
		      RvbarLow, RvbarHigh);

	/* configure the reset vector location */
	Xil_Out32(FPD_APU_RVBARADDR0L, RvbarLow);
	Xil_Out32(FPD_APU_RVBARADDR0H, RvbarHigh);

	/* Read the APU config */
	RegVal = Xil_In32(FPD_APU_CONFIG_0);

	/* Set Aarch state 64 Vs 32 bit */
	if (InstancePtr->AA64nAA32 == XCRX_APU_AA64)
	{
		RegVal |=  FPD_APU_CONFIG_0_AA64N32_MASK_CPU0;
	} else {
		RegVal &= ~(FPD_APU_CONFIG_0_AA64N32_MASK_CPU0);
	}

	/* Set Vector location for 32 bit */
	if (InstancePtr->VInitHi == XCRX_CPU_VINITHI_HIVEC)
	{
		RegVal |=  FPD_APU_CONFIG_0_VINITHI_MASK_CPU0;
	} else {
		RegVal &= ~(FPD_APU_CONFIG_0_VINITHI_MASK_CPU0);
	}

	/* Update the APU configuration */
	Xil_Out32(FPD_APU_CONFIG_0, RegVal);
#if 0
	/*TODO remove the hack to loop A72 */
	Xil_Out32(RvbarLow, 0x14000000);
#endif

	/* Release reset */
	RegVal = Xil_In32(CRF_RST_APU);
	RegVal &= ~(CRF_RST_APU_ACPU0_PWRON_MASK |
		    CRF_RST_APU_ACPU_L2_RESET_MASK |
		    CRF_RST_APU_ACPU0_MASK);
	Xil_Out32(CRF_RST_APU, RegVal);
}

/*****************************************************************************/
/**
 * This function releases the reset of RPU
 *
 * @param	InstancePtr Pointer to the Crx Instance
 *
 * @return	None
 *
 *****************************************************************************/
void XCrx_RelRpu(XCrx *InstancePtr)
{
	u32 RegVal;
	u32 RpuBaseOfst=0U;

	if (InstancePtr->CpuNo == XCRX_RPU_CPU_1)
	{
		XPmcFw_Printf(DEBUG_INFO, "Configuring R5-1: ");
		RpuBaseOfst = XCRX_RPU_1_BASE_OFFSET;
	} else {
		XPmcFw_Printf(DEBUG_INFO, "Configuring R5-0/L: ");
	}

	RegVal = Xil_In32(RPU_RPU_0_CFG + RpuBaseOfst);

	/* Do the vector the configuration */
	if (InstancePtr->VInitHi == XCRX_CPU_VINITHI_HIVEC) {
		/* Start from HiVec */
		XPmcFw_Printf(DEBUG_INFO, "HIVEC, ");
		RegVal |= RPU_RPU_0_CFG_VINITHI_MASK;
	} else {
		/* Start from LoVec */
		XPmcFw_Printf(DEBUG_INFO, "LOVEC, ");
		RegVal &= ~RPU_RPU_0_CFG_VINITHI_MASK;
	}

	/* Do the halt configuration */
	if (InstancePtr->Halt == XCRX_CPU_HALT) {
		/* Halt the CPU */
		XPmcFw_Printf(DEBUG_INFO, "HALT, ");
		RegVal &= ~RPU_RPU_0_CFG_NCPUHALT_MASK;
	} else {
		/* Start the CPU */
		XPmcFw_Printf(DEBUG_INFO, "RUN, ");
		RegVal |= RPU_RPU_0_CFG_NCPUHALT_MASK;
	}
	Xil_Out32(RPU_RPU_0_CFG + RpuBaseOfst, RegVal);

	RegVal = Xil_In32(RPU_RPU_GLBL_CNTL);
	if (InstancePtr->CpuNo == XCRX_RPU_CPU_L)
	{
	/* Lockstep Mode */
		/* No Split, Clamp output, TCM combine */
		XPmcFw_Printf(DEBUG_INFO, "LOCKSTEP\n\r");
		RegVal &= ~RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
		RegVal |= (RPU_RPU_GLBL_CNTL_SLCLAMP_MASK
			    | RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
	} else {
	/* Split mode */
		/* Split, No Clamp output, No TCM combine */
		XPmcFw_Printf(DEBUG_INFO, "SPLIT\n\r");
		RegVal |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
		RegVal &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK
			    | RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
	}
	Xil_Out32(RPU_RPU_GLBL_CNTL, RegVal);

	/* Release reset of R5 */
	RegVal = Xil_In32(CRL_RST_CPU_R5);
	if (InstancePtr->CpuNo == XCRX_RPU_CPU_L)
	{
		RegVal &= ~(CRL_RST_CPU_R5_RESET_CPU0_MASK |
			    CRL_RST_CPU_R5_RESET_CPU1_MASK);
	} else if (InstancePtr->CpuNo == XCRX_RPU_CPU_0) {
		RegVal &= ~CRL_RST_CPU_R5_RESET_CPU0_MASK;
	} else {
		RegVal &= ~CRL_RST_CPU_R5_RESET_CPU1_MASK;
	}
	RegVal &= ~CRL_RST_CPU_R5_RESET_PGE_MASK;
	RegVal &= ~CRL_RST_CPU_R5_RESET_AMBA_MASK;
	Xil_Out32(CRL_RST_CPU_R5, RegVal);
}

/*****************************************************************************/
/**
 * This function wakes up PSM
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XCrx_WakeUpPsm()
{
	XPmcFw_Printf(DEBUG_INFO,
		      "\nWaking Up PSM from Sleep\r\n");
	XPmcFw_UtilRMW(CRL_PSM_RST_MODE,
		       CRL_PSM_RST_MODE_WAKEUP_MASK,
		       CRL_PSM_RST_MODE_WAKEUP_MASK);
}

/*****************************************************************************/
/**
 * This function releases the reset of PSM and put it in sleep mode
 *
 * @param	None
 *
 * @return	None
 *
 *****************************************************************************/
void XCrx_RelPsm()
{
	XPmcFw_Printf(DEBUG_INFO, "Taking PSM out of reset"
		      " and placing in sleep mode\n\r");
	XPmcFw_UtilRMW(CRL_PSM_RST_MODE,
		       CRL_PSM_RST_MODE_RST_MODE_MASK, 0x1U);
	XPmcFw_UtilRMW(PSM_GLOBAL_REG_GLOBAL_CNTRL,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_MB_CLK_EN_FORCE_MASK,
		       PSM_GLOBAL_REG_GLOBAL_CNTRL_MB_CLK_EN_FORCE_MASK);

}
