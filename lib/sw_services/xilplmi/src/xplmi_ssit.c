/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_ssit.h"
#include "xplmi_hw.h"
#include "pmc_global.h"
#include "sleep.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Sync Master command execution.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of SSIT Sync Master command
 *
 *****************************************************************************/
int XPlmi_SsitSyncMaster(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	/* Initiate synchronization to master */
	XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK,
		PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK);

	/* Wait for Master SLR to reach synchronization point */
	Status = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	while ((Status & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) !=
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {
		/* Check if there is an error from Master SLR */
		if ((Status & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) ==
				PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) {
			XPlmi_Printf(DEBUG_GENERAL, "Received error from Master SLR\n\r");
			XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
				PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);

			Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_MASTER_SYNC, Status);
			goto END;
		}
		Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	}

	/* Complete synchronization from slave */
	XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK, 0U);

	Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	while ((Status & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) ==
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {
		Status = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);

		/* Clear existing status to know the actual status from Master SLR */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK);
	}

	XPlmi_Printf(DEBUG_DETAILED, "SSIT Sync Master successful\n\r");
	Status = XST_SUCCESS;
END:
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
int XPlmi_SsitSyncSlaves(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	u32 ErrorStatus = 0U;
	u32 PmcErrStatus2;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) &&
		((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) && (TimeOut != 0x0U)) {
		usleep(1U);
		ErrorStatus = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}
		--TimeOut;
	}

	if (((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) && (TimeOut != 0x0U)) {
		XPlmi_Printf(DEBUG_DETAILED, "Acknowledging from master\r\n");
		/* Acknowledge synchronization */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK);

		/* Use 100us for Acknowledge synchronization */
		TimeOut = 100U;
		while (((SlavesReady & SlavesMask) != 0x0U) &&
			((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) &&
			(TimeOut != 0x0U)) {
			usleep(1U);
			ErrorStatus = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
			PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
			if (!(PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK)) {
				SlavesReady &= (~SSIT_SLAVE_0_MASK);
			}
			if (!(PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK)) {
				SlavesReady &= (~SSIT_SLAVE_1_MASK);
			}
			if (!(PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK)) {
				SlavesReady &= (~SSIT_SLAVE_2_MASK);
			}

			/* Clear existing status to know the actual status from Slave SLRs */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
				(SlavesReady << PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_SHIFT));
			--TimeOut;
		}
	}

	/* If error or timeout trigger error out to all slaves */
	if (((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) != 0x0U) || (TimeOut == 0x0U)) {
		XPlmi_Printf(DEBUG_GENERAL, "Received error from Slave SLR or Timed out\r\n");
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	/* De-assert synchronization acknowledgement from master */
	XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK, 0U);
	XPlmi_Printf(DEBUG_DETAILED, "SSIT Sync Slaves successful\n\r");
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
int XPlmi_SsitWaitSlaves(XPlmi_Cmd * Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	volatile u32 ErrorStatus = 0U;
	volatile u32 PmcErrStatus2;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) &&
		(ErrorStatus == 0x0U) && (TimeOut != 0x0U)) {
		usleep(1U);
		ErrorStatus = XPlmi_In32(PMC_GLOBAL_PMC_ERR1_STATUS) &
			PMC_GLOBAL_SSIT_ERR_MASK;
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}

		--TimeOut;
	}

	/* If error or timeout trigger error out to all slaves */
	if ((ErrorStatus != 0x0U) || (TimeOut == 0x0U)) {
		XPlmi_Printf(DEBUG_GENERAL,
			"Received error from Slave SLR or Timed out\r\n");
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_DETAILED, "SSIT Wait Master successful\n\r");
	Status = XST_SUCCESS;
END:
	return Status;
}
