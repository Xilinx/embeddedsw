/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 0.3   hb    09/20/2021  Added description on error injection &
 *                         correction
 * 0.4   hb    01/06/2022  Added feature to print golden SHA values
 * 0.5   hb    01/27/2022  Added event for NPI scan self diagnosis
 * 0.6   hb    07/28/2022  Updated example with descriptor attribute
 *                         interpretation
 * 0.7   gayu  05/19/2023  Added Test print summary and updated copyright
 *                         information
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "sleep.h"
#include "xsem_client_api.h"
#include "xsem_ipi.h"
#include "xsem_gic_setup.h"
#include "xil_cache.h"

#define NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT	(17U)
#define NPI_STATUS_SHA_COMP_SCAN_ERR_MASK	(0x00020000U)
#define NPI_STATUS_FIRST_SCAN_DONE_SHIFT	(9U)
#define NPI_STATUS_FIRST_SCAN_DONE_MASK		(0x00000200U)
#define NPI_STATUS_SCAN_SUSPENDED_SHIFT		(10U)
#define NPI_STATUS_SCAN_SUSPENDED_MASK		(0x00000400U)
#define NPI_STATUS_SCAN_PSCAN_EN_SHIFT		(11U)
#define NPI_STATUS_SCAN_PSCAN_EN_MASK		(0x00000800U)
#define POLL_TIMEOUT						(4U)
#define NPI_DESCTYPE_MASK					(0x300U)
#define NPI_DESCTYPE_SHIFT					(8U)
#define NPI_DESCTYPE_GT						(0x1U)
#define NPI_DESCTYPE_DDRMC					(0x2U)
#define NPI_DESC_BASE_ADDR_MASK				(0xFFFF0000U)
#define NPI_SLAVE_SKIP_CNT_MASK				(0x000000FFU)
#define DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))

static XIpiPsu IpiInst;
static XScuGic GicInst;
XSem_Notifier Notifier = {
        .Module = XSEM_NOTIFY_NPI,
        .Event = XSEM_EVENT_NPI_CRC_ERR | XSEM_EVENT_NPI_DESC_FMT_ERR |
		XSEM_EVENT_NPI_DESC_ABSNT_ERR | XSEM_EVENT_NPI_SHA_IND_ERR |
		XSEM_EVENT_NPI_SHA_ENGINE_ERR | XSEM_EVENT_NPI_PSCAN_MISSED_ERR |
		XSEM_EVENT_NPI_CRYPTO_EXPORT_SET_ERR | XSEM_EVENT_NPI_SFTY_WR_ERR |
		XSEM_EVENT_NPI_GPIO_ERR | XSEM_EVENT_NPI_SELF_DIAG_FAIL |
		XSEM_EVENT_NPI_GT_ARB_FAIL,
	.Flag = 1U,
};

/* Global variables to hold the event count when notified */
struct XSem_Npi_Events_t{
	u32 CrcEventCnt;
	u32 IntEventCnt;
	u32 DescFmtEventCnt;
	u32 DescAbsntEventCnt;
	u32 ShaIndEventCnt;
	u32 ShaEngineEventCnt;
	u32 PscanMissedEventCnt;
	u32 CryptoExportSetEventCnt;
	u32 SftyWrEventCnt;
	u32 GpioEventCnt;
	u32 SelfDiagFailEventCnt;
	u32 GtArbFailEventCnt;
} NpiEvents;

XSem_DescriptorData DescData_PreInj = {0};
XSem_DescriptorData DescData_PostInj = {0};
u32 TotalDescCnt = 0U;

/*Global variables to hold the Fail count */
u32 FailCnt = 0U;

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

	if ((XSEM_EVENT_ERROR == Payload[0]) && (XSEM_NOTIFY_NPI == Payload[1])) {
		if (XSEM_EVENT_NPI_CRC_ERR == Payload[2]) {
			NpiEvents.CrcEventCnt = 1U;
		} else if (XSEM_EVENT_NPI_DESC_FMT_ERR == Payload[2]) {
			NpiEvents.DescFmtEventCnt = 1U;
			xil_printf("[ALERT] Received Descriptor Format error event"
					" notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_DESC_ABSNT_ERR == Payload[2]) {
			NpiEvents.DescAbsntEventCnt = 1U;
			xil_printf("[ALERT] Received Descriptor Absent error event"
					"notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_SHA_IND_ERR == Payload[2]) {
			NpiEvents.ShaIndEventCnt = 1U;
			xil_printf("[ALERT] Received SHA indicator error event"
					"notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_SHA_ENGINE_ERR == Payload[2]) {
			NpiEvents.ShaEngineEventCnt = 1U;
			xil_printf("[ALERT] Received SHA engine error event notification"
					" from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_PSCAN_MISSED_ERR == Payload[2]) {
			NpiEvents.PscanMissedEventCnt = 1U;
			xil_printf("[ALERT] Received Periodic Scan missed error event"
					" notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_CRYPTO_EXPORT_SET_ERR == Payload[2]) {
			NpiEvents.CryptoExportSetEventCnt = 1U;
			xil_printf("[ALERT] Received Cryptographic Accelerator Disabled"
					" event notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_SFTY_WR_ERR == Payload[2]) {
			NpiEvents.SftyWrEventCnt = 1U;
			xil_printf("[ALERT] Received Safety Write error event"
					" notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_GPIO_ERR == Payload[2]) {
			NpiEvents.GpioEventCnt = 1U;
			xil_printf("[ALERT] Received GPIO error event notification from"
					" XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_SELF_DIAG_FAIL == Payload[2]) {
			NpiEvents.SelfDiagFailEventCnt = 1U;
			xil_printf("[ALERT] Received NPI Self-diagnosis fail event"
					" notification from XilSEM\n\r");
		} else if (XSEM_EVENT_NPI_GT_ARB_FAIL == Payload[2]) {
			NpiEvents.GtArbFailEventCnt = 1U;
			xil_printf("[ALERT] Received GT arbitration failure event"
					" notification from XilSEM\n\r");
		} else {
			xil_printf("%s Some other callback received: %d:%d:%d\n",
					__func__, Payload[0], Payload[1], Payload[2]);
		}
	} else {
		xil_printf("%s Some other callback received: %d\n", __func__,
				Payload[0]);
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
	u32 DescCnt = 0U;
	u32 DescAttrib = 0U;
	u32 DescGldnSha = 0U;
	u32 DescBaseAddr = 0U;
	u32 DescType = 0U;
	u32 SkipCountIndex = 0U;

	xil_printf("-----------------------------------------------------\n\r");
	xil_printf("-----------------Print Report------------------------\n\r");
	xil_printf("-----------------------------------------------------\n\r");

	/* Check if SHA mismatch error is reported */
	Status = XSem_CmdNpiGetStatus(&NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
				__func__, Status);
		FailCnt++;
	}

	Status = DataMaskShift(NpiStatus.Status,
			NPI_STATUS_SHA_COMP_SCAN_ERR_MASK,
			NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT);

	if (Status == 1U) {
		xil_printf("[SUCCESS] SHA comparison error detected\n\r");
	}else{
		FailCnt++;
	}

	/* Check Scan counter */
	Status = XSem_ApiCheckScanCount(&NpiStatus);
	if (Status == XST_FAILURE) {
		xil_printf("[SUCCESS] Scan counter not incrementing\n\r");
	} else {
		xil_printf("[ERROR] Scan counter incrementing\n\r");
		FailCnt++;
	}
	/* Check Heartbeat counter */
	Status = XSem_ApiCheckHbtCount(&NpiStatus);
	if (Status == XST_FAILURE) {
		xil_printf("[SUCCESS] Heartbeat counter not incrementing\n\r");
	} else {
		xil_printf("[ERROR] Heartbeat counter incrementing\n\r");
		FailCnt++;
	}

	/* Check event notification */
	if (NpiEvents.CrcEventCnt == 1U) {
		xil_printf("[SUCCESS] Received CRC error event notification\n\r");
		NpiEvents.CrcEventCnt = 0U;
	} else {
		xil_printf("[FAILURE] No CRC error event notification received\n\r");
		FailCnt++;
	}

	/* Print total descriptor count */
	xil_printf("\n\r----------------------------------------------------\n\r");
	xil_printf("Total Descriptor Count = %u\n\r", TotalDescCnt);

	/* Print golden SHA values before injecting error */
	xil_printf("----------------------------------------------------\n\r");
	xil_printf("Descriptor information before injecting error:\n\n\r");
	for(DescCnt = 0; DescCnt < TotalDescCnt; DescCnt++) {
		/* Store Attribute and goldenSHA for printing */
		DescAttrib = DescData_PreInj.DescriptorInfo[DescCnt].DescriptorAttrib;
		DescGldnSha = DescData_PreInj.DescriptorInfo[DescCnt].DescriptorGldnSha;

		/* Obtain Descriptor Type (Static, GT or DDRMC) */
		DescType = DataMaskShift(DescAttrib, NPI_DESCTYPE_MASK, \
				NPI_DESCTYPE_SHIFT);
		xil_printf("Descriptor %u\n\r", (DescCnt+1U));
		xil_printf("  Type: ");
		/* Print Descriptor type information */
		if (DescType == NPI_DESCTYPE_DDRMC) {
			xil_printf("DDRMC MAIN\n\r");
		} else if (DescType == NPI_DESCTYPE_GT) {
			xil_printf("GT\n\r");
		} else {
			xil_printf("Static\n\r");
		}
		/* Print Golden SHA */
		xil_printf("  Golden SHA: 0x%08x\n\r", DescGldnSha);
		/* Print Descriptor attribute information if descriptor is not static */
		if(DescType > 0U) {
			DescBaseAddr = (DescAttrib & NPI_DESC_BASE_ADDR_MASK);
			SkipCountIndex = (DescAttrib & NPI_SLAVE_SKIP_CNT_MASK);
			xil_printf("  Base Address: 0x%08x\n\r  Skip Count Location:" \
					" SkipCountByte%u\n\r", DescBaseAddr, SkipCountIndex);
		}
	}

	/* Print golden SHA values after injecting error */
	xil_printf("----------------------------------------------------\n\r");
	xil_printf("Descriptor information after injecting error:\n\n\r");
	for(DescCnt = 0; DescCnt < TotalDescCnt; DescCnt++) {
		/* Store Attribute and goldenSHA for printing */
		DescAttrib = DescData_PostInj.DescriptorInfo[DescCnt].DescriptorAttrib;
		DescGldnSha = \
				DescData_PostInj.DescriptorInfo[DescCnt].DescriptorGldnSha;

		/* Obtain Descriptor Type (Static, GT or DDRMC) */
		DescType = DataMaskShift(DescAttrib, NPI_DESCTYPE_MASK, \
				NPI_DESCTYPE_SHIFT);
		xil_printf("Descriptor %u\n\r", (DescCnt+1U));
		xil_printf("  Type: ");
		/* Print Descriptor type information */
		if (DescType == NPI_DESCTYPE_DDRMC) {
			xil_printf("DDRMC MAIN\n\r");
		} else if (DescType == NPI_DESCTYPE_GT) {
			xil_printf("GT\n\r");
		} else {
			xil_printf("Static\n\r");
		}
		/* Print Golden SHA */
		xil_printf("  Golden SHA: 0x%08x\n\r", DescGldnSha);
		/* Print Descriptor attribute information if descriptor is not static */
		if(DescType > 0U) {
			DescBaseAddr = (DescAttrib & NPI_DESC_BASE_ADDR_MASK);
			SkipCountIndex = (DescAttrib & NPI_SLAVE_SKIP_CNT_MASK);
			xil_printf("  Base Address: 0x%08x\n\r  Skip Count Location:" \
					" SkipCountByte%u\n\r", DescBaseAddr, SkipCountIndex);
		}
	}
	xil_printf("----------------------------------------------------\n\r");

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
* @note		Npi example always injects error in the golden SHA of the first
*           descriptor. The injected SHA error can be corrected by executing
*           the error injection sequence again. Real SHA errors are
*           uncorrectable.
*
******************************************************************************/
int main(void)
{
	XStatus Status = XST_FAILURE;
	u32 TempA_32 = 0U;
	u32 TempB_32 = 0U;
	u32 TimeoutCount = 0U;
	u32 TotalCnt = 16U;
	XSemNpiStatus NpiStatus = {0};
	XSemIpiResp IpiResp = {0};

	/* Disable cache to enable PLM access to OCM registers */
	Xil_DCacheDisable();

	Status = XSem_IpiInitApi();
	if (XST_SUCCESS != Status) {
		xil_printf("Ipi init failure with status 0x%x\n\r", \
				__func__, Status);
		FailCnt++;
		goto END;
	}

	XSem_NpiEventRegisterNotifier(1U);

	/* The following sequence demonstrates how to inject errors in NPI
	 * 1. Stop NPI scan
	 * 2. Read current Golden SHA from descriptors
	 * 3. Inject error
	 * 4. Start NPI scan
	 * 5. Read for Golden SHA mismatch error
	 * 6. Read current Golden SHA from descriptors
	 * Note: Execute the sequence again to correct the injected error,
	 *       but the error status will remain till POR.
	 */

	/* Check if NPI is stopped or not started previously */
	Status = XSem_CmdNpiGetStatus(&NpiStatus);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
				__func__, Status);
		FailCnt++;
		goto END;
	}

	TempA_32 = DataMaskShift(NpiStatus.Status, NPI_STATUS_SCAN_PSCAN_EN_MASK,
				NPI_STATUS_SCAN_PSCAN_EN_SHIFT);

	TempB_32 = DataMaskShift(NpiStatus.Status, NPI_STATUS_SCAN_SUSPENDED_MASK,
				NPI_STATUS_SCAN_SUSPENDED_SHIFT);

	/* If either Periodic scan is not enabled or is stopped */
	if ( (TempA_32 == 0U) || (TempB_32 == 1U) ) {
		/* Start NPI scan */
		Status = XSem_CmdNpiStartScan(&IpiInst, &IpiResp);
		if ((XST_SUCCESS == Status) &&
				(CMD_ACK_NPI_STARTSCAN == IpiResp.RespMsg1) &&
				(XST_SUCCESS == IpiResp.RespMsg2)) {
			xil_printf("[%s] Success: Start\n\r", __func__);
		} else {
			xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, Ret 0x%x" \
					"\n\r",	__func__, Status, IpiResp.RespMsg1, \
					IpiResp.RespMsg2);
			Status = XST_FAILURE;
			FailCnt++;
		}
	}

	/* Check scan count to ensure NPI scan is running */
	Status = XSem_ApiCheckScanCount(&NpiStatus);
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI Scan count not incrementing.\n\r", \
				__func__, Status);
		FailCnt++;
	}

	/* Get golden SHA and descriptor information before injecting error */
	Status = XSem_CmdNpiGetGldnSha(&IpiInst, &IpiResp, &DescData_PreInj);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_GET_GLDN_SHA == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Get Golden SHA\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Get Golden SHA Status 0x%x Ack 0x%x, Ret " \
				"0x%x\n\r", __func__, Status, IpiResp.RespMsg1, \
				IpiResp.RespMsg2);
		Status = XST_FAILURE;
		FailCnt++;
	}

	/* Store total descriptor count */
	TotalDescCnt = DescData_PreInj.DescriptorCount;

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
		FailCnt++;
	}

	/* Inject error in Golden SHA of first descriptor */
	Status = XSem_CmdNpiInjectError(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_ERRINJECT == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Inject\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Inject Status 0x%x Ack 0x%x, Ret 0x%x\n\r", \
				__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
		FailCnt++;
	}

	/* Restart NPI scan after injecting error */
	Status = XSem_CmdNpiStartScan(&IpiInst, &IpiResp);
	if ((XST_SUCCESS == Status) &&
			(CMD_ACK_NPI_STARTSCAN == IpiResp.RespMsg1) &&
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Start\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x, Ret 0x%x\n\r", \
				__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
		FailCnt++;
	}

	/* Wait for XilSEM to detect error */
	TimeoutCount = POLL_TIMEOUT;
	while (TimeoutCount != 0U) {
		Status = XSem_CmdNpiGetStatus(&NpiStatus);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] ERROR: NPI Status read failure.\n\r", \
					__func__, Status);
			FailCnt++;
			goto END;
		}
		/* Read NPI_SCAN_ERROR status bit */
		TempA_32 = DataMaskShift(NpiStatus.Status,
						NPI_STATUS_SHA_COMP_SCAN_ERR_MASK,
						NPI_STATUS_SHA_COMP_SCAN_ERR_SHIFT);
		if (TempA_32 == 1U) {
			Status = XST_SUCCESS;

			/* Get golden SHA and descriptor information
			 * after injecting error */
			Status = XSem_CmdNpiGetGldnSha(&IpiInst, &IpiResp,
					&DescData_PostInj);
			if ((XST_SUCCESS == Status) &&
					(CMD_ACK_NPI_GET_GLDN_SHA == IpiResp.RespMsg1) &&
					(XST_SUCCESS == IpiResp.RespMsg2)) {
				xil_printf("[%s] Success: Get Golden SHA\n\r", __func__);
			} else {
				xil_printf("[%s] Error: Get Golden SHA Status 0x%x Ack 0x%x,"
						" Ret 0x%x\n\r", __func__, Status, IpiResp.RespMsg1, \
						IpiResp.RespMsg2);
				Status = XST_FAILURE;
				FailCnt++;
			}

			goto END;
		}
		TimeoutCount--;
		/* Small delay before polling again */
		usleep(25000);
	}
	xil_printf("[%s] ERROR: Timeout occurred waiting for error.\n\r",
			__func__);
	Status = XST_FAILURE;
	FailCnt++;
	goto END;

END:
	PrintErrReport();
	xil_printf("\n\r-------------- Test Report --------------\n\r");
	xil_printf("Total  : %d\n\r", TotalCnt);
	xil_printf("Passed : %d\n\r", (TotalCnt - FailCnt));
	xil_printf("Failed : %d\n\r", FailCnt);

	if(FailCnt) {
			xil_printf("NPI examples Failed \n");
	}else{
			xil_printf("NPI examples ran successfully \n");
		}
		xil_printf("-----------------------------------------\n\r");

	/* Enable cache to disable PLM access to OCM registers */
	Xil_DCacheEnable();

	return Status;
}
