/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_npi_ssit_example.c
 *
 * This file has XilSEM NPI SHA error injection example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hb    07/03/2022  Initial Creation
 * 0.2   hv    08/08/2022  Fixed build error due to updated macros
 * 0.3   hv    08/15/2022  Updated broadcast APIs to check status of all SLRs
 *                         separately
 * 0.4   hb    08/22/2022  Updated to use XSem_Ssit_CmdGetStatus API
 * 0.5   gm    03/06/2023  Updated total test count with SLR count macro
 *                         to support different SSIT devices.
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "xsem_client_api.h"
#include "sleep.h"
#include "xsem_ipi.h"
#include "xsem_gic_setup.h"
#include "xil_cache.h"
#include "xil_util.h"

#define DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))
#define XSEM_CUR_SLR_MASK			(0x00000001U)
#define XSEM_NPI_SCAN_TIME_MASK		(0x0003FF00U)
#define XSEM_NPI_SCAN_TIME_SHIFT	(8U)
#define XSEM_STARTUP_CONFIG_MASK	(0x00000030U)
#define XSEM_STARTUP_CONFIG_SHIFT	(4U)
#define XSEM_NPI_ENABLED_MASK		(0x00000003U)
#define XSEM_NPI_ENABLED_SHIFT		(0U)
#define XSEM_NPI_SCAN_STATUS_MASK	(0xFFFU)
#define XSEM_NPI_SCAN_ACTIVE		(0xA04U)
#define XSEM_NPI_SCAN_IDLE			(0xA01U)

/* IPI instance */
static XIpiPsu IpiInst;
/* Interrupt Instance */
static XScuGic GicInst;
/* NPI Notifier */
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
/* Global value to store the SLR source during event notification */
static u32 EventSrcSlr = 0U;
/* Global value to store information on startup */
u32 ImmediateStartupEn = 0U;
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
} NpiEvents[XSEM_SSIT_MAX_SLR_CNT];

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

	EventSrcSlr = Payload[3];

	if ((XSEM_EVENT_ERROR == Payload[0]) && (XSEM_NOTIFY_NPI == Payload[1])) {
		if (XSEM_EVENT_NPI_CRC_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].CrcEventCnt = 1U;
			xil_printf("[ALERT] Received Crc error event notification"
					" from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_DESC_FMT_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].DescFmtEventCnt = 1U;
			xil_printf("[ALERT] Received Descriptor Format error event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_DESC_ABSNT_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].DescAbsntEventCnt = 1U;
			xil_printf("[ALERT] Received Descriptor Absent error event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_SHA_IND_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].ShaIndEventCnt = 1U;
			xil_printf("[ALERT] Received SHA indicator error event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_SHA_ENGINE_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].ShaEngineEventCnt = 1U;
			xil_printf("[ALERT] Received SHA engine error event notification"
					" from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_PSCAN_MISSED_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].PscanMissedEventCnt = 1U;
			xil_printf("[ALERT] Received Periodic Scan missed error event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_CRYPTO_EXPORT_SET_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].CryptoExportSetEventCnt = 1U;
			xil_printf("[ALERT] Received Cryptographic Accelerator Disabled"
					" event notification from XilSEM on SLR-%u\n\r",
					EventSrcSlr);
		} else if (XSEM_EVENT_NPI_SFTY_WR_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].SftyWrEventCnt = 1U;
			xil_printf("[ALERT] Received Safety Write error event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_GPIO_ERR == Payload[2]) {
			NpiEvents[EventSrcSlr].GpioEventCnt = 1U;
			xil_printf("[ALERT] Received GPIO error event notification from"
					" XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else if (XSEM_EVENT_NPI_SELF_DIAG_FAIL == Payload[2]) {
			NpiEvents[EventSrcSlr].SelfDiagFailEventCnt = 1U;
			xil_printf("[ALERT] Received NPI Self-diagnosis fail event"
					" notification from XilSEM on SLR-%u\n\r",EventSrcSlr);
		} else if (XSEM_EVENT_NPI_GT_ARB_FAIL == Payload[2]) {
			NpiEvents[EventSrcSlr].GtArbFailEventCnt = 1U;
			xil_printf("[ALERT] Received GT arbitration failure event"
					" notification from XilSEM on SLR-%u\n\r", EventSrcSlr);
		} else {
			xil_printf("%s Some other callback received: %d:%d:%d\n",
					__func__, Payload[0], Payload[1], Payload[2],
					Payload[3]);
		}
	} else {
		xil_printf("%s Some other callback received: %d\n", __func__,
				Payload[0]);
	}
}

/*****************************************************************************
 * @brief	Initialize XilSEM IPI instance to process XilSEM
 * 			notifications from PLM.
 *
 * @return	XST_SUCCESS : upon successful initialization of XilSEM IPI.
 * 			XST_FAILURE : any failure in initialization of XilSEM IPI.
 *
 *****************************************************************************/
static XStatus XSem_IpiInitApi (void)
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

/******************************************************************************
 * @brief	This function is used to Register XilSEM event notifications on
 *          master PLM. Master PLM can receive notifications from slave SLRs
 *          and forward to client
 *
 * @param[in]	Enable : Enable event notification in XilSEM
 *
 * @return	Status : Success or Failure
 *
 *****************************************************************************/
static int XSem_NpiEventRegisterNotifier(u32 Enable)
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

/*****************************************************************************
 * @brief	This function is used to send broadcast command to start NPI scan
 * 			on all SLRs using XilSEM Client API XSem_Ssit_CmdNpiStartScan
 *
 * @return	XST_SUCCESS : upon successful start of NPI scan.
 * 			XST_FAILURE : any failure to start NPI scan.
 *
 *****************************************************************************/
static XStatus XSem_ApiNpiStartScan_Broadcast(XIpiPsu *IpiInst,
															XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdNpiStartScan(IpiInst, Resp, \
			XSEM_SSIT_ALL_SLRS_ID);

	if ((XST_SUCCESS == Status) && (CMD_ACK_NPI_STARTSCAN == Resp->RespMsg1) \
			&& (XST_SUCCESS == Resp->RespMsg2) &&
			(XST_SUCCESS == Resp->RespMsg3) &&
			(XST_SUCCESS == Resp->RespMsg4 &&
			(XST_SUCCESS == Resp->RespMsg5))) {
		xil_printf("[%s] Success: NPI StartScan on all SLRs\n\r", __func__);
	} else {
		xil_printf("[%s] Error: NPI StartScan Status 0x%x Ack 0x%x \n", \
			__func__, Status, Resp->RespMsg1);

		/*Check If RespMsg2 is success or not */
		if (Resp->RespMsg2 != XST_SUCCESS){
			/* If not success NPI StartScan is failed in master SLR */
			xil_printf("[%s] NPI StartScan failed on SLR-0 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg2);
		}
		/*Check If RespMsg3 is success or not */
		if (Resp->RespMsg3 != XST_SUCCESS){
			/* If not success NPI StartScan is failed in Slave 1 */
			xil_printf("[%s] NPI StartScan failed on SLR-1 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg3);
		}
		/*Check If RespMsg4 is success or not */
		if (Resp->RespMsg4 != XST_SUCCESS){
			/* If not success NPI StartScan is failed in Slave 2 */
			xil_printf("[%s] NPI StartScan failed on SLR-2 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg4);
		}
		/*Check If RespMsg5 is success or not */
		if (Resp->RespMsg5 != XST_SUCCESS){
			/* If not success NPI StartScan is failed in Slave 3 */
			xil_printf("[%s] NPI StartScan failed on SLR-3 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg5);
		}
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************
 * @brief	This function is used to send broadcast command to stop NPI scan
 * 			on all SLRs using XilSEM Client API XSem_Ssit_CmdNpiStopScan
 *
 * @return	XST_SUCCESS : upon successful stop of NPI scan.
 * 			XST_FAILURE : any failure to stop NPI scan.
 *
 *****************************************************************************/
static XStatus XSem_ApiNpiStopScan_Broadcast(XIpiPsu *IpiInst,
															XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdNpiStopScan(IpiInst, Resp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && (CMD_ACK_NPI_STOPSCAN == Resp->RespMsg1) \
			&& (XST_SUCCESS == Resp->RespMsg2) &&
			(XST_SUCCESS == Resp->RespMsg3) &&
			(XST_SUCCESS == Resp->RespMsg4 &&
			(XST_SUCCESS == Resp->RespMsg5))) {
		xil_printf("[%s] Success: NPI StopScan on all SLRs\n\r", __func__);
	} else {
		xil_printf("[%s] Error: NPI StopScan Status 0x%x Ack 0x%x \n", \
			__func__, Status, Resp->RespMsg1);

		/*Check If RespMsg2 is success or not */
		if (Resp->RespMsg2 != XST_SUCCESS){
			/* If not success NPI StopScan is failed in master SLR */
			xil_printf("[%s] NPI StopScan failed on SLR-0 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg2);
		}
		/*Check If RespMsg3 is success or not */
		if (Resp->RespMsg3 != XST_SUCCESS){
			/* If not success NPI StopScan is failed in Slave 1 */
			xil_printf("[%s] NPI StopScan failed on SLR-1 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg3);
		}
		/*Check If RespMsg4 is success or not */
		if (Resp->RespMsg4 != XST_SUCCESS){
			/* If not success NPI StopScan is failed in Slave 2 */
			xil_printf("[%s] NPI StopScan failed on SLR-2 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg4);
		}
		/*Check If RespMsg5 is success or not */
		if (Resp->RespMsg5 != XST_SUCCESS){
			/* If not success NPI StopScan is failed in Slave 3 */
			xil_printf("[%s] NPI StopScan failed on SLR-3 with error"
					" code = 0x%08x\n\r", __func__, Resp->RespMsg5);
		}
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************
 * @brief	This function injects error in NPI scan serially in all SLRs
 * 			serially using XilSEM Client API XSem_Ssit_CmdNpiInjectError
 *
 * @return	XST_SUCCESS : upon successful error injection in NPI scan.
 * 			XST_FAILURE : any failure to inject error in NPI scan.
 *
 *****************************************************************************/
static XStatus XSem_ApiNpiInjectError(XIpiPsu *IpiInst, XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Check NPI scan configuration and start scan on enabled SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdNpiInjectError(IpiInst, Resp, SlrCnt);
		if ((Status != XST_SUCCESS) || (Resp->RespMsg2 != XST_SUCCESS)) {
			xil_printf("%s Failed to inject error in Npi scan\n\r", \
					__func__);
			goto END;
		}

		if (Resp->RespMsg3 != 0U) {
			xil_printf("[%s] Npi error inject failed on SLR-%u:\n\r", \
					__func__, SlrCnt);
		}
	}
END:
	return Status;
}

/*****************************************************************************
 * @brief	This function gets NPI scan configuration of each SLR serially
 * 			on all SLRs using XilSEM Client API XSem_ApiNpiGetConfig
 *
 * @return	XST_SUCCESS : upon successful retrieval of NPI scan configuration
 * 			XST_FAILURE : any failure in retrieval of NPI scan configuration
 *
 *****************************************************************************/
static XStatus XSem_ApiNpiGetConfig(XIpiPsu *IpiInst, XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 NpiConfig = 0U;
	u32 NpiStartupConfig = 0U;
	u32 NpiScanAvailable = 0U;
	u32 NpiScanTime = 0U;

	xil_printf("-------------- NPI Scan Configuration --------------\n\r");
	/* Check NPI scan configuration on each SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdGetConfig(IpiInst, Resp, SlrCnt);
		if (Status != XST_SUCCESS) {
			xil_printf("Failed to get NPI configuration\n\r");

			goto END;
		}

		if (Resp->RespMsg3 != 0U) {
			xil_printf("Getting Npi scan configuration for SLR-%u:\n\r",
					SlrCnt);

			/* Get NPI config for SLR */
			NpiConfig = Resp->RespMsg3;

			/* Check if NPI scan is available on SLR */
			NpiScanAvailable = DataMaskShift(NpiConfig, \
					XSEM_NPI_ENABLED_MASK, XSEM_NPI_ENABLED_SHIFT);

			if (NpiScanAvailable == 0U) {
				xil_printf("Npi scan is not enabled on SLR\n\r");
				continue;
			}

			/* Get NPI scan startup configuration */
			NpiStartupConfig = DataMaskShift(NpiConfig, \
					XSEM_STARTUP_CONFIG_MASK, \
					XSEM_STARTUP_CONFIG_SHIFT);
			if (NpiStartupConfig == 0x00) {
				xil_printf("Npi scan is configured for deferred " \
						"startup\n\r");
				ImmediateStartupEn = 0U;
			} else if (NpiStartupConfig == 0x01) {
				xil_printf("Npi scan is configured for immediate " \
						"startup\n\r");
				ImmediateStartupEn = 1U;
			} else {
				xil_printf("Error: Unknown Npi scan configuration " \
						"(0x%08x)\n\r",	NpiStartupConfig);
			}

			/* Get NPI scan interval */
			NpiScanTime = DataMaskShift(NpiConfig,\
					XSEM_NPI_SCAN_TIME_MASK, XSEM_NPI_SCAN_TIME_SHIFT);

			if (NpiScanTime == 0U) {
				xil_printf("Npi scan interval invalid %ums\n\n\r", NpiScanTime);
			} else {
				xil_printf("Npi scan interval: %ums\n\n\r", NpiScanTime);
			}

		} else {
			xil_printf("Device not configured for NPI scan on Slr-%u" \
					"\n\r", SlrCnt);
		}
	}
END:
	return Status;
}

/*****************************************************************************
 * @brief	This function gets NPI scan status and checks if NPI scan is
 *          actively running on all SLRs serially using XSem_Ssit_CmdGetStatus
 *
 * @return	XST_SUCCESS : When NPI scan is active on all SLRs
 * 			XST_FAILURE : NPI scan failure in any of the the SLRs
 *
 *****************************************************************************/
XStatus XSem_ApiNpiGetAllStatus(XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	XSemStatus NpiStatus;
	u32 PassCnt = 0U;
	u32 TempVal = 0U;
	u32 SlrCnt;

	xil_printf("-------------- NPI Scan Status --------------\n\r");
	/* Get Status of all SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdGetStatus(&IpiInst, Resp, SlrCnt, &NpiStatus);
		TempVal = DataMaskShift(NpiStatus.NpiStatus,
				XSEM_NPI_SCAN_STATUS_MASK, 0U);
		if ((TempVal == XSEM_NPI_SCAN_ACTIVE) ||
				(TempVal == XSEM_NPI_SCAN_IDLE)) {
			xil_printf("Npi scan is successfully running on SLR-%u\n\r",
					SlrCnt);
			/* Increase pass count */
			PassCnt++;
			/* Clear TempVal for next read */
			TempVal = 0U;
		} else {
			Status = XST_FAILURE;
			xil_printf("Npi scan is not running on SLR-%u,"
					" Status = 0x%08x\n\r", SlrCnt, NpiStatus.NpiStatus);
		}
		xil_printf("\n\r");
		/* Clear structure NpiStatus before reading again */
		Xil_SMemSet(&NpiStatus, sizeof(NpiStatus), 0x00U, sizeof(NpiStatus));
		/* Small delay before sending command */
		usleep(1000);
	}

	if (PassCnt == XSEM_SSIT_MAX_SLR_CNT) {
		Status = XST_SUCCESS;
	}
	return Status;
}

/*****************************************************************************/
/**
*
* @brief	The purpose of this function is to illustrate the usage of
* 			client APIs for NPI scan over IPI for all SLRs in SSIT device.
*
* @return
*
* - XST_SUCCESS - on successful execution of injection and detection of error
* - XST_FAILURE - on failure
*
* @note		NPI example always injects error in the golden SHA of the first
*           descriptor. The injected SHA error can be corrected by executing
*           the error injection sequence again. Real SHA errors are
*           uncorrectable.
*
*           This example assumes NPI scan configuration is same for all SLRs
*
******************************************************************************/
int main(void)
{
	XStatus Status = XST_FAILURE;
	XSemIpiResp IpiResp = {0};
	u32 XSem_TotalTestCnt = (5U + XSEM_SSIT_MAX_SLR_CNT);
	u32 XSem_PassCnt = 0U;
	u32 SlrCnt;

	xil_printf("\nStarting NPI scan example demo for SSIT device\n\r");

	/* Disable cache before using get status API */
	Xil_DCacheDisable();

	/* Initialize IPI Driver
	 * This initialization is required to get XilSEM event notifications
	 * from PLM
	 */
	Status = XSem_IpiInitApi();
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] Ipi init failure with status 0x%x\n\r", \
				__func__, Status);
		goto END;
	}

	/* Enable event Notifications on all SLRs to receive notifications
	 * from PLM upon detection of any error in NPI
	 */
	Status = XSem_NpiEventRegisterNotifier(1U);
	if (XST_SUCCESS != Status) {
		xil_printf("[%s] Failed to register events with status 0x%x\n\r", \
				__func__, Status);
		goto END;
	}

	/* The following sequence demonstrates how to inject errors in CRAM
	 * 1. Check NPI configuration validity
	 * 2. Start scan if scan is configured for deferred startup
	 * 3. Check if NPI scan is actively running on all SLRs
	 * 4. Stop scan on all SLRs
	 * 5. Inject error on all SLRs
	 * 6. Start scan on all SLRs
	 * 7. Wait for notification from PLM
	 */

	/* Get NPI configuration */
	Status = XSem_ApiNpiGetConfig(&IpiInst, &IpiResp);
	if (Status != XST_SUCCESS) {
		xil_printf("[%s] Failed to get NPI configuration with status " \
				"0x%x\n\r", __func__, Status);
	} else {
		XSem_PassCnt++;
	}

	/* If configured for deferred startup, start scan */
	if (ImmediateStartupEn == 0U) {
		Status = XSem_ApiNpiStartScan_Broadcast(&IpiInst, &IpiResp);
		if (Status != XST_SUCCESS) {
			xil_printf("[%s] Failed to start NPI scan with status 0x%x\n\r", \
					__func__, Status);
		} else {
			XSem_PassCnt++;
			XSem_TotalTestCnt++;
			/* Small delay to wait for NPI scan to startup */
			sleep(1U);
		}
	}

	/* Check if NPI scan is functional on all SLRs */
	Status = XSem_ApiNpiGetAllStatus(&IpiResp);
	if (Status != XST_SUCCESS) {
		xil_printf("[%s] NPI scan is not running successfully on all SLRs: " \
				"0x%x\n\r", __func__, Status);
	} else {
		XSem_PassCnt++;
	}

	/* Stop NPI scan before injecting error */
	Status = XSem_ApiNpiStopScan_Broadcast(&IpiInst, &IpiResp);
	if (Status != XST_SUCCESS) {
		xil_printf("[%s] Failed to stop NPI scan with status 0x%x\n\r", \
				__func__, Status);
	} else {
		XSem_PassCnt++;
	}

	/* Inject error in all SLRs */
	Status = XSem_ApiNpiInjectError(&IpiInst, &IpiResp);
	if (Status != XST_SUCCESS) {
		xil_printf("[%s] Failed to inject error with status 0x%x\n\r", \
				__func__, Status);
	} else {
		XSem_PassCnt++;
	}

	/* Restart NPI scan for errors to be detected */
	Status = XSem_ApiNpiStartScan_Broadcast(&IpiInst, &IpiResp);
	if (Status != XST_SUCCESS) {
		xil_printf("[%s] Failed to start NPI scan with status 0x%x\n\r", \
				__func__, Status);
	} else {
		XSem_PassCnt++;
	}

	/* Wait for error to be detected and reported */
	sleep(1U);

	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		if (NpiEvents[SlrCnt].CrcEventCnt == 1U) {
			xil_printf("\nCrc error detected on SLR-%u\n\r", \
					SlrCnt);
			XSem_PassCnt++;
		}
	}

END:
	xil_printf("\n---------------------------------------------\n\r");
	if (XSem_PassCnt == XSem_TotalTestCnt) {
		xil_printf("Success: NPI example test completed\n\r");
	} else {
		xil_printf("\nFailure: NPI example test failed\n\r");
	}
	/* Re-enable cache */
	Xil_DCacheEnable();
	return Status;
}
