/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_ssit.c
*
* This file contains the SSIT related CDO commands
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   08/13/2019 Initial release
*       ma   08/24/2019 Added SSIT commands
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
* 		td   10/19/2020 MISRA C Fixes
* 1.03  ma   12/17/2021 Do not check for SSIT errors during synchronization
* 1.04  tnt  01/10/2022 Update ssit_sync* code to wait for each slave mask
*       bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_ssit.h"
#include "xplmi_hw.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static u32 XPlmi_SsitGetSlaveErrorMask(void);

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function checks which Slave SLR this code is running on and
 * 	returns corresponding SSIT mask of PMC GLOBAL PMC_ERROR2_STATUS register
 *
 * @return	return 0 if error occured. Else return one of the ERR_MASK.
 *
 *****************************************************************************/
static u32 XPlmi_SsitGetSlaveErrorMask(void)
{
	u32 SlrErrMask = 0U;
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	switch (SlrType){
	case SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_TOP:
	case SLR_TYPE_SSIT_DEV_SLAVE_1_SLR_NTOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK;
		break;
	case SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_TOP:
	case SLR_TYPE_SSIT_DEV_SLAVE_2_SLR_NTOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK;
		break;
	case SLR_TYPE_SSIT_DEV_SLAVE_3_SLR_TOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK;
		break;
	default:
		break;
	}

	return SlrErrMask;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Sync Master command execution.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of SSIT Sync Master command
 *
 *****************************************************************************/
int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ErrStatus = (u32)XST_FAILURE;
	u32 SlrErrMask = XPlmi_SsitGetSlaveErrorMask();

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);
	if (0U != SlrErrMask){
		/* Initiate synchronization to master */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK);

		/* Wait for Master SLR to reach synchronization point */
		ErrStatus = Xil_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		while ((ErrStatus & SlrErrMask) != SlrErrMask) {
			ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		}

		/* Complete synchronization from slave */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK, 0U);
		ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		while ((ErrStatus & SlrErrMask) == SlrErrMask) {
			ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
			/* Clear existing status to know the actual status from Master SLR */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, SlrErrMask);
		}

		XPlmi_Printf(DEBUG_INFO, "SSIT Sync Master successful\n\r");
		Status = XST_SUCCESS;
	} else {
		XPlmi_Printf(DEBUG_GENERAL, "SSIT sync master error. Unknown SLR type.\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Sync Slaves command execution.
 *  		Command payload parameters are
 *				* Slaves Mask
 *				* Timeout
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of SSIT Sync Slaves command
 *
 *****************************************************************************/
int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	u32 PmcErrStatus2;

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) && (TimeOut != 0x0U)) {
		usleep(1U);
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}
		--TimeOut;
	}

	if (0x0U != TimeOut) {
		XPlmi_Printf(DEBUG_INFO, "Acknowledging from master\r\n");
		/* Acknowledge synchronization */
		XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, SlavesMask);

		/* Use 100us for Acknowledge synchronization */
		TimeOut = 100U;
		while (((SlavesReady & SlavesMask) != 0x0U) && (TimeOut != 0x0U)) {
			usleep(1U);
			PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_0_MASK);
			}
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_1_MASK);
			}
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_2_MASK);
			}

			/* Clear existing status to know the actual status from Slave SLRs */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
				(SlavesReady << PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_SHIFT));
			--TimeOut;
		}
	}

	/* If timeout, set status */
	if (0x0U == TimeOut) {
		XPlmi_Printf(DEBUG_GENERAL, "Received error from Slave SLR or Timed out\r\n");
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	/* De-assert synchronization acknowledgement from master */
	XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, 0);
	XPlmi_Printf(DEBUG_INFO, "SSIT Sync Slaves successful\n\r");
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Wait Slaves command execution.
 * 			 Command payload parameters are
 *				* Slaves Mask
 *				* Timeout
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of SSIT Wait Slaves command
 *
 *****************************************************************************/
int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	volatile u32 PmcErrStatus2;

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);

	/* Clear any existing SSIT errors in PMC_ERR1_STATUS register */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, PMC_GLOBAL_SSIT_ERR_MASK);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) && (TimeOut != 0x0U)) {
		usleep(1U);
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}

		--TimeOut;
	}

	/* If timeout occurred, set status */
	if (TimeOut == 0x0U) {
		XPlmi_Printf(DEBUG_GENERAL,
			"Received error from Slave SLR or Timed out\r\n");
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "SSIT Wait Master successful\n\r");
	Status = XST_SUCCESS;
END:
	return Status;
}
