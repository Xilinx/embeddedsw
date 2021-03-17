/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsem_client_api.c
*
* @addtogroup xsem_client_apis XilSEM Versal Client APIs
* @{
* This file has IPI command utilities required for XilSEM NPI and CRAM.
* @cond xsem_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   gm   01/04/2021   Initial creation : Cram IPI commands
* 0.2   hb   01/06/2021   Added Npi IPI commands
* 0.3   rb   01/18/2021   Added Npi PMC RAM status read API and corrected
*                         goto label
* 0.4   rb   01/25/2021   API for event registration to SEM Error Manager
* 0.5   rb   03/04/2021   Get NPI Error Information from PMC RAM
* 0.6   hv   03/11/2021   Doxygen changes
* 0.7   hb   03/15/2021   MISRA fixes and formatted code
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#include "xsem_client_api.h"

/****************************************************************************/
/**
 * @brief	CRAM Init Command
 *
 * @param[in]	IpiInst : Ipi Instance
 * @param[out]	Resp : Structure Pointer with IPI response.
 * 		Resp->RespMsg1 : Acknowledgment
 * 		Resp->RespMsg2 : Status of CRAM init
 *
 * @return	XST_FAILURE : on Init fail
 * 		XST_SUCCESS : on Init success
 *****************************************************************************/
XStatus XSem_CmdCfrInit(XIpiPsu *IpiInst, XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: Resp is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_ID_CFR_INIT);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	CRAM Start Scan Command
 *
 * @param[in]	IpiInst : Ipi Instance
 * @param[out]	Resp : Structure Pointer with IPI response.
 * 		Resp->RespMsg1 : Acknowledgment
 * 		Resp->RespMsg2 : Status of CRAM Start Scan
 *
 * @return	XST_FAILURE : On Start fail
 * 		XST_SUCCESS : On Start success
 *****************************************************************************/
XStatus XSem_CmdCfrStartScan(XIpiPsu *IpiInst, XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: Resp is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_ID_CFR_START_SCAN);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	CRAM Stop Scan Command
 *
 * @param[in]	IpiInst : Ipi Instance
 * @param[out]	Resp : Structure Pointer with IPI response.
 * 		Resp->RespMsg1 : Acknowledgment
 * 		Resp->RespMsg2 : Status of CRAM Stop Scan
 *
 * @return	XST_FAILURE : On Stop fail
 * 		XST_SUCCESS : On Stop success
 *****************************************************************************/
XStatus XSem_CmdCfrStopScan(XIpiPsu *IpiInst, XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: Resp is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_ID_CFR_STOP_SCAN);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status){
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status){
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	CRAM Error Injection Command
 *
 * @param[in]	IpiInst : Ipi Instance
 * @param[in]	ErrDetail : Structure Pointer with Error Injection details
 * @param[out]	Resp : Structure Pointer with IPI response
 * 		Resp->RespMsg1 : Acknowledgment
 * 		Resp->RespMsg1 : Status of Error Injection
 *
 * @return	XST_FAILURE : On Inject fail
 * 		XST_SUCCESS : On Inject success
 *****************************************************************************/
XStatus XSem_CmdCfrNjctErr (XIpiPsu *IpiInst, \
			XSemCfrErrInjData *ErrDetail, \
			XSemIpiResp *Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == ErrDetail) {
		XSem_Dbg("[%s] ERROR: ErrDetailStruct is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: Resp is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD5(Payload, CMD_ID_CFR_NJCT_ERR, ErrDetail->Efar,
			ErrDetail->Qword, ErrDetail->Bit, ErrDetail->Row);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status){
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status){
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with " \
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Get CRAM Status
 *
 * @param[out]	CfrStatusInfo : Structure Pointer with Cram Status details
 *
 * @return	XST_FAILURE : In case of NULL pointer reference for
 * 			      CfrStatusInfo
 * 		XST_SUCCESS : On reading the CRAM status successfully
 *****************************************************************************/
XStatus XSem_CmdCfrGetStatus(XSemCfrStatus *CfrStatusInfo)
{
	XStatus Status = XST_FAILURE;
	u32 Index = 0U;

	if (NULL == CfrStatusInfo) {
		XSem_Dbg("[%s] ERROR: CfrStatusInfo is NULL\n\r", __func__);
		goto END;
	}

	CfrStatusInfo->Status = XSem_In32(PMC_RAM_SEM_CRAM_STATUS);
	CfrStatusInfo->ErrCorCnt = XSem_In32(PMC_RAM_SEM_CRAM_COR_BITCNT);

	for (Index = 0U; Index < MAX_CRAMERR_REGISTER_CNT; Index++) {
		CfrStatusInfo->ErrAddrL[Index] = \
			XSem_In32((PMC_RAM_SEM_CRAMERR_ADDRL0 + (Index * 8U)));
		CfrStatusInfo->ErrAddrH[Index] = \
			XSem_In32((PMC_RAM_SEM_CRAMERR_ADDRH0 + (Index * 8U)));
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	NPI Start Scan Command
 *
 * @param[in]	IpiInst : IPI Instance
 * @param[out]	Resp : Structure Pointer with IPI response
 * 		Resp->RespMsg[1] : Acknowledgment
 * 		Resp->RespMsg[2] : Status of NPI Start Scan
 *
 * @return	XST_FAILURE : On start fail
 * 		XST_SUCCESS : On start success
 *****************************************************************************/
XStatus XSem_CmdNpiStartScan (XIpiPsu *IpiInst, XSemIpiResp * Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: RespStruct is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_NPI_STARTSCAN);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with"
		"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with"
		"ErrCode 0x%x\n\r", _func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	NPI Stop Scan Command
 *
 * @param[in]	IpiInst : IPI Instance
 * @param[out]	Resp : Structure Pointer with IPI response
 * 		Resp->RespMsg[1] : Acknowledgment
 * 		Resp->RespMsg[2] : Status of NPI Stop Scan
 *
 * @return	XST_FAILURE : On stop fail
 * 		XST_SUCCESS : On stop success
 *****************************************************************************/
XStatus XSem_CmdNpiStopScan (XIpiPsu *IpiInst, XSemIpiResp * Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}
	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: RespStruct is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_NPI_STOPSCAN);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with"
		"ErrCode 0x%x\n\r", _func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with"
		"ErrCode 0x%x\n\r", _func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	NPI Inject error Command
 *
 * @param[in]	IpiInst : IPI Instance
 * @param[out]	Resp : Structure Pointer with IPI response
 * 		Resp->RespMsg[1] : Acknowledgment
 * 		Resp->RespMsg[2] : Status of NPI Inject error
 *
 * @return	XST_FAILURE : On error inject failure
 * 		XST_SUCCESS : On error inject success
 *****************************************************************************/
XStatus XSem_CmdNpiInjectError (XIpiPsu *IpiInst, XSemIpiResp * Resp)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Resp) {
		XSem_Dbg("[%s] ERROR: RespStruct is NULL\n\r", __func__);
		goto END;
	}

	PACK_PAYLOAD1(Payload, CMD_NPI_ERRINJECT);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with"
		" ErrCode 0x%x\n\r", _func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with"
		" ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Resp->RespMsg1 = Response[1U];
	Resp->RespMsg2 = Response[2U];

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	Get NPI Scan Status
 *
 * @param[out]	NpiStatusInfo : Structure Pointer with NPI Status details
 *
 * @return	XST_FAILURE : In case of NULL pointer reference for
 * 			      NpiStatusInfo
 * 		XST_SUCCESS : On reading the NPI status successfully
 *****************************************************************************/
XStatus XSem_CmdNpiGetStatus(XSemNpiStatus *NpiStatusInfo)
{
	XStatus Status = XST_FAILURE;
	u32 Index = 0U;

	if (NULL == NpiStatusInfo) {
		XSem_Dbg("[%s] ERROR: NpiStatusInfo is NULL\n\r", __func__);
		goto END;
	}

	NpiStatusInfo->Status = XSem_In32(PMC_RAM_SEM_NPI_STATUS);
	NpiStatusInfo->ScanCnt = XSem_In32(PMC_RAM_SEM_NPI_SCAN_CNT);
	NpiStatusInfo->HbCnt = XSem_In32(PMC_RAM_SEM_NPI_HEARTBEAT_CNT);

	for (Index = 0U; Index < MAX_NPI_SLV_SKIP_CNT; Index++) {
		NpiStatusInfo->SlvSkipCnt[Index] = \
			XSem_In32(PMC_RAM_SEM_NPI_SLVSKIP_CNT0 + (Index * 4U));
	}

	for (Index = 0U; Index < MAX_NPI_ERR_INFO_CNT; Index++) {
		NpiStatusInfo->ErrInfo[Index] = \
			XSem_In32(PMC_RAM_SEM_NPIERR_INFO0 + (Index * 4U));
	}

	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is to SEM Event notification
 * 		registration/un-registration.
 *
 * @param[in]	IpiInst : IPI Instance
 * @param[in]	Notifier : Pointer to the notifier object to be associated
 * 		with the requested notification
 *
 * @return	XST_FAILURE : On registration/un-registration fail
 * 		XST_SUCCESS : On registration/un-registration success
 *
 * @note	The caller shall initialize the notifier object before invoking
 *		the XSem_RegisterEvent function.
 *
 *****************************************************************************/
XStatus XSem_RegisterEvent(XIpiPsu *IpiInst, XSem_Notifier* Notifier)
{
	XStatus Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	if (NULL == IpiInst) {
		XSem_Dbg("[%s] ERROR: IpiInst is NULL\n\r", __func__);
		goto END;
	}

	if (NULL == Notifier) {
		XSem_Dbg("[%s] ERROR: Notifier is NULL\n\r", __func__);
		goto END;
	}

	/* Flag will decide to enable/disable event */
	PACK_PAYLOAD4(Payload, CMD_EM_EVENT_REGISTER, Notifier->Module,
			Notifier->Event, Notifier->Flag);

	Status = XSem_IpiSendReqPlm(IpiInst, Payload);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiSendReqPlm failed with "\
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	Status = XSem_IpiPlmRespMsg(IpiInst, Response);
	if (XST_SUCCESS != Status) {
		XSem_Dbg("[%s] ERROR: XSem_IpiPlmRespMsg failed with "\
				"ErrCode 0x%x\n\r", __func__, Status);
		goto END;
	}

	XSem_Dbg("[%s] SUCCESS: 0x%x\n\r", __func__, Status);

END:
	return Status;
}
/** @} */
