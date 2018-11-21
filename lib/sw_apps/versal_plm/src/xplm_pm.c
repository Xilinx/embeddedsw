/******************************************************************************
* Copyright (c) 2018 - 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xplm_pm.c
*
* This file contains the wrapper code xilpm
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/20/2018 Initial release
* 1.01  rp   08/08/2019 Added code to send PM notify callback through IPI
* 1.02  kc   03/23/2020 Minor code cleanup
* 1.03  kc   08/04/2020 Initialized IpiMask to zero for PMC CDO commands
*       kc   08/04/2020 Added default NPLL configuration for master SLR devices
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplm_pm.h"
#include "xplm_default.h"
#include "xpm_api.h"
#include "xpm_subsystem.h"
#include "xplmi_util.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/
/*
 * NPLL CFG params
 * LOCK_DLY[31:25]=0x3f, LOCK_CNT[22:13]=0x2EE, LFHF[11:10]=0x3,
 * CP[8:5]=0x3, RES[3:0]=0x5
 */
#define XPLM_NOCPLL_CFG_VAL		(0x7E5DCC65U)

/*
 * NPLL CTRL params
 * POST_SRC[26:24]=0x0, PRE_SRC[22:20]=0x0, CLKOUTDIV[17:16]=0x3,
 * FBDIV[15:8]=0x48, BYPASS[3]=0x1, RESET[0]=0x1
 */
#define XPLM_NOCPLL_CTRL_VAL		(0x34809U)
#define NOCPLL_TIMEOUT			(100000U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
* @brief This function is registered with XilPm to send any data to IPI master
* when a event comes.
*
* @param	IpiMask IPI master ID
* @param	EventId Id of the event as defined in XilPm
* @param	Payload is pointer to the Data that needs to be sent to the IPI
* 		master
*
* @return	None
*
*****************************************************************************/
static void XPlm_PmRequestCb(const u32 IpiMask, const XPmApiCbId_t EventId, u32 *Payload)
{
#ifdef XPAR_XIPIPSU_0_DEVICE_ID
	XStatus Status = XST_FAILURE;

	if ((PM_INIT_SUSPEND_CB == EventId) || (PM_NOTIFY_CB == EventId)) {
		Status = XPlmi_IpiWrite(IpiMask, Payload, XPLMI_CMD_RESP_SIZE,
					XIPIPSU_BUF_TYPE_MSG);
		if (XST_SUCCESS != Status) {
			XPlmi_Printf(DEBUG_GENERAL,
			 "%s Error in IPI write: %d\r\n", __func__, Status);
		} else {
			Status = XPlmi_IpiTrigger(IpiMask);
			if (XST_SUCCESS != Status) {
				XPlmi_Printf(DEBUG_GENERAL,
				"%s Error in IPI trigger: %d\r\n", __func__, Status);
			}
		}
	} else {
		XPlmi_Printf(DEBUG_GENERAL,
		 "%s Error: Unsupported EventId: %d\r\n", __func__, EventId);
	}
#else
	XPlmi_Printf(DEBUG_GENERAL, "%s Error: IPI is not defined\r\n", __func__);
#endif /* XPAR_XIPIPSU_0_DEVICE_ID */
}

/*****************************************************************************/
/**
* @brief It calls the XilPm initialization API to initialize its structures.
*
* @param	None
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_PmInit(void)
{
	int Status = XST_FAILURE;

	/**
	 * Initialize the XilPm component. It registers the callback handlers,
	 * variables, events
	 */
	Status = XPm_Init(XPlm_PmRequestCb, &XLoader_RestartImage);
	if (Status != XST_SUCCESS)
	{
		Status = XPlmi_UpdateStatus(XPLM_ERR_PM_MOD, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief This function configures the NPLL equal to slave SLR ROM NPLL
*        frequency. It is only required for master SLR devices.
*
* @param	None
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
static int XPlm_ConfigureDefaultNPll(void)
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
			NOCPLL_TIMEOUT);
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
* @brief This function executes the PLM CDO present in PMC RAM.
*
* @param	Arg Not used in the function
*
* @return	Status as defined in xplmi_status.h
*
*****************************************************************************/
int XPlm_ProcessPlmCdo(void *Arg)
{
	int Status = XST_FAILURE;
	XPlmiCdo Cdo;
	u32 SlrType;

	XPlmi_Printf(DEBUG_DETAILED, "%s\n\r", __func__);
	(void )Arg;

	/**
	 * Configure NoC frequency equivalent to the frequency ROM sets in
	 * Slave devices
	 */
	SlrType = XPlmi_In32(PMC_TAP_SLR_TYPE) &
			PMC_TAP_SLR_TYPE_VAL_MASK;
	if (SlrType == XLOADER_SSIT_MASTER_SLR) {
		Status = XPlm_ConfigureDefaultNPll();
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/**
	 *  Pass the PLM CDO to CDO parser, PLM CDO contains
	 *  - Device topology
	 *  - PMC block configuration
	 */

	/** Process the PLM CDO */
	Status = XPlmi_InitCdo(&Cdo);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	Cdo.ImgId = PM_SUBSYS_PMC;
	Cdo.IpiMask = 0U;
	Cdo.PrtnId = 0U;
	Cdo.BufPtr = (u32 *)XPLMI_PMCRAM_BASEADDR;
	Cdo.BufLen = XPLMI_PMCRAM_LEN;
	Status = XPlmi_ProcessCdo(&Cdo);

END:
	return Status;
}
