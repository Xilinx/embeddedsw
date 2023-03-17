/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc. All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplm_plat.c
*
* This file contains the PLMI versal platform specific code.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       ma   07/29/2022 Replaced XPAR_XIPIPSU_0_DEVICE_ID macro with
*                       XPLMI_IPI_DEVICE_ID
* 1.01  ng   11/11/2022 Fixed doxygen file name error
*			 03/16/2023 Misra-C violation Rule 8.4 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xplmi.h"
#include "xplm_plat.h"

/************************** Constant Definitions *****************************/
/**
 * XPLM_NOCPLL_CFG_VAL    NPLL CFG params
 * LOCK_DLY[31:25]=0x3f, LOCK_CNT[22:13]=0x2EE, LFHF[11:10]=0x3,
 * CP[8:5]=0x3, RES[3:0]=0x5
 */
#define XPLM_NOCPLL_CFG_VAL		(0x7E5DCC65U)

/**
 * @{
 * XPLM_NOCPLL_CTRL_VAL    NPLL CTRL params
 * POST_SRC[26:24]=0x0, PRE_SRC[22:20]=0x0, CLKOUTDIV[17:16]=0x3,
 * FBDIV[15:8]=0x48, BYPASS[3]=0x1, RESET[0]=0x1
 *
 */
#define XPLM_NOCPLL_CTRL_VAL		(0x34809U)
#define NOCPLL_TIMEOUT			(100000U)
/** @} */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function configures the NPLL equal to slave SLR ROM NPLL
*        frequency. It is only required for master SLR devices.
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_ConfigureDefaultNPll(void)
{
	int Status = XST_FAILURE;

	/* Set the PLL helper Data */
	Xil_Out32(CRP_NOCPLL_CFG, XPLM_NOCPLL_CFG_VAL);

	/* Set the PLL Basic Controls */
	Xil_Out32(CRP_NOCPLL_CTRL, XPLM_NOCPLL_CTRL_VAL);

	/* De-assert the PLL Reset; PLL is still in bypass mode only */
	XPlmi_UtilRMW(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_RESET_MASK, 0x0U);

	/* Check for NPLL lock */
	Status = XPlmi_UtilPoll(CRP_PLL_STATUS,
			CRP_PLL_STATUS_NOCPLL_LOCK_MASK,
			CRP_PLL_STATUS_NOCPLL_LOCK_MASK,
			NOCPLL_TIMEOUT, NULL);
	if (Status != XST_SUCCESS) {
		Status = XPlmi_UpdateStatus(XPLM_ERR_NPLL_LOCK, 0);
		goto END;
	}

	/* Release the bypass mode */
	XPlmi_UtilRMW(CRP_NOCPLL_CTRL, CRP_NOCPLL_CTRL_BYPASS_MASK, 0x0U);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function enables SLVERR for PMC related modules
*
* @return	None
*
*****************************************************************************/
void XPlm_EnablePlatformSlaveErrors(void)
{
	/* Enable SLVERR for INTPMC_CONFIG registers */
	XPlmi_Out32(INTPMC_CONFIG_IR_ENABLE, XPLMI_SLAVE_ERROR_ENABLE_MASK);
}

#ifdef XPLMI_IPI_DEVICE_ID
/*****************************************************************************/
/**
* @brief	This function updates the keep alive status variable
*
* @param	Val to set the status as started or not started or error
*
* @return	PsmKeepAliveStatus
*
*****************************************************************************/
u32 XPlm_SetPsmAliveStsVal(u32 Val)
{
	static u32 PsmKeepAliveStatus = XPLM_PSM_ALIVE_NOT_STARTED;

	if(Val != XPLM_PSM_ALIVE_RETURN) {
		/* Update the Keep Alive Status */
		PsmKeepAliveStatus = Val;
	}

	return PsmKeepAliveStatus;
}

/*****************************************************************************/
/**
* @brief	This function updates the counter value
*
* @param	Val to Increment or Clear the CounterVal variable
*
* @return	CounterVal
*
*****************************************************************************/
u32 XPlm_UpdatePsmCounterVal(u32 Val)
{
	static u32 CounterVal = 0U;

	if(Val == XPLM_PSM_COUNTER_INCREMENT) {
		/* Increment the counter value */
		CounterVal++;
	}else if(Val == XPLM_PSM_COUNTER_CLEAR){
		/* Clear the counter value */
		CounterVal = 0U;
	} else{
		/* To avoid Misra-C violation  */
	}

	return CounterVal;
}
#endif /* XPLMI_IPI_DEVICE_ID */
