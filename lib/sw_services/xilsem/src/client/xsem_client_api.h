/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xsem_client_api.h
*
* This file has Definitions of commonly used macros and data types needed for
* XilSEM IPI commands interface.
* @cond xsem_internal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date         Changes
* ----  ---  ----------   --------------------------------------------------
* 0.1   gm   01/04/2021   Initial creation : Cram IPI commands
* 0.2   hb   01/06/2021   Added Npi IPI commands
* 0.3   rb   01/18/2021   Added Npi PMC RAM status read API
*       rb   01/25/2021   Updated IPI API command Index
* 0.4   rb   01/25/2021   Support of event registration API
* 0.5   rb   03/04/2021   Updated PMC RAM registers
* 0.6   gm   03/10/2021   Updated cram error event notification
* 0.7   hv   03/11/2021   Doxygen changes
* 0.8   hb   03/15/2021   MISRA fixes, added Event Notifiers macros and
*                         formatted code
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/
#ifndef XSEM_CLIENT_API_H
#define XSEM_CLIENT_API_H
/***************************** Include Files *********************************/
#include "xsem_ipi_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Cfr Commands Acknowledgment ID */
#define CMD_ACK_CFR_INIT		(0x00010301U)
#define CMD_ACK_CFR_START_SCAN		(0x00010302U)
#define CMD_ACK_CFR_STOP_SCAN		(0x00010303U)
#define CMD_ACK_CFR_NJCT_ERR		(0x00010304U)

/* NPI Commands Acknowledgment ID */
#define CMD_ACK_NPI_STARTSCAN		(0x00010305U)
#define CMD_ACK_NPI_STOPSCAN		(0x00010306U)
#define CMD_ACK_NPI_ERRINJECT		(0x00010307U)

#define MAX_CRAMERR_REGISTER_CNT	(7U)
#define MAX_NPI_SLV_SKIP_CNT		(8U)
#define MAX_NPI_ERR_INFO_CNT		(2U)

/* PMC_RAM - Base Address */
#define PMC_RAM_BASEADDR		(0XF2014000U)

/* SEM NPI Status */
#define PMC_RAM_SEM_NPI_STATUS		(PMC_RAM_BASEADDR + 0x00000050U)

/* SEM NPI Slave Skip Count */
#define PMC_RAM_SEM_NPI_SLVSKIP_CNT0	(PMC_RAM_BASEADDR + 0x00000054U)

/* SEM NPI Scan Count */
#define PMC_RAM_SEM_NPI_SCAN_CNT	(PMC_RAM_BASEADDR + 0x00000074U)

/* SEM NPI Heartbeat Count */
#define PMC_RAM_SEM_NPI_HEARTBEAT_CNT	(PMC_RAM_BASEADDR + 0x00000078U)

/* SEM NPI Error Info 0 */
#define PMC_RAM_SEM_NPIERR_INFO0	(PMC_RAM_BASEADDR + 0x0000007CU)

/* SEM NPI Error Info 1 */
#define PMC_RAM_SEM_NPIERR_INFO1	(PMC_RAM_BASEADDR + 0x00000080U)

/* PMC_RAM_SEM_CRAM_STATUS */
#define PMC_RAM_SEM_CRAM_STATUS		(PMC_RAM_BASEADDR + 0X00000084U)

/* PMC_RAM_SEM_CRAMERR_ADDRL0 */
#define PMC_RAM_SEM_CRAMERR_ADDRL0	(PMC_RAM_BASEADDR + 0X00000088U)

/* PMC_RAM_SEM_CRAMERR_ADDRH0 */
#define PMC_RAM_SEM_CRAMERR_ADDRH0	(PMC_RAM_BASEADDR + 0X0000008CU)

/* PMC_RAM_SEM_CRAM_COR_BITCNT */
#define PMC_RAM_SEM_CRAM_COR_BITCNT	(PMC_RAM_BASEADDR + 0X000000C0U)

#define XSem_In32			Xil_In32

/* Cfr Commands ID */
#define CMD_ID_CFR_INIT			(0x01U)
#define CMD_ID_CFR_START_SCAN		(0x02U)
#define CMD_ID_CFR_STOP_SCAN		(0x03U)
#define CMD_ID_CFR_NJCT_ERR		(0x04U)

/* NPI Commands ID */
#define CMD_NPI_STARTSCAN		(0x05U)
#define CMD_NPI_STOPSCAN		(0x06U)
#define CMD_NPI_ERRINJECT		(0x07U)

/* Event Notification Register Command ID */
#define CMD_EM_EVENT_REGISTER		(0x08U)

/**
 * XSemIpiResp - IPI Response Data structure
 */
typedef struct {
	u32 RespMsg1; /**< Response-1 (Ack Header)*/
	u32 RespMsg2; /**< Response-2 */
	u32 RespMsg3; /**< Response-3 */
	u32 RespMsg4; /**< Response-4 */
	u32 RespMsg5; /**< Response-5 */
	u32 RespMsg6; /**< Response-6 */
	u32 RespMsg7; /**< Response-7 (Reserved for CRC)*/
} XSemIpiResp;


/**
 * XSemCfrErrInjData - Cfr Error Injection Data structure
 */
typedef struct {
	u32 Efar;  /**< Frame Addr */
	u32 Qword; /**< QWord 0...24 */
	u32 Bit;   /**< Bit 0...127 */
	u32 Row;   /**< Row no. */
} XSemCfrErrInjData;

/**
 * XSemCfrStatus - Cfr Status Data structure
 */
typedef struct {
	u32 Status; /**< Cfr Status */
	u32 ErrAddrL[MAX_CRAMERR_REGISTER_CNT]; /**< Error register L0...L6 */
	u32 ErrAddrH[MAX_CRAMERR_REGISTER_CNT]; /**< Error register H0...H6 */
	u32 ErrCorCnt; /**< COR ECC count. */
} XSemCfrStatus;

typedef struct {
	u32 Status; /**< Npi Status */
	u32 SlvSkipCnt[MAX_NPI_SLV_SKIP_CNT]; /**< Npi Slave Skip Count */
	u32 ScanCnt; /**< Npi Scan Count */
	u32 HbCnt; /**< Npi Heart Beat Count */
	u32 ErrInfo[MAX_NPI_ERR_INFO_CNT]; /**< NPI Error Information when SHA
						mismatch occur */
} XSemNpiStatus;

/* SEM Module Notification ID */
#define XSEM_NOTIFY_CRAM	(0U)
#define XSEM_NOTIFY_NPI		(1U)

/* SEM Generic Event ID */
#define XSEM_EVENT_ERROR	(1U)

/* CRAM Errors Event Type */
#define XSEM_EVENT_CRAM_UNCOR_ECC_ERR	(0x1U)
#define XSEM_EVENT_CRAM_CRC_ERR		(0x2U)
/* Internal or Fatal errors */
#define XSEM_EVENT_CRAM_INT_ERR		(0x3U)
/* Cor Ecc detected when correction is disabled */
#define XSEM_EVENT_CRAM_COR_ECC_ERR	(0x4U)

/* NPI Errors Event Type */
#define XSEM_EVENT_NPI_CRC_ERR		(0x1U)
/* Internal or Fatal errors */
#define XSEM_EVENT_NPI_INT_ERR		(0x2U)

/* SEM Event Flags */
#define SEM_EVENT_ENABLE	(1U)
#define SEM_EVENT_DISABLE	(0U)

/**
 * XSem_Notifier - Notifier structure register
 */
typedef struct {
	u32 Module; /**< The module to receive notifications*/
	u32 Event; /**< Event type */
	u32 Flag; /**< Event Flag */
} XSem_Notifier;

XStatus XSem_CmdCfrInit(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrStartScan(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrStopScan(XIpiPsu *IpiInst, XSemIpiResp *Resp);
XStatus XSem_CmdCfrNjctErr(XIpiPsu *IpiInst, \
		XSemCfrErrInjData *ErrDetail, XSemIpiResp *Resp);
XStatus XSem_CmdCfrGetStatus(XSemCfrStatus *CfrStatusInfo);

/* NPI functions */
XStatus XSem_CmdNpiStartScan (XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiStopScan (XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiInjectError (XIpiPsu *IpiInst, XSemIpiResp * Resp);
XStatus XSem_CmdNpiGetStatus(XSemNpiStatus *NpiStatusInfo);

/* Event Notification Management */
XStatus XSem_RegisterEvent(XIpiPsu *IpiInst, XSem_Notifier* Notifier);

#ifdef __cplusplus
}
#endif

#endif /* XSEM_CLIENT_API_H */
