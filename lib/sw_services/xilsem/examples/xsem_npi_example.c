/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_npi_example.c
 *
 * This file has XilSEM NPI SHA error injection example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   rb    03/16/2021  Initial Creation
 * </pre>
 *
 *****************************************************************************/

/****************************** Include Files ********************************/
#include "sleep.h"
#include "xsem_client_api.h"

/*************************** Constant Definitions ****************************/

/***************************** Type Definitions ******************************/

/****************** Macros (Inline Functions) Definitions ********************/

#define TARGET_IPI_INT_DEVICE_ID	(XPAR_XIPIPSU_0_DEVICE_ID)

#define NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT	(17U)
#define NPI_STATUS_SHA_COMP_SCAN_ERR_MASK	(0x00020000U)
#define NPI_STATUS_FIRST_SCAN_DONE_SHIFT	(9U)
#define NPI_STATUS_FIRST_SCAN_DONE_MASK		(0x00000200U)
#define POLL_TIMEOUT				(4U)

/*************************** Variable Definitions ****************************/

/*************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
* @brief	The purpose of this function is to illustrate the usage of
* 		client APIs for NPI scan over IPI.
*
* @return	XST_SUCCESS - On successful execution
*		XST_FAILURE - On failure
*
* @note		This example will work for immediate start-up configuration.
*
******************************************************************************/
int main(void)
{
	XStatus Status = XST_FAILURE;
	XSemIpiResp IpiResp = {0U};
	XIpiPsu_Config *IpiCfgPtr = NULL;
	XIpiPsu IpiInst;
	u32 Value = 0U;
	u32 TimeoutCount = 0U;
	XSemNpiStatus NpiStatus = {0U};

	/* Load Config for Processor IPI Channel */
	IpiCfgPtr = XIpiPsu_LookupConfig(TARGET_IPI_INT_DEVICE_ID);
	if (NULL == IpiCfgPtr) {
		xil_printf("[%s] ERROR: IPI LookupConfig failed\n\r", \
				__func__, Status);
		goto END;
	}

	/* Initialize the Instance pointer */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
			IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: IPI instance initialize failed\n\r", \
				__func__, Status);
		goto END;
	}

	/* Wait till first NPI scan is done */
	TimeoutCount = POLL_TIMEOUT;
	while (TimeoutCount != 0U) {
		Status = XSem_CmdNpiGetStatus(&NpiStatus);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
					__func__, Status);
			goto END;
		}
		/* Read FIRST_SCAN_DONE status bit */
		Value = ((NpiStatus.Status & NPI_STATUS_FIRST_SCAN_DONE_MASK) >>
				NPI_STATUS_FIRST_SCAN_DONE_SHIFT);
		if (Value == 1) {
			goto STATUS_POLL_SUCCESS;
		}
		TimeoutCount--;
		/* Small delay before polling again */
		usleep(25000);
	}
	xil_printf("[%s] ERROR: Timeout occurred or NPI scan not" \
			" started\n\r", __func__);
	Status = XST_FAILURE;
	goto END;

STATUS_POLL_SUCCESS:

	/* Stop NPI scan */
	Status = XSem_CmdNpiStopScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_STOPSCAN == IpiResp.RespMsg1)) {
		if (XST_FAILURE == IpiResp.RespMsg2) {
			xil_printf("[%s] Error: Stop Status 0x%x Ack 0x%x, "
					"Ret 0x%x\n\r", __func__, Status,
					IpiResp.RespMsg1, IpiResp.RespMsg2);
			Status = XST_FAILURE;
			goto END;
		} else {
			xil_printf("[%s] Success: NPI Scan Stop\n\r",
					__func__);
		}
	} else {
		xil_printf("[%s] Error: Stop Status 0x%x Ack 0x%x, "
				"Ret 0x%x\n\r", __func__, Status,
				IpiResp.RespMsg1, IpiResp.RespMsg2);
		goto END;
	}

	/* Inject error in first used SHA */
	Status = XSem_CmdNpiInjectError(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_ERRINJECT == IpiResp.RespMsg1)) {
		if (XST_FAILURE == IpiResp.RespMsg2) {
			xil_printf("[%s] Error: Inject Status 0x%x Ack 0x%x, "
					"Ret 0x%x\n\r", __func__, Status,
					IpiResp.RespMsg1, IpiResp.RespMsg2);
			Status = XST_FAILURE;
			goto END;
		} else {
			xil_printf("[%s] Success: NPI Error Inject\n\r",
					__func__);
		}
	} else {
		xil_printf("[%s] Error: Inject Status 0x%x Ack 0x%x, "
				"Ret 0x%x\n\r", __func__, Status,
				IpiResp.RespMsg1, IpiResp.RespMsg2);
		goto END;
	}

	/* Start NPI scan */
	Status = XSem_CmdNpiStartScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_STARTSCAN == IpiResp.RespMsg1)) {
		if (XST_FAILURE == IpiResp.RespMsg2) {
			xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, "
					"Ret 0x%x\n\r", __func__, Status,
					IpiResp.RespMsg1, IpiResp.RespMsg2);
			Status = XST_FAILURE;
			goto END;
		} else {
			xil_printf("[%s] Success: NPI Scan Start\n\r",
					__func__);
		}
	} else {
		xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, "
				"Ret 0x%x\n\r", __func__, Status,
				IpiResp.RespMsg1, IpiResp.RespMsg2);
		goto END;
	}

	/* Wait for XilSEM to detect error */
	TimeoutCount = POLL_TIMEOUT;
	while (TimeoutCount != 0U) {
		Status = XSem_CmdNpiGetStatus(&NpiStatus);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
					__func__, Status);
			goto END;
		}
		/* Read NPI_SCAN_ERROR status bit */
		Value = ((NpiStatus.Status & NPI_STATUS_SHA_COMP_SCAN_ERR_MASK) >>
				NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT);
		if (Value == 1U) {
			xil_printf("[%s] Success: NPI Scan Error detected\n\r",
					__func__);
			goto END;
		}
		TimeoutCount--;
		/* Small delay before polling again */
		usleep(25000);
	}
	xil_printf("[%s] ERROR: Timeout occurred waiting for error\n\r",
			__func__);
	Status = XST_FAILURE;
	goto END;

END:
	return Status;
}
