/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_cram_ssit_example.c
 *
 * This file has XilSEM CRAM error injection example
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hb    07/03/2022  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "sleep.h"
#include "xsem_client_api.h"
#include "xsem_ipi.h"
#include "xsem_gic_setup.h"

#define XSEM_CRAM_COR_EN_MASK		(0x00000004U)
#define XSEM_CRAM_COR_EN_SHIFT		(2U)
#define XSEM_CRAM_ECC_SEL_MASK		(0x00000008U)
#define XSEM_CRAM_ECC_SEL_SHIFT		(3U)
#define XSEM_CRAM_ENABLED_MASK		(0x00000003U)
#define XSEM_CRAM_ENABLED_SHIFT		(0U)
#define XSEM_STARTUP_CONFIG_MASK	(0x00000030U)
#define XSEM_STARTUP_CONFIG_SHIFT	(4U)
#define XSEM_CUR_SLR_MASK		(0x00000001U)
#define DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))

static XIpiPsu IpiInst;
static XScuGic GicInst;
static XSemIpiResp IpiResp = {0};

XSem_Notifier Notifier = {
        .Module = XSEM_NOTIFY_CRAM,
        .Event = XSEM_EVENT_CRAM_UNCOR_ECC_ERR | XSEM_EVENT_CRAM_CRC_ERR | \
		 XSEM_EVENT_CRAM_INT_ERR | XSEM_EVENT_CRAM_COR_ECC_ERR,
	.Flag = 1U,
};

/*Global variables to hold the event count when notified*/
u8 EventCnt_UnCorEcc = 0U;
u8 EventCnt_Crc = 0U;
u8 EventCnt_CorEcc = 0U;
u8 EventCnt_IntErr = 0U;

static u32 IsStartupImmediate = 0U;

/****************************************************************************
 * @brief    	Initialize XilSEM IPI instance and register ISR handler to
 *              process XilSEM notifications from PLM.
 *
 * @return    	XST_SUCCESS : upon successful initialization of XilSEM IPI
 *     		XST_FAILURE : any failure in initialization of XilSEM IPI
 *
 ****************************************************************************/
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
static int Xsem_CfrEventRegisterNotifier(u32 Enable)
{
	int Status;

	if (Enable) {
		Notifier.Flag = 1U;
	} else {
		Notifier.Flag = 0U;
	}
	/* In this example all CRAM events are enabled
	 * If you want to enable a particular event set the Event member in
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
			(XSEM_NOTIFY_CRAM == Payload[1])) {
		if (XSEM_EVENT_CRAM_UNCOR_ECC_ERR == Payload[2]) {
			EventCnt_UnCorEcc = 1U;
			xil_printf("[ALERT] Received uncorrectable error notification " \
					"from XilSEM on SLR-%u\n\r", Payload[3]);
		} else if (XSEM_EVENT_CRAM_CRC_ERR == Payload[2]) {
			EventCnt_Crc = 1U;
		} else if (XSEM_EVENT_CRAM_INT_ERR == Payload[2]) {
			xil_printf("[ALERT] Received internal error event" \
				" notification from XilSEM on SLR-%u\n\r", Payload[3]);
			EventCnt_IntErr = 1U;
			xil_printf("[ALERT] Received internal error notification " \
					"from XilSEM on SLR-%u\n\r", Payload[3]);
		} else if (XSEM_EVENT_CRAM_COR_ECC_ERR == Payload[2]) {
			EventCnt_CorEcc = 1U;
			xil_printf("[ALERT] Received correctable error notification " \
					"from XilSEM on SLR-%u\n\r", Payload[3]);
		} else {
			xil_printf("%s Some other callback received: %d:%d:%d from " \
					"SLR-%u\n",
					__func__, Payload[0], \
					Payload[1], Payload[2], Payload[3]);
		}
	} else {
		xil_printf("%s Some other callback received: %d\n", \
				__func__, Payload[0]);
	}
}
/*****************************************************************************
 *
 * @brief	This function initializes CRAM scan on all SLRs in SSIT device
 * 			using XilSEM Client API XSem_ApiCfrInitCram_Broadcast
 *
 *
 * @return    	XST_SUCCESS : upon successful initialization of CRAM
 *  		XST_FAILURE : any failure in initialization of CRAM.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrInitCram_Broadcast()
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 SlrFailMask = 0U;

	Status = XSem_Ssit_CmdCfrInit(&IpiInst, &IpiResp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && (CMD_ACK_CFR_INIT == IpiResp.RespMsg1) \
			&& (XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Init\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Init Status 0x%x Ack 0x%x Ret 0x%x\n", \
				__func__, Status, IpiResp.RespMsg1, \
				IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	/* Check for Init failure */
	if (Status == XST_FAILURE) {
		/* Check response and list Cram Init failure on SLRs */
		for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
			SlrFailMask = IpiResp.RespMsg3;
			SlrFailMask = ((SlrFailMask >> SlrCnt) & XSEM_CUR_SLR_MASK);
			if (SlrFailMask == 1U) {
				xil_printf("[%s] Cram Init failed on SLR-%u\n\r",
						__func__, SlrCnt);
			}
		}
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function starts CRAM scan on all SLRs in SSIT device
 * 				using XilSEM Client API XSem_Ssit_CmdCfrStartScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrStartScan_Broadcast()
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 SlrFailMask = 0U;

	Status = XSem_Ssit_CmdCfrStartScan(&IpiInst, &IpiResp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_START_SCAN == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Start\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Start Status 0x%x Ack 0x%x Ret 0x%x\n",\
				__func__, Status, IpiResp.RespMsg1, \
				IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	/* Check for Start scan failure */
	if (Status == XST_FAILURE) {
		/* Check response and list Cram start failure on SLRs */
		for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
			SlrFailMask = IpiResp.RespMsg3;
			SlrFailMask = ((SlrFailMask >> SlrCnt) & XSEM_CUR_SLR_MASK);
			if (SlrFailMask == 1U) {
				xil_printf("[%s] Cram start failed on SLR-%u\n\r",
						__func__, SlrCnt);
			}
		}
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function stops CRAM scan on all SLRs in SSIT device
 *				using XilSEM Client API XSem_Ssit_CmdCfrStopScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrStopScan_Broadcast()
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 SlrFailMask = 0U;

	Status = XSem_Ssit_CmdCfrStopScan(&IpiInst, &IpiResp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_STOP_SCAN == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Stop\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Stop Status 0x%x Ack 0x%x Ret 0x%x\n", \
				__func__, Status, IpiResp.RespMsg1, \
				IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	/* Check for Start scan failure */
	if (Status == XST_FAILURE) {
		/* Check response and list Cram stop failure on SLRs */
		for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
			SlrFailMask = IpiResp.RespMsg3;
			SlrFailMask = ((SlrFailMask >> SlrCnt) & XSEM_CUR_SLR_MASK);
			if (SlrFailMask == 1U) {
				xil_printf("[%s] Cram stop failed on SLR-%u\n\r",
						__func__, SlrCnt);
			}
		}
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	Function to inject error in CFRAME in all SLRs using XilSEM
 * 				Client API XSem_Ssit_CmdCfrNjctErr
 *
 * @param    	ErrData : Pointer to Error details
 *
 * @return    	XST_SUCCESS : upon successful error injection.
 *   		XST_FAILURE : Any failure IPI interface
 *     		Error Codes : Below error codes upon failure during injection
 *		0x00500000U : If CRAM Scan is not initialized
 * 		0X00002000U : On getting Cframe driver instance as Null
 * 		0x00800000U : If invalid row is passed as input
 * 		0x00E00000U : If invalid block type is passed as input
 * 		0x00A00000U : If invalid bit is passed as input
 * 		0x00B00000U : If invalid frame is passed as input
 * 		0x00900000U : If invalid qword is passed as input
 * 		0x00D00000U : If input bit is masked
 * 		0x00C00000U : If injection is failed
 *****************************************************************************/
static XStatus XSem_ApiCfrErrNjct(XSemCfrErrInjData *ErrData)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Inject Cram error in all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrNjctErr(&IpiInst, ErrData, &IpiResp, SlrCnt);
		if ((XST_SUCCESS == Status) && \
				(CMD_ACK_CFR_NJCT_ERR == IpiResp.RespMsg1)) {
			if ((XST_SUCCESS != IpiResp.RespMsg2) ){
				xil_printf("[%s] Error: Injection Failed with 0x%x on SLR-%u" \
						"\n", __func__, IpiResp.RespMsg2, SlrCnt);
				Status = (XStatus) IpiResp.RespMsg2;
			} else {
				xil_printf("[%s] Success: Inject on SLR-%u\n\r", \
						__func__, SlrCnt);
			}
		} else {
			xil_printf("[%s] Error: Njct Status 0x%x Ack 0x%x Ret 0x%x\n", \
					__func__, Status, IpiResp.RespMsg1, \
					IpiResp.RespMsg2);
		}
	}
	return Status;
}

/*****************************************************************************
 * @brief	This function gets Cram scan configuration of each SLR serially
 * 			using XilSEM Client API XSem_Ssit_CmdGetConfig
 *
 * @return	XST_SUCCESS : upon successful retrieval of Cram scan configuration
 * 			XST_FAILURE : any failure in retrieval of NPI scan configuration
 *
 *****************************************************************************/
static XStatus XSem_ApiCfrGetConfig()
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 CramConfig = 0U;
	u32 CramStartupConfig = 0U;
	u32 CramScanAvailable = 0U;
	u32 CramEccSel = 0U;
	u32 CramCorEnabled = 0U;

	/* Check NPI scan configuration on each SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdGetConfig(&IpiInst, &IpiResp, SlrCnt);
		if (Status != XST_SUCCESS) {
			xil_printf("%s Failed to get NPI configuration\n\r", \
					__func__);
			goto END;
		}

		if (IpiResp.RespMsg2 != 0U) {
			xil_printf("[%s] Getting Cram scan configuration for SLR-%u:\n\r", \
					__func__, SlrCnt);

			/* Get NPI config for SLR */
			CramConfig = IpiResp.RespMsg2;

			/* Check if NPI scan is available on SLR */
			CramScanAvailable = XSem_DataMaskShift(CramConfig, \
					XSEM_CRAM_ENABLED_MASK, XSEM_CRAM_ENABLED_SHIFT);

			if (CramScanAvailable == 0U) {
				xil_printf("Cram scan is not enabled on SLR\n\r");
				continue;
			}

			/* Get Npi scan startup configuration */
			CramStartupConfig = XSem_DataMaskShift(CramConfig, \
					XSEM_STARTUP_CONFIG_MASK, \
					XSEM_STARTUP_CONFIG_SHIFT);
			if (CramStartupConfig == 0x00) {
				IsStartupImmediate = 0U;
				xil_printf("Cram scan is configured for deferred " \
						"startup\n\r");
			} else if (CramStartupConfig == 0x01) {
				IsStartupImmediate = 1U;
				xil_printf("Cram scan is configured for immediate " \
						"startup\n\r");
			} else {
				xil_printf("Error: Unknown Cram scan configuration " \
						"(0x%08x)\n\r",	CramStartupConfig);
			}

			/* Get Cram ECC selection configuration */
			CramEccSel = XSem_DataMaskShift(CramConfig,\
					XSEM_CRAM_ECC_SEL_MASK, XSEM_CRAM_ECC_SEL_SHIFT);

			if (CramEccSel == 0U) {
				xil_printf("Cram is configured to use HW ECC %ums\n\r", \
						CramEccSel);
			} else {
				xil_printf("Cram is configured to use SW ECC %ums\n\r", \
						CramEccSel);
			}

			/* Get Cram Correction configuration */
			CramCorEnabled = XSem_DataMaskShift(CramConfig,\
					XSEM_CRAM_COR_EN_MASK, XSEM_CRAM_COR_EN_SHIFT);

			if (CramCorEnabled == 0U) {
				xil_printf("Cram configured to not correct error%ums\n\r", \
						CramEccSel);
			} else {
				xil_printf("Cram configured to correct error %ums\n\r", \
						CramEccSel);
			}
		} else {
			xil_printf("[%s] Device not configured for Cram scan on Slr-%u" \
					"\n\r", __func__, SlrCnt);
		}
	}
END:
	return Status;
}


/******************************************************************************
 * @brief	This function is used to inject 1-bit correctable error in all SLRs
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus Xsem_CfrApiInjctCorErr()
{
	/* To inject correctable error, inject error in single bit position in
	 * any of the row/frame/qword. The argument details are
	 * ErrData[0]: Frame Address : 0, Quadword: 3, Bit position: 4, Row: 0
	 * Valid ranges: Row 0-3, Qword 0-24, Bit postion 0-127,
	 * For frame address, refer to the LAST_FRAME_TOP (CFRAME_REG) and
	 * LAST_FRAME_BOT (CFRAME_REG) registers.
	 * In Qword 12, 0-23 & 48-71 are syndrome bits. All other bits are data bits.
	 * To inject single bit error in syndrome, select Qword 12 in any of the
	 * frame with bit position in range of 0-23 or 48-71
	 */
	XSemCfrErrInjData ErrData = {0, 3, 4, 0};
	XStatus Status = 0U;

	Status = XSem_ApiCfrErrNjct(&ErrData);
	if(Status == XST_FAILURE){
		xil_printf("Error injection failed due to IPI failure\n\r");
	}

	return Status;
}

/******************************************************************************
 * @brief	This function is used to inject Uncorrectable error on all SLRs
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus Xsem_CfrApiInjctUnCorErr()
{
	/* To inject uncorrectable error, the injection has to be done in alternate
	 * bit positions in the same qword. The argument details are
	 * ErrData[0]: Frame Address : 0, Quadword: 0, Bit position: 2, Row: 0
	 * ErrData[1]: Frame Address : 0, Quadword: 0, Bit position: 4, Row: 0
	 */
	XSemCfrErrInjData ErrData[2] = {{0, 0, 2, 0},
					{0, 0, 4, 0},
					};
	XStatus Status = 0U;
	u32 Index = 0U;

	for(Index = 0U; Index < 2U; Index++){
		Status = XSem_ApiCfrErrNjct(&ErrData[Index]);
		if(Status == XST_FAILURE){
			xil_printf("Error injection failed due to" \
				" IPI failure\n\r");
			break;
		}
	}

	return Status;
}

/******************************************************************************
 * @brief	This function is used to inject CRC error on all SLRs.
 * 			This error is treated as an correctable error.
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus Xsem_CfrApiInjctCrcErr()
{
	/* To inject CRC error, the injection has to be done in alternate bit
     * positions of the same qword in 3 or more than 3 positions.
	 * The argument details are
	 * ErrData[0]: Frame Address : 0, Quadword: 0, Bit position: 0, Row: 0
	 * ErrData[1]: Frame Address : 0, Quadword: 0, Bit position: 2, Row: 0
	 * ErrData[2]: Frame Address : 0, Quadword: 0, Bit position: 4, Row: 0
	 * ErrData[3]: Frame Address : 0, Quadword: 0, Bit position: 6, Row: 0
	 */
	XSemCfrErrInjData ErrData[4] = {{0, 0, 0, 0},
					{0, 0, 2, 0},
					{0, 0, 4, 0},
					{0, 0, 6, 0},
					};
	XStatus Status = 0U;
	u32 Index = 0U;

	for(Index = 0U; Index < 4U; Index++){
		Status = XSem_ApiCfrErrNjct(&ErrData[Index]);
		if(Status == XST_FAILURE){
			xil_printf("Error injection failed due to" \
				" IPI failure\n\r");
			break;
		}
	}

	return Status;
}

/******************************************************************************
 * @brief	This function is used to Read frame ECC of a particular Frame on
 * 			all SLRs serially using XilSEM Client API
 * 			XSem_Ssit_CmdCfrReadFrameEcc
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus XSem_ApiCfrGetFrameEcc(u32 FrameAddr, u32 RowLoc)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Read frame ECC on all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrReadFrameEcc(&IpiInst, FrameAddr, RowLoc, \
				&IpiResp, SlrCnt);
		if ((XST_SUCCESS == Status) && \
				(CMD_ACK_SEM_READ_FRAME_ECC == IpiResp.RespMsg1)) {
			if ((XST_SUCCESS != IpiResp.RespMsg4) ){
				xil_printf("[%s] Error: Failed to get frame ECC with 0x%x on " \
						"SLR-%u\n", __func__, IpiResp.RespMsg4, SlrCnt);
				Status = (XStatus) IpiResp.RespMsg4;
			} else {
				xil_printf("[%s] Success: Get Frame ECC on SLR-%u\n\r", \
						__func__, SlrCnt);
				xil_printf("Ecc value for segment 0 of SLR-%u = 0x%08x\n\r", \
						SlrCnt, IpiResp.RespMsg2);
				xil_printf("Ecc value for segment 1 of SLR-%u = 0x%08x\n\r", \
						SlrCnt, IpiResp.RespMsg3);
			}
		} else {
			xil_printf("[%s] Error: Njct Status 0x%x Ack 0x%x Ret 0x%x " \
					"SLR-%u\n", __func__, Status, IpiResp.RespMsg1, \
					IpiResp.RespMsg2, IpiResp.RespMsg2, SlrCnt);
		}
	}
	return Status;
}

/******************************************************************************
 * @brief	This function is used to get golden CRC value of a particular Row
 * 			on all SLRs serially using XilSEM Client API XSem_Ssit_CmdCfrGetCrc
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus XSem_ApiCfrGetCrc(u32 RowIndex)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Inject Cram error in all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrGetCrc(&IpiInst, RowIndex, &IpiResp, SlrCnt);
		if ((XST_SUCCESS == Status) && \
				(CMD_ACK_CFR_GET_GLDN_CRC == IpiResp.RespMsg1)) {
			if ((XST_SUCCESS != IpiResp.RespMsg2) ) {
				xil_printf("[%s] Error: Failed to get golden CRC with 0x%x"
						" on SLR-%u\n", __func__, IpiResp.RespMsg2, SlrCnt);
				Status = (XStatus) IpiResp.RespMsg2;
			} else {
				xil_printf("[%s] Success: Get Crc on SLR-%u\n\r", \
						__func__, SlrCnt);
				xil_printf("Golden CRC of SLR-%u = 0x%08x\n\r", \
						SlrCnt, IpiResp.RespMsg5);
			}
		} else {
			xil_printf("[%s] Error: Njct Status 0x%x Ack 0x%x Ret 0x%x "
					"SLR-%u\n", \
					__func__, Status, IpiResp.RespMsg1, \
					IpiResp.RespMsg2, SlrCnt);
		}
	}
	return Status;
}

/***************************************************************************
 * @brief	Main showing example with usage of functions to StartScan,
 * 			StopScan & Error Inject on all SLRs of SSIT device
 *
 * @note	This test assumes that CRAM configuration is same for all SLRs
 **************************************************************************/

int main(void)
{
	XStatus Status = XST_SUCCESS;
	u32 CframeAddr = 0xFU;
	u32 RowLoc = 0U;
	u32 IsErrorInjected = 0U;

	/* Initialize IPI Driver
	 * This initialization is required to get XilSEM event notifications
	 * from PLM
	 */
	Status = XSem_IpiInitApi();
	if (XST_SUCCESS != Status) {
		xil_printf("Ipi init failure with status 0x%x\n\r", \
				__func__, Status);
		goto END;
	}

	/* Enable event Notifications on all SLRs to receive notifications
	 * from PLM upon detection of any error in CFRAME
	 */
	Xsem_CfrEventRegisterNotifier(1U);

	/* The following sequence demonstrates how to inject errors in CRAM
	 * 1. initialize scan if CRAM scan is configured for deferred startup
	 * 2. Stop scan
	 * 3. InjectError
	 * 4. StartScan
	 * 5. Wait for notification from PLM
	 * 6. Read Frame ECC of a particular frame
	 * 7. Read golden CRC for a particular frame
	 */

	/* Get CRAM configuration */
	Status = XSem_ApiCfrGetConfig();
	if (XST_SUCCESS != Status) {
		xil_printf("Get configuration Failed\n\r");
		goto END;
	}

	/* Initialize CRAM scan if not initialized
	 * This is applicable when CRAM Scan is set for deferred start up.
	 */
	if (IsStartupImmediate == 0U) {
		Status = XSem_ApiCfrInitCram_Broadcast();
		if (Status != XST_SUCCESS) {
			xil_printf("Cram scan Initialization failed\n\r");
			goto END;
		}

	}

	/* Read CRAM configuration of all SLRs over IPI */

	/* Stop Scan */
	Status = XSem_ApiCfrStopScan_Broadcast();
	if (XST_SUCCESS != Status) {
		xil_printf("Stop Scan Failed\n\r");
		goto END;
	}

	/* Error injection : This example demonstrates how to inject
	 * uncorrectable error in each SLR. If you want to inject CRC error,
	 * you need to replace the function call Xsem_CfrApiInjctUnCorErr
	 * with Xsem_CfrApiInjctCrcErr.
	 * If you want to inject correctable error replace the function call
	 * with Xsem_CfrApiInjctCorErr
	 */
	Status = Xsem_CfrApiInjctCorErr();
	if (Status == XST_SUCCESS) {
		IsErrorInjected = 1U;
	}

	/* Start Scan to enable CRAM to detect the injected error.
	 * In case of CRC or uncorrectable error, XilSEM stops the scan
	 * and enters into idle state which means no further scan will be
	 * performed.
	 * In case of correctable error, XilSEM corrects the error
	 * if the correction is enabled in the configuration. Else, it will
	 * report the error and enters into idle state.
	 */
	Status = XSem_ApiCfrStartScan_Broadcast();
	if (XST_SUCCESS != Status) {
		xil_printf("Cram Start Scan Failed\n\r");
		goto END;
	}

	/*Wait for the error to be detected*/
	sleep(2);

	if (IsErrorInjected == 1U) {
		/* Replace EventCnt_CorEcc with EventCnt_UnCorEcc if testing for
		 * Uncorrectable errors */
		if (EventCnt_CorEcc == 1) {
			xil_printf("Success: Error event detected\n\r");
		} else {
			xil_printf("Failure: Error event not detected\n\r");
			goto END;
		}
	}

	/* Read Frame ECC values of a particular frame over IPI */
	Status = XSem_ApiCfrGetFrameEcc(CframeAddr, RowLoc);
	if (XST_SUCCESS != Status) {
		xil_printf("Failed to read Frame ECC\n\r");
		goto END;
	}

	/* Read Frame ECC values of a particular frame over IPI */
	Status = XSem_ApiCfrGetCrc(RowLoc);
	/* Read Golden ECC for Row 0 for each SLR */
	if (XST_SUCCESS != Status) {
			xil_printf("Failed to read Golden ECC\n\r");
	}
END:
	return 0;
}
