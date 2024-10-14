/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_ssit.h
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
* 1.06  skg  10/04/2022 Added logic to handle invalid commands
*       ng   11/11/2022 Fixed doxygen file name error
*       is   12/19/2022 Added support for XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX
*       bm   01/03/2023 Handle SSIT Events from PPU1 IRQ directly
*       dd   03/28/2023 Updated doxygen comments
*       pre  07/11/2024 Implemented secure PLM to PLM communication
*       pre  07/30/2024 Fixed misrac violations
*       kpt  08/26/2024 Changed XPLMI_SLV_EVENT_TIMEOUT timeout to two seconds
*       pre  09/18/2024 Removed XPLMI_SLR_INDEX_SHIFT, SLR index macros
*       pre  09/24/2024 Added key zeroization and saving new key in PPU RAM
*       pre  09/30/24 Added support for get secure communication status command
*       pre  10/07/2024 Added slave error notification at slave and processing at master
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
#include "xil_error_node.h"

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

/* Slave SLR0 Event Buffer Address in Master SLR */
#define XPLMI_SLAVE_SLR0_EVENT_BUFFER_ADDR			0xF2015A00U
/* Event Response Buffer Address in Slave SLRs */
#define XPLMI_SLR_EVENT_RESP_BUFFER_ADDR			0xF2015A00U
/* Space between SLR event buffers */
#define XPLMI_SLR_REQ_AND_RESP_MAX_SIZE_IN_WORDS	0x80U
/* Slave error notification address */
#define XPLMI_SLAVE_ERROR_ADDRESS	0xF20159F0U
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
/* SSIT Maximum Slave SLRs */
#define XPLMI_SSIT_MAX_SLAVE_SLRS			0x3U
/* SSIT Maximum events per each array index*/
#define XPLMI_SSIT_MAX_BITS					32U
/* SSIT Maximum message length */
#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
#define XPLMI_KEY_SIZE_BYTES       (32U) /**< Key size in bytes */
#define XPLMI_IV_SIZE_WORDS        (4U) /**< IV size in words */
#define XPLMI_IV_SIZE_BYTES        (XPLMI_IV_SIZE_WORDS * XPLMI_WORD_LEN) /**< IV size in bytes */
#define HEADER_OFFSET              (0U) /**< Offset of header in command */
#define PAYLOAD_OFFSET             (1U) /**< Offset of payload in command */
#define LEN_BYTES_SHIFT            (16U) /**< Shift value to extract length bytes */
#define XPLMI_IV2_OFFSET_INCMD     (6U) /**< Offset of IV2 in command */
#define SECCOMM_SLAVE_INDEX        (1U) /**< Index of slave SLR */
#define XPLMI_SSIT_MAX_MSG_LEN	    0x20U /**< Maximum length of SSIT msg */
#else
#define XPLMI_SSIT_MAX_MSG_LEN		0x8U
#endif
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

#define XPLMI_MODULE_AND_APIDMASK  (XPLMI_CMD_MODULE_ID_MASK | XPLMI_CMD_API_ID_MASK)
                                   /**< Mask to extract module and api ID together */

/**
 * Event ID for SSIT sync event which is the first event
 */
enum SsitEventIndex {
	XPLMI_SLRS_SYNC_EVENT_INDEX,
	XPLMI_SLRS_MESSAGE_EVENT_INDEX,
	XPLMI_SEM_NOTIFY_ERR_EVENT_INDEX,
	XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX,
	XPLMI_SLAVE_ERR_NOTIFY_EVENT_INDEX,
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

#define XPLMI_SLV_EVENT_TIMEOUT        (0x1E8480U) /**< Two seconds timeout for message events */
/**
 * @}
 * @endcond
 */

/**
 * SSIT Single EAM Event Macros
 *
 * Enable SSIT Single EAM Event forwarding from Slave SLRs to Master SLR
 * for this specific error interrupt
 *  - XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_ID is the Error NodeId
 *  - XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK is the Error Event Mask
 *  - XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_TRIG is the trigger register address
 *
 * On Master SLR, this interrupt will trigger the same error in the local EAM
 */
#define XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_ID	(XIL_NODETYPE_EVENT_ERROR_PMC_ERR1)
#define XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK	(XIL_EVENT_ERROR_MASK_AIE_CR)
#define XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_TRIG	(PMC_GLOBAL_PMC_ERR1_TRIG)

/**************************** Type Definitions *******************************/
#define XPLMI_GET_MSGBUFF_ADDR(SlrIndex)	(XPLMI_SLAVE_SLR0_EVENT_BUFFER_ADDR + \
					(((u32)SlrIndex - 1U) * \
					XPLMI_SLR_REQ_AND_RESP_MAX_SIZE_IN_WORDS * XPLMI_WORD_LEN))

/*
 * SSIT events related structure definitions
 */
typedef struct {
	u32 EventOrigin; /**< Event origin */
	XPlmi_EventHandler_t EventHandler; /**< Event handler */
}XPlmi_SsitEvents_t;

typedef struct {
	u8 SlrIndex; /**< Slr index */
	u8 IsIntrEnabled; /**< Interruput enable status check */
	u32 SlavesMask; /**< Slaves mask */
	XPlmi_SsitEvents_t Events[XPLMI_SSIT_MAX_EVENTS]; /**< Array of SSIT maximum events */
	XPlmi_TaskNode *Task1; /**< Task1 pointer to the TaskNode structure */
	XPlmi_TaskNode *Task2; /**< Task2 pointer to the TaskNode structure */
	XPlmi_TaskNode *Task3; /**< Task3 pointer to the TaskNode structure */
}XPlmi_SsitEventStruct_t;

/*
 * SSIT event vector table structure definition
 */
typedef struct {
	u32 Events32[XPLMI_SSIT_MAX_EVENT32_INDEX]; /**< Array of SSIT maximum events 32 */
}XPlmi_SsitEventVectorTable_t;

#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
typedef enum
{
	NOTESTABLISHED = 0,
	ESTABLISHED,
} XPlmi_SecCommEstFlag;
#endif

typedef struct
{
	int (*SendMessage)(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);  /**< This function sends the message/response in the Buf with or without encryption */
	int (*ReceiveMessage)(u32* Buf, u32 BufSize, u32 SlrIndex, u32 IsCfgSecCommCmd);  /**< This function receives the message/response with or without decryption */
	int (*AesKeyWrite)(u32 SlrIndex, u32 KeyAddr);  /**< Writes the key provided into respective SLR's AES key registers */
	int (*KeyIvUpdate)(XPlmi_Cmd *Cmd); /**< This function updates the key and IV with new key and new IV */
	u32 NewKeyAddr; /**< Address of new key used for secure communication */
}XPlmi_SsitCommParams;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
/* Functions related to SSIT events between SLRs */
int XPlmi_SsitEventsInit(XPlmi_SsitCommParams *XPlm_SsitCommParams);
u8 XPlmi_SsitIsIntrEnabled(void);
void XPlmi_SsitSetIsIntrEnabled(u8 Value);
void XPlmi_SsitErrHandler(void *Data);
int XPlmi_SlaveSlrErrEventHandler(void *Data);
u8 XPlmi_SsitIsEventPending(u8 SlrIndex, u32 EventIndex);
void XPlmi_SsitSlaveErrorNotify(u32 ErrStatus);
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

u8 XPlmi_GetSlrIndex(void);
u32 XPlmi_GetSlavesSlrMask(void);
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
int XPlmi_SendIpiCmdToSlaveSlr(u32 * Payload, u32 * RespBuf);
int XPlmi_SsitSingleEamEventHandler(void *Data);
int XPlmi_SsitCfgSecComm(XPlmi_Cmd *Cmd);
int XPlmi_GetSsitSecCommStatus(XPlmi_Cmd *Cmd);

/* SSIT Sync Related functions */
int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd);
int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd);
int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd);

#ifdef PLM_ENABLE_SECURE_PLM_TO_PLM_COMM
XPlmi_SecCommEstFlag XPlmi_SsitGetSecCommEstFlag(u32 SlrIndex);
void XPlmi_SsitSetCommEstFlag(u32 SlrIndex);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_SSIT_H */

/** @} */
