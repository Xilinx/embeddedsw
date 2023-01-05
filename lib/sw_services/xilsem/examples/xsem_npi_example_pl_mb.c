/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_npi_example_pl_mb.c
 *
 * This file demonstrates on how to use XilSEM NPI client interface on PL
 * Microblaze to send commands to PLM firmware and read responses from PLM.
 *
 * @note
 * The default linker settings places a software stack, heap and data in
 * PL MB memory. For this example to work, any data shared between client
 * running on PL and server running on PMC, should be placed in area which
 * is accessible to both client and server.
 *
 * Following is the procedure to provide shared memory region which can be
 * accessed by server
 *
 * In linker script(lscript.ld) user can add new memory section in source
 * tab as shown below
 *
 * 		.sem_decdata : {
 * 			KEEP (*(.sem_decdata))
 * 		} > axi_noc_0_C0_DDR_LOW0
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who       Date        Changes
 * ----  ----   ----------  ---------------------------------------------------
 * 0.1   gupta   11/16/2022  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "sleep.h"
#include "xsem_client_api.h"
#include "xil_cache.h"
#include "xintc.h"

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

/* Interrupt Controller device ID */
#define INTC_DEVICE_ID			XPAR_INTC_0_DEVICE_ID
/* IPI device ID to use for this test */
#define IPI_CHANNEL_ID			XPAR_XIPIPSU_0_DEVICE_ID
/* Destination IPI channel mask used for this test */
#define IPI_PMC_MASK			0x2U
/**
 * Interrupt Number of IPI whose interrupt output is connected to the input
 * of the Interrupt Controller
 */
#define INTC_DEVICE_IPI_INT_ID	XPAR_AXI_INTC_0_VERSAL_CIPS_0_PSPMC_0_PS_PL_IRQ_LPD_IPI_IPI1_INTR

static XIpiPsu IpiInst;
static XIntc InterruptController;

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

XSem_DescriptorData DescData_PreInj __attribute__((section (".sem_decdata"))) __attribute__((no_reorder)) = {0};
XSem_DescriptorData DescData_PostInj __attribute__((section (".sem_decdata"))) __attribute__((no_reorder)) = {0};
u32 TotalDescCnt = 0U;

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

	/**
	 * Small delay to ensure NPI scan progresses before second read.
	 * The delay value assumes 100ms as NPI scan interval
	 */
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

	/**
	 * Small delay to ensure NPI scan progresses before second read.
	 * The delay value assumes 100ms as NPI scan interval
	 */
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
	/**
	 * In this example all NPI events are enabled
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

	Status = XIpiPsu_ReadMessage(&IpiInst, IPI_PMC_MASK, Payload, \
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
	if (NpiEvents.CrcEventCnt == 1U) {
		xil_printf("[SUCCESS] Received CRC error event notification\n\r");
		NpiEvents.CrcEventCnt = 0U;
	} else {
		xil_printf("[FAILURE] No CRC error event notification received\n\r");
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

/******************************************************************************/
/**
*
* This function connects the IPI interrupts to the interrupt controller and to
* the processor.
*
* @param	Intc instance pointer.
*
* @return	Status : Success or Failure
*
****************************************************************************/
int SetUpInterruptSystem(XIntc *XIntcInstancePtr)
{
	int Status = XST_FAILURE;

	/**
	 * Connect IPI interrupt handler that will be called when an interrupt
	 * for the IPI occurs, the IPI interrupt handler performs the
	 * specific interrupt processing for IPI.
	 */
	Status = XIntc_Connect(XIntcInstancePtr, INTC_DEVICE_IPI_INT_ID,
				   (XInterruptHandler)XSem_IpiCallback,
				   (void *)0);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts.
	 */
	Status = XIntc_Start(XIntcInstancePtr, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Enable the interrupt for the device. */
	XIntc_Enable(XIntcInstancePtr, INTC_DEVICE_IPI_INT_ID);

	/* Initialize the exception table. */
	Xil_ExceptionInit();

	/* Register the interrupt controller handler with the exception table. */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)XIntc_InterruptHandler,
				XIntcInstancePtr);

	/* Enable exceptions. */
	Xil_ExceptionEnable();

END:
	return Status;
}

/*****************************************************************************/
/**
*
* This function initializes Interrupt Controller and IPI drivers.
*
* @return	XST_SUCCESS to indicate success, otherwise XST_FAILURE.
*
****************************************************************************/
int IntcAndIpiInit(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *CfgPtr;

	xil_printf("Initializing IPI start\r\n");

	/* Look Up the config data */
	CfgPtr = XIpiPsu_LookupConfig(IPI_CHANNEL_ID);
	xil_printf("IPI look up config done\r\n");

	/* Init with the Cfg Data */
	XIpiPsu_CfgInitialize(&IpiInst, CfgPtr, CfgPtr->BaseAddress);
	xil_printf("IPI config init done\r\n");

	/**
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	Status = XIntc_Initialize(&InterruptController, INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("Intc init done\r\n");

	/**
	 * Perform a self-test to ensure that the hardware was built
	 * correctly.
	 */
	Status = XIntc_SelfTest(&InterruptController);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("Intc self test done\r\n");

	/* Setup the Interrupt System. */
	Status = SetUpInterruptSystem(&InterruptController);
	if (Status != XST_SUCCESS) {
		goto END;
	}
	xil_printf("setup interrupt done\r\n");

	/* Enable reception of IPIs from PMC */
	XIpiPsu_InterruptEnable(&IpiInst, IPI_PMC_MASK);
	xil_printf("IPI intr enabled\r\n");

	/* Clear Any existing Interrupts */
	XIpiPsu_ClearInterruptStatus(&IpiInst, IPI_PMC_MASK);
	xil_printf("IPI intr clear done\r\n");

END:
	return Status;
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
	XSemNpiStatus NpiStatus = {0};
	XSemIpiResp IpiResp = {0};

	/**
	 * Disable cache to get the shared buffer data from physical memory
	 * which is updated by PLM in response to NPI commands
	 */
	Xil_DCacheDisable();

	/**
	 * Initialize IPI Driver
	 * This initialization is required to get XilSEM event notifications
	 * from PLM
	 */
	Status = IntcAndIpiInit();
	if (Status != XST_SUCCESS) {
		xil_printf("IPI and INTC init failed: 0x%x\r\n", Status);
		xil_printf("Register notify example failed\r\n");
		goto END;
	}

	XSem_NpiEventRegisterNotifier(1U);

	/**
	 * The following sequence demonstrates how to inject errors in NPI
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
		}
	}

	/* Check scan count to ensure NPI scan is running */
	Status = XSem_ApiCheckScanCount(&NpiStatus);
	if (XST_FAILURE == Status) {
		xil_printf("[%s] ERROR: NPI Scan count not incrementing.\n\r", \
				__func__, Status);
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
	goto END;

END:
	PrintErrReport();
	return Status;
}
