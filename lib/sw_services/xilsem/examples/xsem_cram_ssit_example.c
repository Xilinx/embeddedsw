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
 * Ver  Who        Date        Changes
 * ---- ----     ----------  --------------------------------------------------
 * 0.1  hb       07/03/2022  Initial Creation
 * 0.2	gupta    07/18/2022  Added cram get status API and updated example
 * 0.3	gupta    08/15/2022  Updated broad cast APIs to check status of all SLRs
 *							 seperately
 * </pre>
 *
 *****************************************************************************/
#include "xil_cache.h"
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
#define XSEM_STARTUP_CONFIG_MASK	(0x00000060U)
#define XSEM_STARTUP_CONFIG_SHIFT	(5U)
#define XSEM_CUR_SLR_MASK		(0x00000001U)
#define XSem_DataMaskShift(Data, Mask, Shift)	(((Data) & (Mask)) >> (Shift))

/* CRAM STATUS register values */
#define CFR_STATUS_INIT			(0x10005U) /* Status after cram init */
#define CFR_STATUS_IDLE 		(0x10011U) /* Status after StopScan */
#define CFR_STATUS_UNCOR_ECC 		(0x10211U) /* Status after Uncor Ecc */
#define CFR_STATUS_CRC 			(0x10411U) /* Status after Crc detection */
#define CFR_STATUS_COR_ECC 		(0x14805U) /* Status after cor ecc */
/* Status after CorEcc detection */
#define CFR_STATUS_DETECT_ONLY_COR_ECC		(0x0001C811U)

static XIpiPsu IpiInst;
static XScuGic GicInst;
static XSemIpiResp IpiResp = {0U};

XSem_Notifier Notifier = {
        .Module = XSEM_NOTIFY_CRAM,
        .Event = XSEM_EVENT_CRAM_UNCOR_ECC_ERR | XSEM_EVENT_CRAM_CRC_ERR | \
		 XSEM_EVENT_CRAM_INT_ERR | XSEM_EVENT_CRAM_COR_ECC_ERR,
	.Flag = 1U,
};

/*Global variables to hold the event count when notified*/
u8 EventCnt_UnCorEcc[XSEM_SSIT_MAX_SLR_CNT] = {0U};
u8 EventCnt_Crc[XSEM_SSIT_MAX_SLR_CNT] = {0U};
u8 EventCnt_CorEcc[XSEM_SSIT_MAX_SLR_CNT] = {0U};
u8 EventCnt_IntErr[XSEM_SSIT_MAX_SLR_CNT] = {0U};
u8 EventFromSlr = 0xFFU;

u8 TestID = 0xFFU;
static u32 IsStartupImmediate = 0U;
static u32 CramCorEnabled = 0U;

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
 *		master PLM. Master PLM can receive notifications from slave
 *		SLRs and forward to client
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

	EventFromSlr = Payload[3];

	if ((XSEM_EVENT_ERROR == Payload[0]) && \
			(XSEM_NOTIFY_CRAM == Payload[1])) {
		if (XSEM_EVENT_CRAM_UNCOR_ECC_ERR == Payload[2]) {
			++EventCnt_UnCorEcc[EventFromSlr];
			xil_printf("[ALERT] Received uncorrectable error" \
				" notification from XilSEM on SLR-%u\n\r", \
				EventFromSlr);
		} else if (XSEM_EVENT_CRAM_CRC_ERR == Payload[2]) {
			++EventCnt_Crc[EventFromSlr];
			xil_printf("[ALERT] Received CRC error event" \
				" notification from XilSEM on SLR-%u\n\r", \
				EventFromSlr);
		} else if (XSEM_EVENT_CRAM_INT_ERR == Payload[2]) {
			++EventCnt_IntErr[EventFromSlr];
			xil_printf("[ALERT] Received internal error" \
				" notification from XilSEM on SLR-%u\n\r", \
				EventFromSlr);
		} else if (XSEM_EVENT_CRAM_COR_ECC_ERR == Payload[2]) {
			++EventCnt_CorEcc[EventFromSlr];
			xil_printf("[ALERT] Received correctable error" \
				" notification from XilSEM on SLR-%u\n\r", \
				EventFromSlr);
		} else {
			xil_printf("%s Some other callback received: %d:%d:%d"\
				" from SLR-%u\n", __func__, Payload[0], \
				Payload[1], Payload[2], EventFromSlr);
		}
	} else {
		xil_printf("%s Some other callback received: %d from " \
			"SLR-%u\n", __func__, Payload[0], EventFromSlr);
	}
}

/*****************************************************************************
 *
 * @brief	This function initializes CRAM scan in one SLR in SSIT device
 * 		using XilSEM Client API XSem_Ssit_CmdCfrInit
 *
 *
 * @return    	XST_SUCCESS : upon successful initialization of CRAM
 *  		XST_FAILURE : any failure in initialization of CRAM.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrInitCram_InOneSlr(u32 TargetSlr)
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrInit(&IpiInst, &IpiResp, \
			TargetSlr);
	if ((XST_SUCCESS == Status) && (CMD_ACK_CFR_INIT == IpiResp.RespMsg1) \
			&& (XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Init on SLR-%u\n\r", \
			__func__, TargetSlr);
	} else {
		xil_printf("[%s] Error: Cfr Init Status 0x%x Ack 0x%x " \
			"Ret 0x%x\n", __func__, Status, IpiResp.RespMsg1, \
			IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 *
 * @brief	This function initializes CRAM scan on all SLRs in SSIT device
 * 		using XilSEM Client API XSem_Ssit_CmdCfrInit
 *
 *
 * @return    	XST_SUCCESS : upon successful initialization of CRAM
 *  		XST_FAILURE : any failure in initialization of CRAM.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrInitCram_Broadcast()
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrInit(&IpiInst, &IpiResp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && (CMD_ACK_CFR_INIT == IpiResp.RespMsg1) \
			&& (XST_SUCCESS == IpiResp.RespMsg2) &&
			(XST_SUCCESS == IpiResp.RespMsg3) &&
			(XST_SUCCESS == IpiResp.RespMsg4 &&
			(XST_SUCCESS == IpiResp.RespMsg5))) {
		xil_printf("[%s] Success: Cfr Init on all SLRs\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Cfr Init Status 0x%x Ack 0x%x \n", \
			__func__, Status, IpiResp.RespMsg1);

		/*Check If RespMsg2 is success or not */
		if (IpiResp.RespMsg2 != XST_SUCCESS){
			/* If not success Cfr Init is failed in master SLR */
			xil_printf("[%s] Cfr Init failed on SLR-0 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg2);
		}
		/*Check If RespMsg3 is success or not */
		if (IpiResp.RespMsg3 != XST_SUCCESS){
			/* If not success Cfr Init is failed in Slave 1 */
			xil_printf("[%s] Cfr Init failed on SLR-1 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg3);
		}
		/*Check If RespMsg4 is success or not */
		if (IpiResp.RespMsg4 != XST_SUCCESS){
			/* If not success Cfr Init is failed in Slave 2 */
			xil_printf("[%s] Cfr Init failed on SLR-2 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg4);
		}
		/*Check If RespMsg5 is success or not */
		if (IpiResp.RespMsg5 != XST_SUCCESS){
			/* If not success Cfr Init is failed in Slave 3 */
			xil_printf("[%s] Cfr Init failed on SLR-3 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg5);
		}
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function starts Cfr scan in one SLR of SSIT device
 *		using XilSEM Client API XSem_Ssit_CmdCfrStopScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_Ssit_ApiCfrStartScan_InOneSlr(u32 TargetSlr)
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrStartScan(&IpiInst, &IpiResp, \
			TargetSlr);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_START_SCAN == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Start scan on Slr-%u\n\r", \
			__func__, TargetSlr);
	} else {
		xil_printf("[%s] Error: Cfr Start scan on Slr-%u. Status 0x%x "
			"Ack 0x%x Ret 0x%x SlrFailMask 0x%x\n", __func__, \
			TargetSlr, Status, IpiResp.RespMsg1, IpiResp.RespMsg2,\
			IpiResp.RespMsg3);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function starts Cfr scan on all SLRs in SSIT device
 *		using XilSEM Client API XSem_Ssit_CmdCfrStartScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrStartScan_Broadcast()
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrStartScan(&IpiInst, &IpiResp, \
		XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && (CMD_ACK_CFR_START_SCAN == IpiResp.RespMsg1) \
			&& (XST_SUCCESS == IpiResp.RespMsg2) &&
			(XST_SUCCESS == IpiResp.RespMsg3) &&
			(XST_SUCCESS == IpiResp.RespMsg4 &&
			(XST_SUCCESS == IpiResp.RespMsg5))) {
		xil_printf("[%s] Success: Cfr StartScan on all SLRs\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Cfr StartScan Status 0x%x Ack 0x%x \n", \
			__func__, Status, IpiResp.RespMsg1);

		/*Check If RespMsg2 is success or not */
		if (IpiResp.RespMsg2 != XST_SUCCESS){
			/* If not success Cfr StartScan is failed in master SLR */
			xil_printf("[%s] Cfr StartScan failed on SLR-0 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg2);
		}
		/*Check If RespMsg3 is success or not */
		if (IpiResp.RespMsg3 != XST_SUCCESS){
			/* If not success Cfr StartScan is failed in Slave 1 */
			xil_printf("[%s] Cfr StartScan failed on SLR-1 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg3);
		}
		/*Check If RespMsg4 is success or not */
		if (IpiResp.RespMsg4 != XST_SUCCESS){
			/* If not success Cfr StartScan is failed in Slave 2 */
			xil_printf("[%s] Cfr StartScan failed on SLR-2 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg4);
		}
		/*Check If RespMsg5 is success or not */
		if (IpiResp.RespMsg5 != XST_SUCCESS){
			/* If not success Cfr StartScan is failed in Slave 3 */
			xil_printf("[%s] Cfr StartScan failed on SLR-3 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg5);
		}
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function stops Cfr scan in one SLR of SSIT device
 *		using XilSEM Client API XSem_Ssit_CmdCfrStopScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_Ssit_ApiCfrStopScan_InOneSlr(u32 TargetSlr)
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrStopScan(&IpiInst, &IpiResp, \
			TargetSlr);
	if ((XST_SUCCESS == Status) && \
		(CMD_ACK_CFR_STOP_SCAN == IpiResp.RespMsg1) && \
		(XST_SUCCESS == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Cfr Stop scan on Slr-%u\n\r", \
			__func__, TargetSlr);
	} else {
		xil_printf("[%s] Error: Cfr Stop scan on Slr-%u. Status 0x%x "
			"Ack 0x%x Ret 0x%x SlrFailMask 0x%x\n", __func__, \
			TargetSlr, Status, IpiResp.RespMsg1, IpiResp.RespMsg2,\
			IpiResp.RespMsg3);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function stops Cfr scan on all SLRs in SSIT device
 *		using XilSEM Client API XSem_Ssit_CmdCfrStopScan
 *
 * @return    	XST_SUCCESS : upon successful completion of stop scan.
 *    		XST_FAILURE : any failure in stop scan.
 *
 ******************************************************************************/
static XStatus XSem_ApiCfrStopScan_Broadcast()
{
	XStatus Status = XST_FAILURE;

	Status = XSem_Ssit_CmdCfrStopScan(&IpiInst, &IpiResp, \
			XSEM_SSIT_ALL_SLRS_ID);
	if ((XST_SUCCESS == Status) && (CMD_ACK_CFR_STOP_SCAN == IpiResp.RespMsg1) \
			&& (XST_SUCCESS == IpiResp.RespMsg2) &&
			(XST_SUCCESS == IpiResp.RespMsg3) &&
			(XST_SUCCESS == IpiResp.RespMsg4 &&
			(XST_SUCCESS == IpiResp.RespMsg5))) {
		xil_printf("[%s] Success: Cfr StopScan on all SLRs\n\r", __func__);
	} else {
		xil_printf("[%s] Error: Cfr StopScan Status 0x%x Ack 0x%x \n", \
			__func__, Status, IpiResp.RespMsg1);

		/*Check If RespMsg2 is success or not */
		if (IpiResp.RespMsg2 != XST_SUCCESS){
			/* If not success Cfr StopScan is failed in master SLR */
			xil_printf("[%s] Cfr StopScan failed on SLR-0 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg2);
		}
		/*Check If RespMsg3 is success or not */
		if (IpiResp.RespMsg3 != XST_SUCCESS){
			/* If not success Cfr StopScan is failed in Slave 1 */
			xil_printf("[%s] Cfr StopScan failed on SLR-1 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg3);
		}
		/*Check If RespMsg4 is success or not */
		if (IpiResp.RespMsg4 != XST_SUCCESS){
			/* If not success Cfr StopScan is failed in Slave 2 */
			xil_printf("[%s] Cfr StopScan failed on SLR-2 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg4);
		}
		/*Check If RespMsg5 is success or not */
		if (IpiResp.RespMsg5 != XST_SUCCESS){
			/* If not success Cfr StopScan is failed in Slave 3 */
			xil_printf("[%s] Cfr StopScan failed on SLR-3 with error"
					" code = 0x%08x\n\r", __func__, IpiResp.RespMsg5);
		}
		Status = XST_FAILURE;
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	This function gets status of any SLR in SSIT device
 * 		using XilSEM Client API XSem_Ssit_CmdCfrGetStatus
 *
 * @param[in]	TargetSlr :	target slr index
 * @param[out]	CfrStatusInfo	:	Cram status structure pointer
 *
 * @return    	XST_SUCCESS : upon successful completion of start scan.
 *    		XST_FAILURE : any failure in start scan.
 *
 ******************************************************************************/
static XStatus XSem_Ssit_ApiCfrGetStatusSlr(u32 TargetSlr, \
		XSemCfrStatus *CfrStatusInfo)
{
	XStatus Status = XST_FAILURE;
	XSemStatus StatusInfo = {0U};
	u32 Index = 0U;

	Status = XSem_Ssit_CmdGetStatus(&IpiInst, &IpiResp, TargetSlr, \
			&StatusInfo);
	if ((XST_SUCCESS == Status) && \
		(CMD_ACK_CFR_GET_STATUS == IpiResp.RespMsg1) && \
		(TargetSlr == IpiResp.RespMsg2)) {
		xil_printf("[%s] Success: Getting Cfr Status of SLR-%u\n\r", \
			__func__, TargetSlr);
		/* update data in to [out] param cram structure */
		CfrStatusInfo->Status = StatusInfo.CramStatus;
		for(Index = 0U; Index < MAX_CRAMERR_REGISTER_CNT; Index++) {
			CfrStatusInfo->ErrAddrH[Index] = StatusInfo.ErrAddrH[Index];
			CfrStatusInfo->ErrAddrL[Index] = StatusInfo.ErrAddrL[Index];
		}
		CfrStatusInfo->ErrCorCnt = StatusInfo.ErrCorCnt;
	} else {
		xil_printf("[%s] Error: Getting Cfr Status. Status 0x%x "
			"Ack 0x%x SlrFailMask 0x%x\n", __func__, Status, \
				IpiResp.RespMsg1, IpiResp.RespMsg2);
		Status = XST_FAILURE;
	}

	return Status;
}

/*****************************************************************************
 *
 * @brief    	Function to inject error in CFRAME on all SLRs using XilSEM
 * 		Client API XSem_Ssit_CmdCfrNjctErr
 *
 * @param    	ErrData : Pointer to Error details
 *
 * @return    	XST_SUCCESS : upon successful error injection.
 *   		XST_FAILURE : Any failure IPI interface
 *     		Error Codes : Below error codes upon failure during injection
 *		0x00500000U : If Cfr Scan is not initialized
 * 		0X00002000U : On getting Cframe driver instance as Null
 * 		0x00800000U : If invalid row is passed as input
 * 		0x00E00000U : If invalid block type is passed as input
 * 		0x00A00000U : If invalid bit is passed as input
 * 		0x00B00000U : If invalid frame is passed as input
 * 		0x00900000U : If invalid qword is passed as input
 * 		0x00D00000U : If input bit is masked
 * 		0x00C00000U : If injection is failed
 *****************************************************************************/
static XStatus XSem_ApiCfrErrNjct_OnAllSLRs(XSemCfrErrInjData *ErrData)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Inject Cram error in all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrNjctErr(&IpiInst, ErrData, &IpiResp,\
			SlrCnt);
		if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_NJCT_ERR == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg2)) {
			xil_printf("[%s] Success: Inject on SLR-%u\n\r", \
				__func__, SlrCnt);
		} else {
			xil_printf("[%s] Error: Injection Failed with 0x%x" \
				"on SLR-%u \n", __func__, IpiResp.RespMsg2, \
				SlrCnt);
			xil_printf("[%s] Error: Njct Status 0x%x Ack 0x%x \n",\
				__func__, Status, IpiResp.RespMsg1);
			Status = (XStatus) IpiResp.RespMsg2;
		}
		xil_printf("--------------------------------------------"\
			"---\n\r");
	}
	return Status;
}

/*****************************************************************************
 *
 * @brief    	Function to inject error in CFRAME in any one SLR using XilSEM
 * 		Client API XSem_Ssit_CmdCfrNjctErr
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
XStatus XSem_Ssit_ApiCfrErrNjct_InOneSlr(XSemCfrErrInjData *ErrData, \
		u32 TargetSlr)
{
	XStatus Status = XST_FAILURE;

	/* Inject Cram error in Target Slr */

	Status = XSem_Ssit_CmdCfrNjctErr(&IpiInst, ErrData, &IpiResp, TargetSlr);
	if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_NJCT_ERR == IpiResp.RespMsg1)) {
		if ((XST_SUCCESS != IpiResp.RespMsg2) ){
			xil_printf("[%s] Error: Injection Failed with 0x%x" \
				"on SLR-%u \n", __func__, IpiResp.RespMsg2, \
				TargetSlr);
			Status = (XStatus) IpiResp.RespMsg2;
		} else {
			xil_printf("[%s] Success: Inject on SLR-%u\n\r", \
				__func__, TargetSlr);
		}
	} else {
		xil_printf("[%s] Error: Njct Status 0x%x Ack 0x%x Ret 0x%x\n",\
			__func__, Status, IpiResp.RespMsg1, IpiResp.RespMsg2);
	}

	return Status;
}

/*****************************************************************************
 * @brief	This function gets Cram scan configuration of each SLR serially
 * 		using XilSEM Client API XSem_Ssit_CmdGetConfig
 *
 * @return	XST_SUCCESS : upon successful retrieval of Cram scan
 * 			      configuration
 * 		XST_FAILURE : any failure in retrieval of CRAM scan
 * 			      configuration
 *
 *****************************************************************************/
static XStatus XSem_ApiCfrGetConfig(u32 *CramConfig)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;
	u32 CramStartupConfig = 0U;
	u32 CramScanAvailable = 0U;
	u32 CramEccSel = 0U;

	/* Check CRAM scan configuration on each SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdGetConfig(&IpiInst, &IpiResp, SlrCnt);
		if (Status != XST_SUCCESS) {
			xil_printf("%s Failed to get CRAM configuration\n\r", \
				__func__);
			goto END;
		}
		/* Check if CRAM is configured on current SLR */
		if (IpiResp.RespMsg2 != 0U) {
			xil_printf("[%s] Getting Cram scan configuration for" \
				"SLR-%u:\n\r", __func__, SlrCnt);
			xil_printf("---------------------------------------"\
				"--------\n\r");
			/* Get CRAM config for SLR */
			CramConfig[SlrCnt] = IpiResp.RespMsg2;

			/* Check if CRAM scan is available on SLR */
			CramScanAvailable = \
				XSem_DataMaskShift(CramConfig[SlrCnt], \
				XSEM_CRAM_ENABLED_MASK, \
				XSEM_CRAM_ENABLED_SHIFT);

			if (CramScanAvailable == 0U) {
				xil_printf("Cram scan is not enabled on" \
					"SLR-%u\n\r", SlrCnt);
				continue;
			}

			/* Get CRAM scan startup configuration */
			CramStartupConfig = \
				XSem_DataMaskShift(CramConfig[SlrCnt], \
				XSEM_STARTUP_CONFIG_MASK, \
				XSEM_STARTUP_CONFIG_SHIFT);
			if (CramStartupConfig == 0x00U) {
				IsStartupImmediate = 0U;
				xil_printf("Cram scan is configured for" \
					"deferred startup\n\r");
			} else if (CramStartupConfig == 0x01U) {
				IsStartupImmediate = 1U;
				xil_printf("Cram scan is configured for" \
					"immediate startup\n\r");
			} else {
				xil_printf("Error: Unknown Cram scan startup" \
					"configuration (0x%08x)\n\r", \
					CramStartupConfig);
			}

			/* Get Cram ECC selection configuration */
			CramEccSel = XSem_DataMaskShift(CramConfig[SlrCnt],\
				XSEM_CRAM_ECC_SEL_MASK, \
				XSEM_CRAM_ECC_SEL_SHIFT);

			if (CramEccSel == 0U) {
				xil_printf("Cram is configured to use" \
					" HW ECC \n\r");
			} else {
				xil_printf("Cram is configured to use" \
					"SW ECC \n\r");
			}

			/* Get Cram Correction configuration */
			CramCorEnabled = XSem_DataMaskShift(CramConfig[SlrCnt],\
				XSEM_CRAM_COR_EN_MASK, XSEM_CRAM_COR_EN_SHIFT);

			if (CramCorEnabled == 0U) {
				xil_printf("Cram configured to not correct" \
					"error\n\r");
			} else {
				xil_printf("Cram configured to correct" \
					"error\n\r");
			}
		} else {
			xil_printf("[%s] Device not configured for Cram scan"\
				" on Slr-%u \n\r", __func__, SlrCnt);
		}
		xil_printf("-------------------------------------"\
			"----------\n\r");
	}
END:
	return Status;
}


/******************************************************************************
 * @brief	This function is used to inject 1-bit correctable error
 * 		in all SLRs
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
	 * In Qword 12, 0-23 & 48-71 are syndrome bits. All other bits are data
	 * bits.
	 * To inject single bit error in syndrome, select Qword 12 in any of
	 * the frame with bit position in range of 0-23 or 48-71
	 */
	XSemCfrErrInjData ErrData = {0, 3, 4, 0};
	XStatus Status = 0U;

	/* Set TestID for correctable Error test */
	TestID = 0U;

	Status = XSem_ApiCfrErrNjct_OnAllSLRs(&ErrData);
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
	/* To inject uncorrectable error, the injection has to be done in
	 * alternate bit positions in the same qword. The argument details are
	 * ErrData[0]: Frame Address : 1, Quadword: 2, Bit position: 3, Row: 1
	 * ErrData[1]: Frame Address : 1, Quadword: 2, Bit position: 5, Row: 1
	 */
	XSemCfrErrInjData ErrData[2] = {{1, 2, 3, 1},
					{1, 2, 5, 1}};
	XStatus Status = 0U;
	u32 Index = 0U;

	/* Set TestID for Uncorrectable Error test */
	TestID = 1U;

	for(Index = 0U; Index < 2U; Index++){
		Status = XSem_ApiCfrErrNjct_OnAllSLRs(&ErrData[Index]);
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
 * 		This error is treated as an correctable error.
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

	/* Set TestID for crc Error test */
	TestID = 2U;

	for(Index = 0U; Index < 4U; Index++){
		Status = XSem_ApiCfrErrNjct_OnAllSLRs(&ErrData[Index]);
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
 * 		all SLRs serially using XilSEM Client API
 * 		XSem_Ssit_CmdCfrReadFrameEcc
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus XSem_ApiCfrGetFrameEcc(u32 FrameAddr, u32 RowLoc)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Read frame ECC on all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrReadFrameEcc(&IpiInst, FrameAddr, \
			RowLoc, &IpiResp, SlrCnt);
		if ((XST_SUCCESS == Status) && \
			(CMD_ACK_SEM_READ_FRAME_ECC == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg4)) {
			xil_printf("[%s] Success: Get Frame ECC on" \
				"SLR-%u\n\r", __func__, SlrCnt);
			xil_printf("Ecc value for segment 0 of "\
				"SLR-%u = 0x%08x\n\r", SlrCnt, IpiResp.RespMsg2);
			xil_printf("Ecc value for segment 1 of SLR-%u ="\
				" 0x%08x\n\r", SlrCnt, IpiResp.RespMsg3);
			Status = XST_SUCCESS;

		} else {
			xil_printf("[%s] Error: Frame Ecc Status 0x%x Ack "\
				"0x%x Ret 0x%x SLR-%u\n", __func__, Status, \
				IpiResp.RespMsg1, IpiResp.RespMsg4, SlrCnt);
			xil_printf("[%s] Error: Failed to get frame ECC with"\
				" error code 0x%x on SLR-%u\n", __func__, \
				IpiResp.RespMsg4, SlrCnt);
			Status = (XStatus) IpiResp.RespMsg4;
		}
		xil_printf("----------------------------------------"\
			"-------\n\r");
	}
	return Status;
}

/******************************************************************************
 * @brief	This function is used to get golden CRC value of a particular
 * 		Row on all SLRs serially using XilSEM Client API
 * 		XSem_Ssit_CmdCfrGetCrc
 *
 * @return	Status : Success or Failure
 *****************************************************************************/
static XStatus XSem_ApiCfrGetCrc(u32 RowIndex)
{
	XStatus Status = XST_FAILURE;
	u32 SlrCnt;

	/* Inject Cram error in all SLRs serially */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		Status = XSem_Ssit_CmdCfrGetCrc(&IpiInst, RowIndex, \
			&IpiResp, SlrCnt);
		if ((XST_SUCCESS == Status) && \
			(CMD_ACK_CFR_GET_GLDN_CRC == IpiResp.RespMsg1) && \
			(XST_SUCCESS == IpiResp.RespMsg6)) {
			xil_printf("[%s] Success: Get Crc on SLR-%u\n\r", \
				__func__, SlrCnt);
			xil_printf("Golden CRC of SLR-%u = 0x%08x\n\r", \
				SlrCnt, IpiResp.RespMsg4);
		} else {
			xil_printf("[%s] Error: Golden CRC Status 0x%x \
				""Ack 0x%x Ret 0x%x SLR-%u\n", \
				__func__, Status, IpiResp.RespMsg1, \
				IpiResp.RespMsg6, SlrCnt);
			xil_printf("[%s] Error: Failed to get golden"\
				" CRC with error code 0x%x on SLR-%u\n", \
				__func__, IpiResp.RespMsg6, SlrCnt);
			Status = (XStatus) IpiResp.RespMsg6;
		}
		xil_printf("-------------------------------------"\
			"----------\n\r");
	}
	return Status;
}

/***************************************************************************
 * @brief	Main showing example with usage of functions to StartScan,
 * 		StopScan & Error Inject on all SLRs of SSIT device
 *
 * @note	This test assumes that CRAM configuration is same for all
 * 		SLRs
 **************************************************************************/
int main(void)
{
	XStatus Status = XST_SUCCESS;
	u32 CframeAddr = 0xFU;
	u32 RowLoc = 0U;
	u32 CramConfig[XSEM_SSIT_MAX_SLR_CNT] = {0};
	u32 SlrCnt = 0U;
	XSemCfrStatus CfrStatusInfo = {0};
	Xil_DCacheDisable();
	u32 IntialCorErrCnt[XSEM_SSIT_MAX_SLR_CNT] = {0U};
	u32 FailCnt = 0U;
	u32 IncreaseInCorErrCnt = 0U;

	/* Initialize IPI Driver
	 * This initialization is required to get XilSEM event notifications
	 * from PLM
	 */
	Status = XSem_IpiInitApi();
	if (XST_SUCCESS != Status) {
		xil_printf("Ipi init failure with status 0x%x\n\r", \
				__func__, Status);
		++FailCnt;
		goto END;
	}
	xil_printf("==================================================\n\r");
	/* Enable event Notifications on all SLRs to receive notifications
	 * from PLM upon detection of any error in CFRAME
	 */
	Xsem_CfrEventRegisterNotifier(1U);

	/* Read CRAM configuration of all SLRs over IPI */
	/* Get CRAM configuration */
	Status = XSem_ApiCfrGetConfig(CramConfig);
	if (XST_SUCCESS != Status) {
		xil_printf("Get configuration Failed\n\r");
		++FailCnt;
		goto END;
	}
	xil_printf("==================================================\n\r");
	/* Initialize CRAM scan if not initialized
	 * This is applicable when CRAM Scan is set for deferred start up.
	 */
	if (IsStartupImmediate == 0U) {
		Status = XSem_ApiCfrInitCram_Broadcast();
		if (Status != XST_SUCCESS) {
			xil_printf("Cram scan Initialization failed\n\r");
			++FailCnt;
			goto END;
		}
	}
	xil_printf("==================================================\n\r");
	/* Read Frame ECC values of a particular frame over IPI */
	Status = XSem_ApiCfrGetFrameEcc(CframeAddr, RowLoc);
	if (XST_SUCCESS != Status) {
		xil_printf("Failed to read Frame ECC\n\r");
		++FailCnt;
		goto END;
	}
	xil_printf("==================================================\n\r");
	/* Read Frame ECC values of a particular frame over IPI */
	Status = XSem_ApiCfrGetCrc(RowLoc);
	/* Read Golden ECC for Row 0 for each SLR */
	if (XST_SUCCESS != Status) {
			xil_printf("Failed to read Golden ECC\n\r");
			++FailCnt;
	}
	xil_printf("==================================================\n\r");

	/* Get Status and capture initial error count of all SLRs */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++){
		Status = XSem_Ssit_ApiCfrGetStatusSlr(SlrCnt, &CfrStatusInfo);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] Cram Get Status failed\n", __func__);
			++FailCnt;
			goto END;
		}
		IntialCorErrCnt[SlrCnt] = CfrStatusInfo.ErrCorCnt;
	}

	xil_printf("==================================================\n\r");
	/**
	 * The following sequence demonstrates how to inject errors in CRAM and
	 * verify CRAM scan behavior
	 * 1. Initialize scan if CRAM scan of all SLRs (in case of deferred start)
	 * 2. Stop scan of all SLRs using stop scan Broadcast IPI command
	 * 3. Inject 1-bit error in all the SLRs (Repeat this step for injecting
	 *    multiple errors)
	 * 4. Start scan of any one SLR
	 * 5. Wait for error event notification from PLM
	 * 6. After receiving notification then repeat 4 & 5 steps for next SLR
	 */

	/* Stop Scan */
	Status = XSem_ApiCfrStopScan_Broadcast();
	if (XST_SUCCESS != Status) {
		xil_printf("Stop Scan Failed\n\r");
		++FailCnt;
		goto END;
	}
	xil_printf("==================================================\n\r");

	/* Error injection : This example demonstrates how to inject
	 * uncorrectable error in each SLR. If you want to inject CRC error,
	 * you need to replace the function call Xsem_CfrApiInjctUnCorErr
	 * with Xsem_CfrApiInjctCrcErr.
	 * If you want to inject correctable error replace the function call
	 * with Xsem_CfrApiInjctCorErr
	 */
	Status = Xsem_CfrApiInjctCorErr();
	if (Status != XST_SUCCESS) {
		xil_printf("Error Injection Failed\n\r");
		++FailCnt;
		goto END;
	}
	xil_printf("==================================================\n\r");
	/**
	 * Start Scan to enable CRAM to detect the injected error.
	 * In case of CRC or uncorrectable error, XilSEM stops the scan
	 * and enters into idle state which means no further scan will be
	 * performed.
	 * In case of correctable error, XilSEM corrects the error
	 * if the correction is enabled in the configuration. Else, it will
	 * report the error and enters into idle state.
	 */

	/**
	 * Start Cram scan of one SLR, Wait for the event notification
	 * till timeout, repeat for all SLRs
	 */
	for (SlrCnt = 0U; SlrCnt < XSEM_SSIT_MAX_SLR_CNT; SlrCnt++) {
		/* Start scan */
		Status = XSem_Ssit_ApiCfrStartScan_InOneSlr(SlrCnt);
		if (XST_SUCCESS != Status) {
			xil_printf("Cram Start Scan Failed\n\r");
			++FailCnt;
			goto END;
		}

		/*Wait for the error to be detected*/
		sleep(2);

		/* Get Cfr Status */
		Status = XSem_Ssit_ApiCfrGetStatusSlr(SlrCnt, &CfrStatusInfo);
		if (XST_SUCCESS != Status) {
			xil_printf("[%s] Cram Get Status failed\n", __func__);
			++FailCnt;
			goto END;
		}
		/* Calculate increase in correctable error count */
		IncreaseInCorErrCnt = CfrStatusInfo.ErrCorCnt - \
			IntialCorErrCnt[SlrCnt];

		switch(TestID){
		case 0:
			/**
			 * Check if the event is received and it is from current Slr
			 */
			if ((EventCnt_CorEcc[SlrCnt] == 1U) && \
				(EventFromSlr == SlrCnt)) {
				xil_printf("Success: Correctable Error event"\
					" received\n\r");
				/* Clear Event count to check next event */
				--EventCnt_CorEcc[SlrCnt];
			} else {
				xil_printf("Failure: Correctable Error event"\
					" not received\n\r");
				++FailCnt;
			}

			/* Check if correction enabled */
			if (CramCorEnabled == 1U){
				/* Check Cram status */
				if(CfrStatusInfo.Status == CFR_STATUS_COR_ECC)
				{
					xil_printf("Success: Cfr is set to "\
						"expected status. Status = "\
						"0x%08x\n\r", \
						CfrStatusInfo.Status);
				} else {
					xil_printf("Failure: Cfr set to "\
						"unexpected status. Status"\
						" = 0x%08x\n\r", \
						CfrStatusInfo.Status);
					++FailCnt;
				}

				/* Check Cram Cor Err count */
				if(IncreaseInCorErrCnt == 1U)
				{
					xil_printf("Success: correctable "\
						"Error count increased as"\
						" expected\n\r");
				} else {
					xil_printf("Failure: correctable "\
						"Error count. "\
						"IncreaseInCorErrCnt = "\
						"0x%08x\n\r", \
						IncreaseInCorErrCnt);
					++FailCnt;
				}
			} else {
				/* Check Cram status */
				if(CfrStatusInfo.Status == \
					CFR_STATUS_DETECT_ONLY_COR_ECC)
				{
					xil_printf("Success: Cfr is set to "\
						"expected status. Status = "\
						"0x%08x\n\r", \
						CfrStatusInfo.Status);
				} else {
					xil_printf("Failure: Cfr set to "\
						"unexpected status. Status "\
						"= 0x%08x\n\r", \
						CfrStatusInfo.Status);
					++FailCnt;
				}

				/* Check Cram Cor Err count */
				if(IncreaseInCorErrCnt == 0U)
				{
					xil_printf("Success: correctable "\
						"Error count remained same as"
						" expected\n\r");
				} else {
					xil_printf("Failure: correctable "\
						"Error count. "\
						"IncreaseInCorErrCnt"\
						" = 0x%08x\n\r", \
						IncreaseInCorErrCnt);
					++FailCnt;
				}
			}
			break;
		case 1:
			/**
			 * Check if the event is received and it is from current Slr
			 */
			if ((EventCnt_UnCorEcc[SlrCnt] == 1U) && \
				(EventFromSlr == SlrCnt)) {
				xil_printf("Success: UnCorrectable Error"\
					" event received\n\r");
				/* Clear Event count to check next event */
				--EventCnt_UnCorEcc[SlrCnt];
			} else {
				xil_printf("Failure: UnCorrectable Error "\
					"event not received\n\r");
				++FailCnt;
			}

			/* Check Cram status */
			if(CfrStatusInfo.Status == CFR_STATUS_UNCOR_ECC)
			{
				xil_printf("Success: Cfr is set to expected"\
					" status. Status = 0x%08x\n\r", \
					CfrStatusInfo.Status);
			} else {
				xil_printf("Failure: Cfr set to unexpected"\
					" status. Status = 0x%08x\n\r", \
					CfrStatusInfo.Status);
				++FailCnt;
			}

			/* Check Cram Cor Err count */
			if(IncreaseInCorErrCnt == 0U)
			{
				xil_printf("Success: correctable Error count"\
					" remained same as expected\n\r");
			} else {
				xil_printf("Failure: correctable Error count."
					" IncreaseInCorErrCnt = 0x%08x\n\r", \
					IncreaseInCorErrCnt);
				++FailCnt;
			}
			break;
		case 2:
			/**
			 * Check if the event is received and it is from
			 * current Slr
			 */
			if ((EventCnt_Crc[SlrCnt] == 1U) && \
				(EventFromSlr == SlrCnt)) {
				xil_printf("Success: CRC Error event "\
					"received\n\r");
				/* Clear Event count to check next event */
				--EventCnt_Crc[SlrCnt];
			} else {
				xil_printf("Failure: CRC Error event not "
					"received\n\r");
				++FailCnt;
			}

			/* Check Cram status */
			if(CfrStatusInfo.Status == CFR_STATUS_CRC)
			{
				xil_printf("Success: Cfr is set to expected"\
					" status. Status = 0x%08x\n\r", \
					CfrStatusInfo.Status);
			} else {
				xil_printf("Failure: Cfr set to unexpected "\
					"status. Status = 0x%08x\n\r", \
					CfrStatusInfo.Status);
				++FailCnt;
			}

			/* Check Cram Cor Err count */
			if(IncreaseInCorErrCnt == 0U)
			{
				xil_printf("Success: correctable Error count"\
					" remained same as expected\n\r");
			} else {
				xil_printf("Failure: correctable Error count."
					" IncreaseInCorErrCnt = 0x%08x\n\r", \
					IncreaseInCorErrCnt);
				++FailCnt;
			}
			break;
		default:
			xil_printf("Failure: unknown TestID\n\r");
			break;
		}
		xil_printf("------------------------------------------"\
			"-----\n\r");
	}

END:
	xil_printf("----------------------------------------------------\n\r");
	xil_printf("-----------------Print Report-----------------------\n\r");
	xil_printf("----------------------------------------------------\n\r");
	if (FailCnt == 0U){
		xil_printf("CRAM Tests Ran Successfully\n\r");
	} else {
		xil_printf("CRAM Tests FailCnt = %d\n", FailCnt);
	}
	return 0;
}
