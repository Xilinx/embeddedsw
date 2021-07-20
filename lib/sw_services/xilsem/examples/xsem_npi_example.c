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
 * 0.2   hb    07/20/2021  Added event notification and restructured code
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "sleep.h"
#include "xsem_client_api.h"
#include "xsem_ipi.h"
#include "xsem_gic_setup.h"

#define NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT	(17U)
#define NPI_STATUS_SHA_COMP_SCAN_ERR_MASK	(0x00020000U)
#define NPI_STATUS_FIRST_SCAN_DONE_SHIFT	(9U)
#define NPI_STATUS_FIRST_SCAN_DONE_MASK		(0x00000200U)
#define NPI_STATUS_SCAN_SUSPENDED_SHIFT		(10U)
#define NPI_STATUS_SCAN_SUSPENDED_MASK		(0x00000400U)
#define NPI_STATUS_SCAN_PSCAN_EN_SHIFT		(11U)
#define NPI_STATUS_SCAN_PSCAN_EN_MASK		(0x00000800U)
#define POLL_TIMEOUT						(4U)
#define DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))

static XIpiPsu IpiInst;
static XScuGic GicInst;
XSem_Notifier Notifier = {
		.Module = XSEM_NOTIFY_NPI,
		.Event = XSEM_EVENT_NPI_INT_ERR | XSEM_EVENT_NPI_CRC_ERR,
		.Flag = 1U,
};

/* Global variables to hold the event count when notified */
u8 EventCnt_Crc = 0U;
u8 EventCnt_IntErr = 0U;

/*****************************************************************************
 * @brief	Initialize XilSEM IPI instance to process XilSEM
 * 			notifications from PLM.
 *
 * @return	XST_SUCCESS : upon successful initialization of XilSEM IPI.
 * 			XST_FAILURE : any failure in initialization of XilSEM IPI.
 *
 *****************************************************************************/
XStatus XSem_IpiInitApi (void)
{
	XStatus Status = XST_FAILURE;

	/* GIC Initialize */
	Status = GicSetupInterruptSystem(&GicInst);
	if (Status != XST_SUCCESS) {
		xil_printf("GicSetupInterruptSystem failed with error: %d\r\n",\
						Status);
		goto END;
	}

	Status = IpiInit(&IpiInst, &GicInst);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] IPI Init Error: Status 0x%x\r\n", \
						__func__, Status);
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************
 * @brief	Initializes(if previously uninitialized) and starts NPI scan
 * 			using Client IPI interface.
 *
 * @return	XST_SUCCESS : upon successful completion of NPI start scan.
 * 			XST_FAILURE : any failure in NPI start scan.
 *
 *****************************************************************************/
XStatus XSem_NpiApiStartScan(void)
{
	XStatus Status = XST_FAILURE;
	XSemIpiResp IpiResp = {0};

	/* Start NPI scan */
	Status = XSem_CmdNpiStartScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_STARTSCAN == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Start\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, Ret 0x%x\n\r", \
				__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 * @brief	Stops active NPI scan using Client IPI interface.
 *
 * @return	XST_SUCCESS : upon successful completion of NPI stop scan.
 * 			XST_FAILURE : any failure in NPI stop scan.
 *
 *****************************************************************************/
XStatus XSem_NpiApiStopScan(void)
{
	XStatus Status = XST_FAILURE;
	XSemIpiResp IpiResp = {0};

	/* Stop NPI scan */
	Status = XSem_CmdNpiStopScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_STOPSCAN == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Stop\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Stop Status 0x%x Ack 0x%x, Ret 0x%x\n\r", \
				__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 * @brief	Injects error at golden SHA of first NPI descriptor
 *
 * @return	XST_SUCCESS : upon successful error injection
 * 			XST_FAILURE : any failure in injecting error
 *
 *****************************************************************************/
XStatus XSem_NpiApiErrNjct(void)
{
	XStatus Status = XST_FAILURE;
	XSemIpiResp IpiResp = {0};

	/* Inject error in first used SHA */
	Status = XSem_CmdNpiInjectError(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_ERRINJECT == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Inject\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Inject Status 0x%x Ack 0x%x, Ret 0x%x\n\r", \
				__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 * @brief	Checks if NPI scan count is incremented
 *
 * @return	XST_SUCCESS : Scan count increments as expected
 * 			XST_FAILURE : Scan count does not increment
 *
 *****************************************************************************/
XStatus XSem_ApiCheckScanCount(XSemNpiStatus *NpiStatus)
{
	XStatus Status = XST_FAILURE;
	u32 InitialScanCount = 0U;
	u32 ProgressedScanCount = 0U;

	/* Get NPI scan status */
	Status = XSem_CmdNpiGetStatus(NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
				__func__, Status);
		goto END;
	}

	InitialScanCount = NpiStatus->ScanCnt;

	/* Small delay to ensure NPI scan progresses before second read.
	 * The delay value assumes 100ms as NPI scan interval */
	usleep(120000);

	/* Get NPI status again to update statuses */
	Status = XSem_CmdNpiGetStatus(NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
				__func__, Status);
		goto END;
	}

	ProgressedScanCount = NpiStatus->ScanCnt;

	if (ProgressedScanCount > InitialScanCount) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
		goto END;
	}

END:
	return Status;
}

/*****************************************************************************
 * @brief	Checks if NPI heartbeat count is incremented
 *
 * @return	XST_SUCCESS : Heartbeat count increments as expected
 * 			XST_FAILURE : Scan count does not increment
 *
 *****************************************************************************/
XStatus XSem_ApiCheckHbtCount(XSemNpiStatus *NpiStatus)
{
	XStatus Status = XST_FAILURE;
	u32 InitialHbtCount = 0U;
	u32 ProgressedHbtCount = 0U;

	/* Get NPI scan status */
	Status = XSem_CmdNpiGetStatus(NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
				__func__, Status);
		goto END;
	}

	InitialHbtCount = NpiStatus->HbCnt;

	/* Small delay to ensure NPI scan progresses before second read.
	 * The delay value assumes 100ms as NPI scan interval */
	usleep(120000);

	/* Get NPI status again to update statuses */
	Status = XSem_CmdNpiGetStatus(NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure\n\r", \
				__func__, Status);
		goto END;
	}

	ProgressedHbtCount = NpiStatus->HbCnt;

	if (ProgressedHbtCount > InitialHbtCount) {
		Status = XST_SUCCESS;
	} else {
		Status = XST_FAILURE;
	}

END:
	return Status;
}

/******************************************************************************
 * @brief	This function is used to Register XilSEM even notifications
 *
 * @param[in]	Enable : Enable event notification in XilSEM
 *
 * @return	Status : Success or Failure
 *
 *****************************************************************************/
int XSem_NpiEventRegisterNotifier(u32 Enable)
{
	int Status;

	if (Enable) {
		Notifier.Flag = 1U;
	} else {
		Notifier.Flag = 0U;
	}
	/* In this example all NPI events are enabled
	 * If you want to enable particular event set the Event member in
	 * Notifier structure with corresponding event.
	 */
	Status = XSem_RegisterEvent(&IpiInst, &Notifier);

	return Status;
}

/******************************************************************************
 * @brief	IpiCallback to receive event messages
 *
 * @param[in]	InstancePtr : Pointer to IPI driver instance
 *
 *****************************************************************************/
void XSem_IpiCallback(XIpiPsu *const InstancePtr)
{
	int Status;
	u32 Payload[PAYLOAD_ARG_CNT] = {0};

	Status = XIpiPsu_ReadMessage(&IpiInst, SRC_IPI_MASK, Payload, \
			PAYLOAD_ARG_CNT, XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		xil_printf("ERROR #%d while reading IPI buffer\n", Status);
		return;
	}

	if ((XSEM_EVENT_ERROR == Payload[0]) && \
			(XSEM_NOTIFY_NPI == Payload[1])) {
		if (XSEM_EVENT_NPI_CRC_ERR == Payload[2]) {
			EventCnt_Crc = 1U;
		} else if (XSEM_EVENT_NPI_INT_ERR == Payload[2]) {
			xil_printf("[ALERT] Received internal error event notification"
					" from XilSEM\n\r");
			EventCnt_IntErr = 1U;
		} else {
			xil_printf("%s Some other callback received: %d:%d:%d\n",
					__func__, Payload[0], \
					Payload[1], Payload[2]);
		}
	} else {
		xil_printf("%s Some other callback received: %d\n", \
				__func__, Payload[0]);
	}
}

/******************************************************************************
 * @brief	Verifies and Prints error report
 *
 *****************************************************************************/
void PrintErrReport(void)
{
	XStatus Status = 0U;
	XSemNpiStatus NpiStatus = {0};

	xil_printf("-----------------------------------------------------\n\r");
	xil_printf("-----------------Print Report------------------------\n\r");
	xil_printf("-----------------------------------------------------\n\r");

	/* Check if SHA mismatch error is reported */
	Status = XSem_CmdNpiGetStatus(&NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
				__func__, Status);
	}

	Status = DataMaskShift(NpiStatus.Status,
			NPI_STATUS_SHA_COMP_SCAN_ERR_MASK,
			NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT);

	if (Status == 1U) {
		xil_printf("[SUCCESS] SHA comparison error detected\n\r");
	}

	/* Check Scan counter */
	Status = XSem_ApiCheckScanCount(&NpiStatus);
	if (Status == XST_FAILURE) {
		xil_printf("[SUCCESS] Scan counter not incrementing\n\r");
	} else {
		xil_printf("[ERROR] Scan counter incrementing\n\r");
	}
	/* Check Heartbeat counter */
	Status = XSem_ApiCheckHbtCount(&NpiStatus);
	if (Status == XST_FAILURE) {
		xil_printf("[SUCCESS] Heartbeat counter not incrementing\n\r");
	} else {
		xil_printf("[ERROR] Heartbeat counter incrementing\n\r");
	}

	/* Check event notification */
	if ( (EventCnt_Crc == 1U) && (EventCnt_IntErr == 0U) ) {
		xil_printf("[SUCCESS] Received CRC error event notification\n\r");
		EventCnt_Crc = 0U;
	} else {
		xil_printf("[FAILURE] No CRC error event notification received\n\r");
	}
}

/*****************************************************************************/
/**
*
* @brief	The purpose of this function is to illustrate the usage of
* 			client APIs for NPI scan over IPI.
*
* @return
*
* - XST_SUCCESS - on successful execution of injection and detection of error
* - XST_FAILURE - on failure
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	XStatus Status = XST_FAILURE;
	u32 TempA_32 = 0U;
	u32 TempB_32 = 0U;
	u32 TimeoutCount = 0U;
	XSemNpiStatus NpiStatus = {0};

	Status = XSem_IpiInitApi();
	if (XST_SUCCESS != Status) {
		xil_printf("Ipi init failure with status 0x%x\n\r", \
				__func__, Status);
		goto END;
	}

	XSem_NpiEventRegisterNotifier(1U);

	/* The following sequence demonstrates how to inject errors in NPI
	 * 1. Stop NPI scan
	 * 2. Inject error
	 * 3. Start NPI scan
	 * 4. Read for Golden SHA mismatch error
	 */

	/* Check if NPI is stopped or not started previously */
	Status = XSem_CmdNpiGetStatus(&NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
				__func__, Status);
		goto END;
	}

	TempA_32 = DataMaskShift(NpiStatus.Status, NPI_STATUS_SCAN_PSCAN_EN_MASK,
				NPI_STATUS_SCAN_PSCAN_EN_SHIFT);

	TempB_32 = DataMaskShift(NpiStatus.Status, NPI_STATUS_SCAN_SUSPENDED_MASK,
				NPI_STATUS_SCAN_SUSPENDED_SHIFT);

	/* If either Periodic scan is not enabled or is stopped */
	if ( (TempA_32 == 0U) || (TempB_32 == 1U) ) {
		/* Start NPI scan */
		Status = XSem_NpiApiStartScan();
		if (XST_FAILURE == Status) {
			xil_printf("[%s] ERROR: NPI scan failed to start\n\r.", __func__);
			goto END;
		}
	}

	/* Check scan count to ensure NPI scan is running */
	Status = XSem_ApiCheckScanCount(&NpiStatus);
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI Scan count not incrementing.\n\r", \
				__func__, Status);
		goto END;
	}

	/* Stop NPI scan before injecting error */
	Status = XSem_NpiApiStopScan();
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI scan failed to stop\n\r.", __func__);
		goto END;
	}
	/* Inject error in Golden SHA of first descriptor */
	Status = XSem_NpiApiErrNjct();
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI error inject failed\n\r.", __func__);
		goto END;
	}

	/* Restart NPI scan after injecting error */
	Status = XSem_NpiApiStartScan();
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI scan failed to start\n\r.", __func__);
		goto END;
	}

	/* Wait for XilSEM to detect error */
	TimeoutCount = POLL_TIMEOUT;
	while (TimeoutCount != 0U) {
		Status = XSem_CmdNpiGetStatus(&NpiStatus);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
					__func__, Status);
			goto END;
		}
		/* Read NPI_SCAN_ERROR status bit */
		TempA_32 = DataMaskShift(NpiStatus.Status,
						NPI_STATUS_SHA_COMP_SCAN_ERR_MASK,
						NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT);
		if (TempA_32 == 1) {
			Status = XST_SUCCESS;
			goto END;
		}
		TimeoutCount--;
		/* Small delay before polling again */
		usleep(25000);
	}
	xil_printf("[%s] ERROR: Timeout occurred waiting for error\n\r", __func__);
	Status = XST_FAILURE;
	goto END;

END:
	PrintErrReport();
	return Status;
}
