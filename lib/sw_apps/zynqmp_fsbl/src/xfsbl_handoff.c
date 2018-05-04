/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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

/*****************************************************************************/
/**
 *
 * @file xfsbl_handoff.c
 *
 * This is the main file which contains handoff code for the FSBL.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  kc   10/21/13 Initial release
 * 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
 *       vns           Added support for HIVEC.
 *       bo   01/25/17 During handoff again R5 is restored to LOVEC.
 *       sc   02/04/17 Lock XMPU/XPPU for further access but by default
 *                     it is by passed.
 *       bv   03/17/17 Modified such that XFsbl_PmInit is done only duing
 *                     system reset
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xil_cache.h"
#include "psu_init.h"
#include "xfsbl_main.h"
#include "xfsbl_image_header.h"
#include "xfsbl_bs.h"

/************************** Constant Definitions *****************************/
#define XFSBL_CPU_POWER_UP		(0x1U)
#define XFSBL_CPU_SWRST			(0x2U)

/**
 * Aarch32 or Aarch64 CPU definitions
 */
#define APU_CONFIG_0_AA64N32_MASK_CPU0 (0x1U)
#define APU_CONFIG_0_AA64N32_MASK_CPU1 (0x2U)
#define APU_CONFIG_0_AA64N32_MASK_CPU2 (0x4U)
#define APU_CONFIG_0_AA64N32_MASK_CPU3 (0x8U)

#define APU_CONFIG_0_VINITHI_MASK_CPU0  (u32)(0x100U)
#define APU_CONFIG_0_VINITHI_MASK_CPU1  (u32)(0x200U)
#define APU_CONFIG_0_VINITHI_MASK_CPU2  (u32)(0x400U)
#define APU_CONFIG_0_VINITHI_MASK_CPU3  (u32)(0x800U)

#define APU_CONFIG_0_VINITHI_SHIFT_CPU0	(8U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU1	(9U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU2	(10U)
#define APU_CONFIG_0_VINITHI_SHIFT_CPU3	(11U)

#define OTHER_CPU_HANDOFF				(0x0U)
#define A53_0_64_HANDOFF_TO_A53_0_32	(0x1U)
#define A53_0_32_HANDOFF_TO_A53_0_64	(0x2U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

static u32 XFsbl_SetCpuPwrSettings (u32 CpuSettings, u32 Flags);
static void XFsbl_UpdateResetVector (u64 HandOffAddress, u32 CpuSettings,
		u32 HandoffType, u32 Vector);
static u32 XFsbl_Is32BitCpu(u32 CpuSettings);
static u32 XFsbl_CheckEarlyHandoffCpu(u32 CpuId);
static u32 XFsbl_ProtectionConfig(void);


/**
 * Functions defined in xfsbl_handoff.S
 */
extern void XFsbl_Exit(PTRSIZE HandoffAddress, u32 Flags);

/************************** Variable Definitions *****************************/

#ifdef ARMR5
/* Variables defined in xfsbl_partition_load.c */
extern u8 R5LovecBuffer[32];
extern u32 TcmSkipLength;
extern PTRSIZE TcmSkipAddress;
#endif

static u32 XFsbl_Is32BitCpu(u32 CpuSettings)
{
	u32 Status;
	u32 CpuId;
	u32 ExecState;

        CpuId = CpuSettings & XIH_PH_ATTRB_DEST_CPU_MASK;
        ExecState = CpuSettings & XIH_PH_ATTRB_A53_EXEC_ST_MASK;

	if ((CpuId == XIH_PH_ATTRB_DEST_CPU_R5_0) ||
	        (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_1) ||
	(CpuId == XIH_PH_ATTRB_DEST_CPU_R5_L)  ||
		(ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32))
	{
		Status = TRUE;
	} else {
		Status = FALSE;
	}

	return Status;
}


/****************************************************************************/
/**
 * This function will set up the settings for the CPU's
 * This can power up the CPU or do a soft reset to the CPU's
 *
 * @param CpuId specifies for which CPU settings should be done
 *
 * @param Flags is used to specify the settings for the CPU
 * 			XFSBL_CPU_POWER_UP - This is used to power up the CPU
 * 			XFSBL_CPU_SWRST - This is used to trigger the reset to CPU
 *
 * @return
 * 		- XFSBL_SUCCESS on successful settings
 * 		- XFSBL_FAILURE
 *
 * @note
 *
 *****************************************************************************/

static u32 XFsbl_SetCpuPwrSettings (u32 CpuSettings, u32 Flags)
{
	u32 RegValue;
	u32 Status;
	u32 CpuId;
	u32 ExecState;
	u32 PwrStateMask;

	/**
	 * Reset the CPU
	 */
	if ((Flags & XFSBL_CPU_SWRST) != 0U)
	{
	CpuId = CpuSettings & XIH_PH_ATTRB_DEST_CPU_MASK;
		ExecState = CpuSettings & XIH_PH_ATTRB_A53_EXEC_ST_MASK;
		switch(CpuId)
		{

			case XIH_PH_ATTRB_DEST_CPU_A53_0:

			PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU0_MASK |
				PMU_GLOBAL_PWR_STATE_FP_MASK |
				PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

			Status = XFsbl_PowerUpIsland(PwrStateMask);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_A53_0_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_A53_0_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Set to Aarch32 if enabled
			 */
			if (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)
			{
				RegValue = XFsbl_In32(APU_CONFIG_0);
				RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU0);
				XFsbl_Out32(APU_CONFIG_0, RegValue);
			}

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRF_APB_ACPU_CTRL);
			RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
			             CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
			XFsbl_Out32(CRF_APB_ACPU_CTRL, RegValue);

			/**
			 * Release reset
			 */
			RegValue = XFsbl_In32(CRF_APB_RST_FPD_APU);
			RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU0_RESET_MASK |
					CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
					CRF_APB_RST_FPD_APU_ACPU0_PWRON_RESET_MASK);
			XFsbl_Out32(CRF_APB_RST_FPD_APU, RegValue);

			break;

		case XIH_PH_ATTRB_DEST_CPU_A53_1:

			PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU1_MASK |
				PMU_GLOBAL_PWR_STATE_FP_MASK |
				PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

			Status = XFsbl_PowerUpIsland(PwrStateMask);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_A53_1_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_A53_1_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Set to Aarch32 if enabled
			 */
			if (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)
			{
				RegValue = XFsbl_In32(APU_CONFIG_0);
				RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU1);
				XFsbl_Out32(APU_CONFIG_0, RegValue);
			}

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRF_APB_ACPU_CTRL);
			RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
			             CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
			XFsbl_Out32(CRF_APB_ACPU_CTRL, RegValue);

			/**
			 * Release reset
			 */
			RegValue = XFsbl_In32(CRF_APB_RST_FPD_APU);
			RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU1_RESET_MASK |
					CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
					CRF_APB_RST_FPD_APU_ACPU1_PWRON_RESET_MASK);
			XFsbl_Out32(CRF_APB_RST_FPD_APU, RegValue);

			break;

		case XIH_PH_ATTRB_DEST_CPU_A53_2:

			PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU2_MASK |
				PMU_GLOBAL_PWR_STATE_FP_MASK |
				PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

			Status = XFsbl_PowerUpIsland(PwrStateMask);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_A53_2_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_A53_2_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Set to Aarch32 if enabled
			 */
			if (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)
			{
				RegValue = XFsbl_In32(APU_CONFIG_0);
				RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU2);
				XFsbl_Out32(APU_CONFIG_0, RegValue);
			}

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRF_APB_ACPU_CTRL);
			RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
			             CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
			XFsbl_Out32(CRF_APB_ACPU_CTRL, RegValue);

			/**
			 * Release reset
			 */
			RegValue = XFsbl_In32(CRF_APB_RST_FPD_APU);
			RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU2_RESET_MASK |
					CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
					CRF_APB_RST_FPD_APU_ACPU2_PWRON_RESET_MASK);

			XFsbl_Out32(CRF_APB_RST_FPD_APU, RegValue);

			break;

		case XIH_PH_ATTRB_DEST_CPU_A53_3:

			PwrStateMask = PMU_GLOBAL_PWR_STATE_ACPU3_MASK |
				PMU_GLOBAL_PWR_STATE_FP_MASK |
				PMU_GLOBAL_PWR_STATE_L2_BANK0_MASK;

			Status = XFsbl_PowerUpIsland(PwrStateMask);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_A53_3_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_A53_3_POWER_UP\r\n");
				goto END;
			}


			/**
			 * Set to Aarch32 if enabled
			 */
			if (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)
			{
				RegValue = XFsbl_In32(APU_CONFIG_0);
				RegValue &= ~(APU_CONFIG_0_AA64N32_MASK_CPU3);
				XFsbl_Out32(APU_CONFIG_0, RegValue);
			}

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRF_APB_ACPU_CTRL);
			RegValue |= (CRF_APB_ACPU_CTRL_CLKACT_FULL_MASK |
			             CRF_APB_ACPU_CTRL_CLKACT_HALF_MASK);
			XFsbl_Out32(CRF_APB_ACPU_CTRL, RegValue);

			/**
			 * Release reset
			 */
			RegValue = XFsbl_In32(CRF_APB_RST_FPD_APU);
			RegValue &= ~(CRF_APB_RST_FPD_APU_ACPU3_RESET_MASK |
					CRF_APB_RST_FPD_APU_APU_L2_RESET_MASK |
					CRF_APB_RST_FPD_APU_ACPU3_PWRON_RESET_MASK);

			XFsbl_Out32(CRF_APB_RST_FPD_APU, RegValue);

			break;

		case XIH_PH_ATTRB_DEST_CPU_R5_0:

			Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_0_MASK);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_0_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_0_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Place R5, TCM's in split mode
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue |= (RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
			 * Place R5-0 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_0_CFG);
			RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_0_CFG, RegValue);

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
			RegValue |= (CRL_APB_CPU_R5_CTRL_CLKACT_MASK);
			XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

			/**
			 * Provide some delay,
			 * so that clock propogates properly.
			 */
			(void)usleep(0x50U);


			/**
			 * Release reset to R5-0
			 */
			RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
			XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);


			/**
			 * Take R5-0 out of HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_0_CFG);
			RegValue |= RPU_RPU_0_CFG_NCPUHALT_MASK;
			XFsbl_Out32(RPU_RPU_0_CFG, RegValue);
			break;

		case XIH_PH_ATTRB_DEST_CPU_R5_1:

			Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_1_MASK);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_1_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_1_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Place R5, TCM's in split mode
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue |= RPU_RPU_GLBL_CNTL_SLSPLIT_MASK;
			RegValue &= ~(RPU_RPU_GLBL_CNTL_TCM_COMB_MASK);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLCLAMP_MASK);
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
			 * Place R5-1 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_1_CFG);
			RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_1_CFG, RegValue);

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
			RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
			XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

			/**
			 * Provide some delay,
			 * so that clock propogates properly.
			 */
			(void)usleep(0x50U);

			/**
			 * Release reset to R5-1
			 */
			RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
			XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);


			/**
			 * Take R5-1 out of HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_1_CFG);
			RegValue |= RPU_RPU_1_CFG_NCPUHALT_MASK;
			XFsbl_Out32(RPU_RPU_1_CFG, RegValue);
			break;
		case XIH_PH_ATTRB_DEST_CPU_R5_L:

			Status = XFsbl_PowerUpIsland(PMU_GLOBAL_PWR_STATE_R5_0_MASK);
			if (Status != XFSBL_SUCCESS) {
				Status = XFSBL_ERROR_R5_L_POWER_UP;
				XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_R5_L_POWER_UP\r\n");
				goto END;
			}

			/**
			 * Place R5, TCM's in safe mode
			 */
			RegValue = XFsbl_In32(RPU_RPU_GLBL_CNTL);
			RegValue &= ~(RPU_RPU_GLBL_CNTL_SLSPLIT_MASK);
			RegValue |= RPU_RPU_GLBL_CNTL_TCM_COMB_MASK;
			RegValue |= RPU_RPU_GLBL_CNTL_SLCLAMP_MASK;
			XFsbl_Out32(RPU_RPU_GLBL_CNTL, RegValue);

			/**
			 * Place R5-0 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_0_CFG);
			RegValue &= ~(RPU_RPU_0_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_0_CFG, RegValue);

			/**
			 * Place R5-1 in HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_1_CFG);
			RegValue &= ~(RPU_RPU_1_CFG_NCPUHALT_MASK);
			XFsbl_Out32(RPU_RPU_1_CFG, RegValue);

			/**
			 *  Enable the clock
			 */
			RegValue = XFsbl_In32(CRL_APB_CPU_R5_CTRL);
			RegValue |= CRL_APB_CPU_R5_CTRL_CLKACT_MASK;
			XFsbl_Out32(CRL_APB_CPU_R5_CTRL, RegValue);

			/**
			 * Provide some delay,
			 * so that clock propogates properly.
			 */
			(void )usleep(0x50U);

			/**
			 * Release reset to R5-0, R5-1
			 */
			RegValue = XFsbl_In32(CRL_APB_RST_LPD_TOP);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R50_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_R51_RESET_MASK);
			RegValue &= ~(CRL_APB_RST_LPD_TOP_RPU_AMBA_RESET_MASK);
			XFsbl_Out32(CRL_APB_RST_LPD_TOP, RegValue);


			/**
			 * Take R5-0 out of HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_0_CFG);
			RegValue |= RPU_RPU_0_CFG_NCPUHALT_MASK;
			XFsbl_Out32(RPU_RPU_0_CFG, RegValue);

			/**
			 * Take R5-1 out of HALT state
			 */
			RegValue = XFsbl_In32(RPU_RPU_1_CFG);
			RegValue |= RPU_RPU_1_CFG_NCPUHALT_MASK;
			XFsbl_Out32(RPU_RPU_1_CFG, RegValue);
			break;

		default:
			XFsbl_Printf(DEBUG_GENERAL,
			    "XFSBL_ERROR_HANDOFF_CPUID\n\r");
			Status = XFSBL_ERROR_HANDOFF_CPUID;
			break;
		}

	}
	else
	{
		Status = XFSBL_SUCCESS;
	}
END:
	return Status;
}

/****************************************************************************/
/**
 * FSBL exit function before the assembly code
 *
 * @param HandoffAddress is handoff address for the FSBL running cpu
 *
 * @param Flags is to determine whether to handoff to applicatio or
 * 			to be in wfe state
 *
 * @return None
 *
 *
 *****************************************************************************/
void XFsbl_HandoffExit(u64 HandoffAddress, u32 Flags)
{

	/**
	 * Flush the L1 data cache and L2 cache, Disable Data Cache
	 */
	Xil_DCacheDisable();

	XFsbl_Printf(DEBUG_GENERAL,"Exit from FSBL \n\r");

	/**
	 * Exit to handoff address
	 * PTRSIZE is used since handoff is in same running cpu
	 * and address is of PTRSIZE
	 */
	XFsbl_Exit((PTRSIZE) HandoffAddress, Flags);

	/**
	 * should not reach here
	 */
	return ;

}


/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static void XFsbl_UpdateResetVector (u64 HandOffAddress, u32 CpuSettings,
		u32 HandoffType, u32 Vector)
{
	u32 HandOffAddressLow;
	u32 HandOffAddressHigh;
	u32 LowAddressReg;
	u32 HighAddressReg;
	u32 CpuId;
	u32 RegVal;
	u32 ExecState;

	CpuId = CpuSettings & XIH_PH_ATTRB_DEST_CPU_MASK;
	ExecState = CpuSettings & XIH_PH_ATTRB_A53_EXEC_ST_MASK;

	/**
	 * Put R5 or A53-32 in Lovec/Hivec
	 */
	if ((CpuId == XIH_PH_ATTRB_DEST_CPU_R5_0)
			|| (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_L)) {
		RegVal = XFsbl_In32(RPU_RPU_0_CFG);
		RegVal &= ~RPU_RPU_0_CFG_VINITHI_MASK;
		RegVal |= (Vector << RPU_RPU_0_CFG_VINITHI_SHIFT);
		XFsbl_Out32(RPU_RPU_0_CFG, RegVal);
	}

	else if ((CpuId == XIH_PH_ATTRB_DEST_CPU_R5_1)
			|| (CpuId == XIH_PH_ATTRB_DEST_CPU_R5_L)) {
		RegVal = XFsbl_In32(RPU_RPU_1_CFG);
		RegVal &= ~RPU_RPU_1_CFG_VINITHI_MASK;
		RegVal |= (Vector << RPU_RPU_1_CFG_VINITHI_SHIFT);
		XFsbl_Out32(RPU_RPU_1_CFG, RegVal);
	}

	else if ((CpuId == XIH_PH_ATTRB_DEST_CPU_A53_0)
		&& (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)) {
		RegVal = XFsbl_In32(APU_CONFIG_0);
		RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU0;
		RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU0);
		XFsbl_Out32(APU_CONFIG_0, RegVal);
	}

	else if ((CpuId == XIH_PH_ATTRB_DEST_CPU_A53_1)
		&& (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)) {
		RegVal = XFsbl_In32(APU_CONFIG_0);
		RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU1;
		RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU1);
		XFsbl_Out32(APU_CONFIG_0, RegVal);
	}

	else if ((CpuId == XIH_PH_ATTRB_DEST_CPU_A53_2)
			&& (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)) {
		RegVal = XFsbl_In32(APU_CONFIG_0);
		RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU2;
		RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU2);
		XFsbl_Out32(APU_CONFIG_0, RegVal);
		}

	else if ((CpuId == XIH_PH_ATTRB_DEST_CPU_A53_3)
		&& (ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)) {
		RegVal = XFsbl_In32(APU_CONFIG_0);
		RegVal &= ~APU_CONFIG_0_VINITHI_MASK_CPU3;
		RegVal |= (Vector << APU_CONFIG_0_VINITHI_SHIFT_CPU3);
		XFsbl_Out32(APU_CONFIG_0, RegVal);
	}
	else
	{
		/* for MISRA C compliance */
	}

	if ((XFsbl_Is32BitCpu(CpuSettings)==FALSE)
			&& (HandoffType != A53_0_32_HANDOFF_TO_A53_0_64))
	{
		/**
		 * for A53 cpu, write 64bit handoff address
		 * to the RVBARADDR in APU
		 */

		HandOffAddressLow = (u32 )(HandOffAddress & 0xFFFFFFFFU);
		HandOffAddressHigh = (u32 )((HandOffAddress>>32)
							& 0xFFFFFFFFU);
		switch (CpuId)
		{
			case XIH_PH_ATTRB_DEST_CPU_A53_0:
				LowAddressReg = APU_RVBARADDR0L;
				HighAddressReg = APU_RVBARADDR0H;
				break;
			case XIH_PH_ATTRB_DEST_CPU_A53_1:
				LowAddressReg = APU_RVBARADDR1L;
				HighAddressReg = APU_RVBARADDR1H;
				break;
			case XIH_PH_ATTRB_DEST_CPU_A53_2:
				LowAddressReg = APU_RVBARADDR2L;
				HighAddressReg = APU_RVBARADDR2H;
				break;
			case XIH_PH_ATTRB_DEST_CPU_A53_3:
				LowAddressReg = APU_RVBARADDR3L;
				HighAddressReg = APU_RVBARADDR3H;
				break;
			default:
				/**
				 * error can be triggered here
				 */
				LowAddressReg = 0U;
				HighAddressReg = 0U;
				break;
		}
		XFsbl_Out32(LowAddressReg, HandOffAddressLow);
		XFsbl_Out32(HighAddressReg, HandOffAddressHigh);
	}

	return;
}

/*****************************************************************************/
/**
 * This function handoff the images to the respective cpu's
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number of the image
 *
 * @param	EarlyHandoff is flag to indicate if called for early handoff or not
 *
 * @return	returns the error codes described in xfsbl_error.h on any error
 *
 * @note	This function should not return incase of success
 *
 *****************************************************************************/

u32 XFsbl_Handoff (const XFsblPs * FsblInstancePtr, u32 PartitionNum, u32 EarlyHandoff)
{
	u32 Status;
	u32 CpuIndex;
	u32 CpuId;
	u32 ExecState;
	u32 CpuSettings;
	u64 HandoffAddress;
	u64 RunningCpuHandoffAddress=0U;
	u32 RunningCpuExecState=0U;
	u32 RunningCpuHandoffAddressPresent=FALSE;
	u32 CpuNeedsEarlyHandoff;
	const XFsblPs_PartitionHeader * PartitionHeader;

	static u32 CpuIndexEarlyHandoff = 0;

	/* Restoring the SD card detection signal */
	XFsbl_Out32(IOU_SLCR_SD_CDN_CTRL, 0X0U);
	PartitionHeader =
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];

	if (FsblInstancePtr->ResetReason == XFSBL_PS_ONLY_RESET)
		{
		/**Remove PS-PL isolation to allow u-boot and linux to access PL*/
			(void)psu_ps_pl_isolation_removal_data();
			(void)psu_ps_pl_reset_config_data();
		}
	if(FsblInstancePtr->ResetReason != XFSBL_APU_ONLY_RESET){

	Status = XFsbl_PmInit();
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_PM_INIT;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PM_INIT\r\n");
		goto END;
	}


	Status = XFsbl_ProtectionConfig();
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	XFsbl_Printf(DEBUG_GENERAL, "Protection configuration applied\r\n");

	}

	/**
	 * if JTAG bootmode, be in while loop as of now
	 * Check if Process can be parked in HALT state
	 */
	if (FsblInstancePtr->PrimaryBootDevice ==
			XFSBL_JTAG_BOOT_MODE)
	{
		Status = XFsbl_PLCheckForDone();
		if(Status==XFSBL_SUCCESS)
		{
			/**Remove PS-PL isolation as bitstream is loaded*/
			(void)psu_ps_pl_isolation_removal_data();
			(void)psu_ps_pl_reset_config_data();
		}
		/**
		 * Mark Error status with Fsbl completed
		 */
		XFsbl_Out32(XFSBL_ERROR_STATUS_REGISTER_OFFSET,
		    XFSBL_COMPLETED);

		if (XGet_Zynq_UltraMp_Platform_info() == (u32)(0X2U))
		{
			/**
			 * Flush the L1 data cache and L2 cache, Disable Data Cache
			 */
			Xil_DCacheDisable();
			XFsbl_Printf(DEBUG_GENERAL,"Exit from FSBL. \n\r");
#ifdef ARMA53_64
			XFsbl_Out32(0xFFFC0000U, 0x14000000U);
#else
			XFsbl_Out32(0xFFFC0000U, 0xEAFFFFFEU);
#endif
			XFsbl_Exit(0xFFFC0000U, XFSBL_HANDOFFEXIT);
		} else {
			/**
			 * Exit from FSBL
			 */
			XFsbl_HandoffExit(0U, XFSBL_NO_HANDOFFEXIT);
		}

	}

	/**
	 * if XIP image present
	 * Put QSPI in linear mode
	 */

	/**
	 * FSBL hook before Handoff
	 */
	Status = XFsbl_HookBeforeHandoff(EarlyHandoff);
	if (Status != XFSBL_SUCCESS)
	{
		Status = XFSBL_ERROR_HOOK_BEFORE_HANDOFF;
		XFsbl_Printf(DEBUG_GENERAL,
				"XFSBL_ERROR_HOOK_BEFORE_HANDOFF\r\n");
		goto END;
	}

	/**
	 * Disable Data Cache to have smooth data
	 * transfer between the processors.
	 * Data transfer is required to update flag for CPU out of reset
	 */
	Xil_DCacheDisable();

	/**
	 * get cpu out of reset
	 *
	 */

	/**
	 * If we are doing early handoff, remember the CPU index to avoid
	 * traversing through for the next early handoff
	 */
	if (EarlyHandoff == TRUE) {
		CpuIndex = CpuIndexEarlyHandoff;
	}
	else
	{
		CpuIndex = 0U;
	}

	while (CpuIndex < FsblInstancePtr->HandoffCpuNo)
	{
		CpuSettings =
				FsblInstancePtr->HandoffValues[CpuIndex].CpuSettings;

		CpuId = CpuSettings & XIH_PH_ATTRB_DEST_CPU_MASK;
		ExecState = CpuSettings & XIH_PH_ATTRB_A53_EXEC_ST_MASK;

		/**
		 * Run the code in this loop in the below conditions:
		 * - This function called for early handoff and CPU needs early handoff
		 * - This function called for regular handoff and CPU doesn't need early
		 *   handoff
		 * - This function called for regular handoff and CPU needs early
		 *   handoff AND if handoff is to running CPU
		 *
		 */
		CpuNeedsEarlyHandoff = XFsbl_CheckEarlyHandoffCpu(CpuId);
		if (((CpuNeedsEarlyHandoff == TRUE) && (EarlyHandoff == TRUE)) ||
				((EarlyHandoff != TRUE) && (CpuNeedsEarlyHandoff != TRUE)) ||
				(((EarlyHandoff != TRUE) && (CpuNeedsEarlyHandoff == TRUE)) &&
						(CpuId == FsblInstancePtr->ProcessorID))) {

			/**
			 * Check if handoff address is present
			 */
			if (CpuId != FsblInstancePtr->ProcessorID)
			{

				/* Check if handoff CPU is supported */
				Status = XFsbl_CheckSupportedCpu(CpuId);
				if (XFSBL_SUCCESS != Status)
				{
					XFsbl_Printf(DEBUG_GENERAL,
							"XFSBL_ERROR_UNAVAILABLE_CPU\n\r");
					Status = XFSBL_ERROR_UNAVAILABLE_CPU;
					goto END;
				}

				/**
				 * Check for power status of the cpu
				 * Update the IVT
				 * Take cpu out of reset
				 */
				Status = XFsbl_SetCpuPwrSettings(
						CpuSettings, XFSBL_CPU_POWER_UP);
				if (XFSBL_SUCCESS != Status)
				{
					XFsbl_Printf(DEBUG_GENERAL,"Power Up "
							"Cpu 0x%0lx failed \n\r", CpuId);

					XFsbl_Printf(DEBUG_GENERAL,
							"XFSBL_ERROR_PWR_UP_CPU\n\r");
					Status = XFSBL_ERROR_PWR_UP_CPU;
					goto END;
				}

				/**
				 * Read the handoff address from structure
				 */
				HandoffAddress = (u64 )
					FsblInstancePtr->HandoffValues[CpuIndex].HandoffAddress;

				/**
				 * Update the handoff address at reset vector address
				 */
				XFsbl_UpdateResetVector(HandoffAddress, CpuSettings,
						OTHER_CPU_HANDOFF,
						XFsbl_GetVectorLocation(PartitionHeader) >>
							XIH_ATTRB_VECTOR_LOCATION_SHIFT);

				XFsbl_Printf(DEBUG_INFO,"CPU 0x%0lx reset release, "
						"Exec State 0x%0lx, HandoffAddress: %0lx\n\r",
						CpuId, ExecState, (PTRSIZE )HandoffAddress);

				/**
				 * Take CPU out of reset
				 */
				Status = XFsbl_SetCpuPwrSettings(
						CpuSettings, XFSBL_CPU_SWRST);
				if (XFSBL_SUCCESS != Status)
				{
					goto END;
				}
			} else {
				/**
				 * Update the running cpu handoff address
				 */
				if(RunningCpuHandoffAddressPresent == FALSE) {
					RunningCpuHandoffAddressPresent = TRUE;
				}
				RunningCpuHandoffAddress = (u64 )
				FsblInstancePtr->HandoffValues[CpuIndex].HandoffAddress;
				RunningCpuExecState = ExecState;

				/**
				 * Update reset vector address for
				 * - FSBL running on A53-0 (64bit), handoff to A53-0 (32 bit)
				 * - FSBL running on A53-0 (32bit), handoff to A53-0 (64 bit)
				 */
				if ((FsblInstancePtr->A53ExecState ==
						XIH_PH_ATTRB_A53_EXEC_ST_AA64) &&
						(ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)) {
					Status = XFSBL_ERROR_UNSUPPORTED_HANDOFF;
					XFsbl_Printf(DEBUG_GENERAL,
						"XFSBL_ERROR_UNSUPPORTED_HANDOFF : A53-0 64 bit to 32 bit\n\r");
					goto END;
				}
				else if ((FsblInstancePtr->A53ExecState ==
						XIH_PH_ATTRB_A53_EXEC_ST_AA32) &&
						(ExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA64)) {
					Status = XFSBL_ERROR_UNSUPPORTED_HANDOFF;
					XFsbl_Printf(DEBUG_GENERAL,
						"XFSBL_ERROR_UNSUPPORTED_HANDOFF : A53-0 32 bit to 64 bit\n\r");
					goto END;
				}
				else
				{
					/* for MISRA C compliance */
				}
			}
		}

		if ((EarlyHandoff == TRUE) && (CpuNeedsEarlyHandoff == TRUE)){

			/* Enable cache again as we will continue loading partitions */
			Xil_DCacheEnable();

			if (PartitionNum <
					(FsblInstancePtr->
							ImageHeader.ImageHeaderTable.NoOfPartitions-1U)) {
				/**
				 * If this is not the last handoff CPU, return back and continue
				 * loading remaining partitions in stage 3
				 */
				CpuIndexEarlyHandoff++;
				Status = XFSBL_STATUS_CONTINUE_PARTITION_LOAD;
			}
			else {
				/**
				 * Early handoff to all required CPUs is done, continue with
				 * regular handoff for remaining applications, as applicable
				 */
				Status = XFSBL_STATUS_CONTINUE_OTHER_HANDOFF;
			}
			goto END;
		}

		/**
		 * Go to the next cpu
		 */
		CpuIndex++;
		CpuIndexEarlyHandoff++;
	}



#ifdef ARMR5

	/**
	 * Remove the R5 vectors from TCM and load APP data
	 * if present
	 */

	if (TcmSkipLength != 0U) {
		/* Restore R5LovecBuffer to LOVEC
		 * This will store partitions vectors to LOVEC
		 * TcmSkipAddress is always 0x0,TcmSkipLength is 32.
		 */
		(void)XFsbl_MemCpy((u8*)TcmSkipAddress,(u8*)R5LovecBuffer,TcmSkipLength);
		XFsbl_Printf(DEBUG_DETAILED,"XFsbl_Handoff:Restored R5LovecBuffer to LOVEC for R5.\n\r");

	}
#endif


	/**
	 * Mark Error status with Fsbl completed
	 */
	XFsbl_Out32(XFSBL_ERROR_STATUS_REGISTER_OFFSET, XFSBL_COMPLETED);

#ifdef XFSBL_WDT_PRESENT
	/* Stop WDT as we are exiting FSBL */
	XFsbl_StopWdt();
#endif

	/**
	 * call to the handoff routine
	 * which will never return
	 */
	if (RunningCpuHandoffAddressPresent ==  TRUE)
	{
		XFsbl_Printf(DEBUG_INFO,
		    "Running Cpu Handoff address: 0x%0lx, Exec State: %0lx\n\r",
		     (PTRSIZE )RunningCpuHandoffAddress, RunningCpuExecState);
		if (RunningCpuExecState == XIH_PH_ATTRB_A53_EXEC_ST_AA32)
		{
			XFsbl_HandoffExit(RunningCpuHandoffAddress,
			    XFSBL_HANDOFFEXIT_32);
		} else {
			XFsbl_HandoffExit(RunningCpuHandoffAddress,
			    XFSBL_HANDOFFEXIT);
		}
	} else {
		XFsbl_HandoffExit(0U, XFSBL_NO_HANDOFFEXIT);
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * This function determines if the given CPU needs early handoff or not.
 * Currently early handoff is provided for R5
 *
 * @param	CpuId is Mask of CPU Id in partition attributes
 *
 * @return	TRUE if this CPU needs early handoff, and FALSE if not
 *
 *****************************************************************************/
static u32 XFsbl_CheckEarlyHandoffCpu(u32 CpuId)
{
	u32 CpuNeedEarlyHandoff = FALSE;
#if defined(XFSBL_EARLY_HANDOFF)
	if ((CpuId == XIH_PH_ATTRB_DEST_CPU_R5_0) ||
			(CpuId == XIH_PH_ATTRB_DEST_CPU_R5_1) ||
			(CpuId == XIH_PH_ATTRB_DEST_CPU_R5_L))
	{
		CpuNeedEarlyHandoff = TRUE;
	}
#endif
	return CpuNeedEarlyHandoff;

}

/*****************************************************************************/
/**
 * This function determines if the given partition needs early handoff
 *
 * @param	FsblInstancePtr is pointer to the XFsbl Instance
 *
 * @param	PartitionNum is the partition number of the image
 *
 * @return	TRUE if this partitions needs early handoff, and FALSE if not
 *
 *****************************************************************************/
u32 XFsbl_CheckEarlyHandoff(XFsblPs * FsblInstancePtr, u32 PartitionNum)
{

	u32 Status = FALSE;
#if defined(XFSBL_EARLY_HANDOFF)
	u32 CpuNeedsEarlyHandoff = FALSE;
	u32 DestinationCpu = 0;
	u32 DestinationDev = 0;
	u32 DestinationCpuNxt = 0;
	u32 DestinationDevNxt = 0;

	DestinationCpu = XFsbl_GetDestinationCpu(
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);
	DestinationDev = XFsbl_GetDestinationDevice(
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum]);
	if ((DestinationCpu == XIH_PH_ATTRB_DEST_CPU_NONE) &&
			((DestinationDev == XIH_PH_ATTRB_DEST_DEVICE_PS) ||
					(DestinationDev == XIH_PH_ATTRB_DEST_DEVICE_NONE)))
	{
		/* If dest device is not PS, retain the dest CPU as NONE/0 */
		DestinationCpu = FsblInstancePtr->ProcessorID;
	}

	if ((PartitionNum + 1) <=
		(FsblInstancePtr->ImageHeader.ImageHeaderTable.NoOfPartitions-1U)) {

		DestinationCpuNxt = XFsbl_GetDestinationCpu(
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum + 1]);
		DestinationDevNxt = XFsbl_GetDestinationDevice(
			&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum + 1]);

		if ((DestinationCpuNxt == XIH_PH_ATTRB_DEST_CPU_NONE) &&
				((DestinationDevNxt == XIH_PH_ATTRB_DEST_DEVICE_PS) ||
						(DestinationDevNxt == XIH_PH_ATTRB_DEST_DEVICE_NONE))) {
			DestinationCpuNxt = FsblInstancePtr->ProcessorID;
		}
	}

	/**
	 *  Early handoff needed if destination CPU needs early handoff AND
	 *  if handoff CPU is not same as running CPU AND
	 *  if this is the last partition of this application
	 */
	CpuNeedsEarlyHandoff = XFsbl_CheckEarlyHandoffCpu(DestinationCpu);
	if ((CpuNeedsEarlyHandoff == TRUE) &&
			(DestinationCpu != FsblInstancePtr->ProcessorID) &&
			(DestinationCpuNxt != DestinationCpu)) {

		Status = TRUE;
	}
#endif
	return Status;
}

/****************************************************************************/
/**
*
* @param
*
* @return
*
* @note
*
*
*****************************************************************************/
static u32 XFsbl_ProtectionConfig(void)
{
	u32 CfgRegVal1;
	u32 CfgRegVal3;
	u32 Status;
	/* Disable Tamper responses*/
	CfgRegVal1 = XFsbl_In32(XFSBL_PS_SYSMON_CONFIGREG1);
	CfgRegVal3 = XFsbl_In32(XFSBL_PS_SYSMON_CONFIGREG3);

	XFsbl_Out32(XFSBL_PS_SYSMON_CONFIGREG1,CfgRegVal1 | XFSBL_PS_SYSMON_CFGREG1_ALRM_DISBL_MASK);
	XFsbl_Out32(XFSBL_PS_SYSMON_CONFIGREG3, CfgRegVal3 | XFSBL_PS_SYSMON_CFGREG3_ALRM_DISBL_MASK);

	/* FSBL shall bypass XPPU and FPD XMPU configuration BY DEFAULT.
	*  This means though the Isolation configuration through hdf is used throughout the
	*  software flow, for the hardware, isolation will only be limited to just OCM.
	*/
#ifdef XFSBL_PROT_BYPASS
	psu_apply_master_tz();
	psu_ocm_protection();
#else
	/* Apply protection configuration */
	Status = (u32)psu_protection();
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_PROTECTION_CFG;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PROTECTION_CFG\r\n");
		goto END;
	}

	/* Lock XMPU/XPPU for further access */
	Status = (u32)psu_protection_lock();
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_PROTECTION_CFG;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_PROTECTION_CFG\r\n");
		goto END;
	}
#endif

	/*Enable Tamper responses*/

	XFsbl_Out32(XFSBL_PS_SYSMON_CONFIGREG1, CfgRegVal1);
	XFsbl_Out32(XFSBL_PS_SYSMON_CONFIGREG3, CfgRegVal3);
	Status = XFSBL_SUCCESS;
	goto END;

END:
	return Status;
}
