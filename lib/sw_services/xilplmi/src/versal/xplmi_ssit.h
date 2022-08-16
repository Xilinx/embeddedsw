/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xplmi_ssit.h
* @addtogroup xplmi_apis XilPlmi Versal APIs
* @{
* @cond xplmi_internal
* This file contains declarations for SSIT functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ma   08/13/2019 Initial release
*       ma   08/24/2019 Added SSIT commands
* 1.01  bsv  04/04/2020 Code clean up
* 1.02  bm   10/14/2020 Code clean up
*       ana  10/19/2020 Added doxygen comments
* 1.04  tnt  01/10/2022 Added more constants to support ssit_sync_slave
* 1.05  ma   05/10/2022 Added PLM to PLM communication feature
*       hb   06/15/2022 Added event XPLMI_SEM_NOTIFY_ERR_EVENT_INDEX
*       is   07/10/2022 Added support for XPlmi_SsitSendMsgEventAndGetResp API
*       ma   08/10/2022 Added dummy PLM to PLM communication APIs to be used
*                       by other components when the feature is not enabled
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XPLMI_SSIT_H
#define XPLMI_SSIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xplmi.h"

/************************** Constant Definitions *****************************/
/**
 * @{
 * @cond xplmi_internal
 */

/**
 * SSIT defines
 */
#define PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK	(1U)
#define PMC_GLOBAL_SSIT_ERR_IRQ_OUT_1_MASK	(2U)
#define PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK	(4U)
#define PMC_GLOBAL_SSIT_ERR_MASK		(0xE0000000U)

/**
 * SSIT SLR Masks
 */
#define SSIT_SLAVE_0_MASK			(1U)
#define SSIT_SLAVE_1_MASK			(2U)
#define SSIT_SLAVE_2_MASK			(4U)

/**
 * SSIT PLM-PLM communication related event handler definition
 */
typedef int (*XPlmi_EventHandler_t)(void *Data);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/**
 * SSIT SLR Index
 */
#define XPLMI_SSIT_MASTER_SLR_INDEX		(0U)
#define XPLMI_SSIT_SLAVE0_SLR_INDEX		(1U)
#define XPLMI_SSIT_SLAVE1_SLR_INDEX		(2U)
#define XPLMI_SSIT_SLAVE2_SLR_INDEX		(3U)
#define XPLMI_SSIT_INVALID_SLR_INDEX	(4U)

/* SSIT Maximum Slave SLRs */
#define XPLMI_SSIT_MAX_SLAVE_SLRS			0x3U
/* SSIT Maximum events per each array index*/
#define XPLMI_SSIT_MAX_BITS					32U

/**
 * SSIT SLR global base addresses
 *  - PMC Local Address - 0xF0000000U
 *  - Master SLR Global Address - 0x100000000UL
 *  - Slave SLR0 Global Address - 0x108000000UL
 *  - Slave SLR1 Global Address - 0x110000000UL
 *  - Slave SLR2 Global Address - 0x118000000UL
 */
#define XPLMI_PMC_BASEADDR					(0xF0000000U)
#define XPLMI_SSIT_MASTER_SLR_BASEADDR		(0x100000000UL)
#define XPLMI_SSIT_SLR_ADDR_DIFF			(0x8000000U)

/**
 * Event ID for SSIT sync event which is the first event
 */
enum SsitEventIndex {
	XPLMI_SLRS_SYNC_EVENT_INDEX,
	XPLMI_SLRS_MESSAGE_EVENT_INDEX,
	XPLMI_SEM_NOTIFY_ERR_EVENT_INDEX,
	XPLMI_SSIT_MAX_EVENTS
};

/* Array index for events */
#define XPLMI_SSIT_MAX_EVENT32_INDEX		((XPLMI_SSIT_MAX_EVENTS/XPLMI_SSIT_MAX_BITS) + \
		(((XPLMI_SSIT_MAX_EVENTS%XPLMI_SSIT_MAX_BITS) == 0x0U) ? 0U : 1U))

/**
 * Masks for EventOrigin
 */
#define XPLMI_SSIT_MASTER_SLR_MASK		0x1U
#define XPLMI_SSIT_SLAVE_SLR0_MASK		0x2U
#define XPLMI_SSIT_SLAVE_SLR1_MASK		0x4U
#define XPLMI_SSIT_SLAVE_SLR2_MASK		0x8U
#define XPLMI_SSIT_ALL_SLAVE_SLRS_MASK	0xEU
#define XPLMI_SSIT_ALL_SLRS_MASK		0xFU
/**
 * @}
 * @endcond
 */

/**************************** Type Definitions *******************************/
/*
 * SSIT events related structure definitions
 */
typedef struct {
	u8 EventOrigin;
	XPlmi_EventHandler_t EventHandler;
}XPlmi_SsitEvents_t;

typedef struct {
	u8 SlrIndex;
	u8 IsIntrEnabled;
	XPlmi_SsitEvents_t Events[XPLMI_SSIT_MAX_EVENTS];
	XPlmi_TaskNode *Task1;
	XPlmi_TaskNode *Task2;
	XPlmi_TaskNode *Task3;
}XPlmi_SsitEventStruct_t;

/*
 * SSIT event vector table structure definition
 */
typedef struct {
	u32 Events32[XPLMI_SSIT_MAX_EVENT32_INDEX];
}XPlmi_SsitEventVectorTable_t;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* Functions related to SSIT events between SLRs */
int XPlmi_SsitEventsInit(void);
u8 XPlmi_SsitIsIntrEnabled(void);
u8 XPlmi_GetSlrIndex(void);
void XPlmi_SsitSetIsIntrEnabled(u8 Value);
void XPlmi_SsitErrHandler(u32 ErrorNodeId, u32 RegMask);
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

int XPlmi_SsitRegisterEvent(u32 EventIndex, XPlmi_EventHandler_t Handler,
		u8 EventOrigin);
int XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(u8 SlrIndex, u32* ReqBuf,
		u32 ReqBufSize);
int XPlmi_SsitReadEventBuffer(u32* ReqBuf, u32 ReqBufSize);
int XPlmi_SsitTriggerEvent(u8 SlrIndex, u32 EventIndex);
int XPlmi_SsitWaitForEvent(u8 SlrIndex, u32 EventIndex, u32 TimeOut);
int XPlmi_SsitReadResponse(u8 SlrIndex, u32* RespBuf, u32 RespBufSize);
int XPlmi_SsitWriteResponseAndAckMsgEvent(u32 *RespBuf, u32 RespBufSize);
int XPlmi_SsitAcknowledgeEvent(u8 SlrIndex, u32 EventIndex);
u64 XPlmi_SsitGetSlrAddr(u32 Address, u8 SlrIndex);
int XPlmi_SsitSendMsgEventAndGetResp(u8 SlrIndex, u32 *ReqBuf, u32 ReqBufSize,
		u32 *RespBuf, u32 RespBufSize, u32 WaitForEventCompletion);

/* SSIT Sync Related functions */
int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd);
int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd);
int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd);
#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_SSIT_H */

/** @} */
