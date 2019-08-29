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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_ssit.h"
#include "pmc_global.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief This function provides SSIT Sync Master command execution
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of SSIT Sync Master command
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

			Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_SSIT_MASTER_SYNC, Status);
			goto END;
		}
		Status = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	}

	/* Complete synchronization from slave */
	XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK, 0U);

	Status = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
	while ((Status & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) ==
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {

		Status = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);

		/* Clear existing status to know the actual status from Master SLR */
		Xil_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK);
	}

	XPlmi_Printf(DEBUG_DETAILED, "SSIT Sync Master successful\n\r");
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides SSIT Sync Slaves command execution
 *  Command payload parameters are
 *	* Slaves Mask
 *	* Timeout
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of SSIT Sync Slaves command

 *
 *****************************************************************************/
int XPlmi_SsitSyncSlaves(XPlmi_Cmd * Cmd)
{
	u32 SlavesMask = Cmd->Payload[0];
	u32 TimeOut = Cmd->Payload[1];
	u32 SlavesReady = 0;
	u32 ErrorStatus = 0;
	u32 PmcErrStatus2;
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) &&
		((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) && (TimeOut != 0x0U)) {

		ErrorStatus = Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
		PmcErrStatus2 = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
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

		TimeOut = 1000;
		while (((SlavesReady & SlavesMask) != 0x0U) &&
			((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) && (TimeOut != 0x0U)) {
			ErrorStatus = Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
			PmcErrStatus2 = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
			if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) {
				SlavesReady &= (~SSIT_SLAVE_0_MASK);
			}
			if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) {
				SlavesReady &= (~SSIT_SLAVE_1_MASK);
			}
			if (PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) {
				SlavesReady &= (~SSIT_SLAVE_2_MASK);
			}

			/* Clear exiting status to know the actual status from Slave SLRs */
			Xil_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
				(SlavesReady << PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_SHIFT));
			--TimeOut;
		}
	}

	/* If error or timeout trigger error out to all slaves */
	if (((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) != 0x0U) || (TimeOut == 0x0U)) {
		XPlmi_Printf(DEBUG_GENERAL, "Received error from Slave SLR or Timed out\r\n");
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	/* De-assert synchronization acknowledgement from master */
	XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK,
			0x0U);
	XPlmi_Printf(DEBUG_DETAILED, "SSIT Sync Slaves successful\n\r");
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief This function provides SSIT Wait Slaves command execution
 *  Command payload parameters are
 *	* Slaves Mask
 *	* Timeout
 *
 * @param Pointer to the command structure
 *
 * @return Returns the Status of SSIT Wait Slaves command
 *
 *****************************************************************************/
int XPlmi_SsitWaitSlaves(XPlmi_Cmd * Cmd)
{
	u32 SlavesMask = Cmd->Payload[0];
	u32 TimeOut = Cmd->Payload[1];
	u32 SlavesReady = 0;
	u32 ErrorStatus = 0;
	u32 PmcErrStatus2;
	int Status = XST_FAILURE;

	XPlmi_Printf(DEBUG_DETAILED, "%s %p\n\r", __func__, Cmd);

	/* Wait until all Slaves initiate synchronization point */
	while (((SlavesReady & SlavesMask) != SlavesMask) &&
		((ErrorStatus & PMC_GLOBAL_SSIT_ERR_MASK) == 0x0U) && (TimeOut != 0x0U)) {

		ErrorStatus = Xil_In32(PMC_GLOBAL_PMC_ERR1_STATUS);
		PmcErrStatus2 = Xil_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
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
		XPlmi_Printf(DEBUG_GENERAL, "Received error from Slave SLR or Timed out\r\n");
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		Status = XPLMI_UPDATE_STATUS(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_DETAILED, "SSIT Wait Master successful\n\r");
	Status = XST_SUCCESS;
END:
	return Status;
}
