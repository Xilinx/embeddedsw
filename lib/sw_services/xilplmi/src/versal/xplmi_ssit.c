/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file versal/xplmi_ssit.c
*
* This file contains the SSIT related code and is applicable only for versal
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
* 		td   10/19/2020 MISRA C Fixes
* 1.03  ma   12/17/2021 Do not check for SSIT errors during synchronization
* 1.04  tnt  01/10/2022 Update ssit_sync* code to wait for each slave mask
*       bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
* 1.05  ma   05/10/2022 Added PLM to PLM communication feature
*       bsv  06/03/2022 Add CommandInfo to a separate section in elf
*       hb   06/15/2022 Removed static declaration of XPlmi_SsitGetSlrAddr
*       is   07/10/2022 Added support for XPlmi_SsitSendMsgEventAndGetResp API
*       bm   07/24/2022 Set PlmLiveStatus during boot time
*       ma   08/10/2022 Added dummy PLM to PLM communication APIs to be used
*                       by other components when the feature is not enabled
*                       Also, check if SSIT interrupts are enabled before
*                       triggering any event
*       ma   09/02/2022 Clear SSIT Errors in PMC_ERR2_STATUS register in
*                       Slave SLRs after completing synchronization
*       ma   09/17/2022 Check SlavesMask before checking for sync initiation
*                       from Slave SLRs in XPlmi_SsitSyncEventHandler
* 1.06  skg  10/04/2022 Added logic to handle invalid commands
*       ng   11/11/2022 Fixed doxygen file name error
*       is   12/19/2022 Formatting, warning fixes in XPlmi_SendIpiCmdToSlaveSlr
*       is   12/19/2022 Added support for XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX
*       bm   01/03/2023 Handle SSIT Events from PPU1 IRQ directly
*       bm   01/03/2023 Notify Other SLRs about Secure Lockdown
*       bm   03/11/2023 Use XPLMI_BIT macro for getting bit position mask
*       bm   03/11/2023 Added redundancy on SSIT Event Trigger register write
*       dd   03/28/2023 Updated doxygen comments
*       ng   03/30/2023 Updated algorithm and return values in doxygen comments
* 1.07  bm   06/23/2023 Added SSIT Msg Event access permissions validation
*       sk   07/18/2023 Added NULL check in SsitCreateTask
*       sk   07/26/2023 Added SlrType check redundancy in XPlmi_SsitEventsInit
*       sk   07/26/2023 Added temporal check for Sld Notification
*                       in XPlmi_SsitErrHandler
*       rama 08/10/2023 Changed SSIT sync error prints to DEBUG_ALWAYS for
*                       debug level_0 option
*       dd   09/12/2023 MISRA-C violation Rule 10.3 fixed
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_ssit.h"
#include "xplmi_hw.h"
#include "xil_error_node.h"
#include "sleep.h"
#include "xplmi_modules.h"
#include "xplmi_wdt.h"
#include "xplmi_util.h"
#include "xplmi_proc.h"
#include "xplmi_tamper.h"

/************************** Function Prototypes ******************************/
static u32 XPlmi_SsitGetSlaveErrorMask(void);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
/************************** Constant Definitions *****************************/
/*
 * PMC RAM allocated for SSIT Events
 *   -SsitEvents structure address - 0xF2015000U
 *   -Event Vector Table address - 0xF2015800U
 *   -Event Buffers on Master SLR
 *     -Slave SLR0 Event Buffer - 0xF2015A00U
 *     -Slave SLR1 Event Buffer - 0xF2015C00U
 *     -Slave SLR2 Event Buffer - 0xF2015E00U
 *   -Response Buffer on Slave SLR - 0xF2015A00U
 */
/* SsitEvents structure address */
#define XPLMI_SSIT_EVENTS_HANDLER_STRUCT_ADDR		0xF2015000U
/* Event vector table address */
#define XPLMI_SSIT_EVENT_VECTOR_TABLE_ADDR			0xF2015800U
/* Slave SLR0 Event Buffer Address in Master SLR */
#define XPLMI_SLAVE_SLR0_EVENT_BUFFER_ADDR			0xF2015A00U
/* Event Response Buffer Address in Slave SLRs */
#define XPLMI_SLR_EVENT_RESP_BUFFER_ADDR			0xF2015A00U
/* Space between SLR event buffers */
#define XPLMI_SLR_REQ_AND_RESP_MAX_SIZE_IN_WORDS	0x80U

/* Timeout to wait for Slave SLRs ack during SLD Handshake - 1ms */
#define XPLMI_WAIT_FOR_SLAVE_SLRS_ACK_TIMEOUT	(1000U)
#define XPLMI_WAIT_FOR_ALL_SLRS_READY_TIMEOUT	(1000U)
#define XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_US		(1U)
#define XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_NS		(XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_US * 1000)

/* Ssit Event IPI Index */
#define XPLMI_SSIT_EVENT_IPI_INDEX		(0x1U)

/**************************** Type Definitions *******************************/
#define XPLMI_GET_EVENT_ARRAY_INDEX(EventIndex)		(u8)(EventIndex/XPLMI_SSIT_MAX_BITS)

#define XPLMI_GET_MSGBUFF_ADDR(SlrIndex)	(XPLMI_SLAVE_SLR0_EVENT_BUFFER_ADDR + \
					(((u32)SlrIndex - 1U) * \
					XPLMI_SLR_REQ_AND_RESP_MAX_SIZE_IN_WORDS * XPLMI_WORD_LEN))
/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
static XPlmi_TaskNode *XPlmi_SsitCreateTask(u8 SlrIndex);
static u8 XPlmi_SsitIsEventPending(u8 SlrIndex, u32 EventIndex);
static int XPlmi_SsitEventHandler(void *Data);
static int XPlmi_SsitSyncEventHandler(u32 SlavesMask, u32 TimeOut, u8 IsWait);
static int XPlmi_SsitMsgEventHandler(void *Data);
static void XPlmi_GetEventTableIndex(u8 SlrIndex, u8 *LocalEvTableIndex,
		u8 *RemoteEvTableIndex);
static u32 XPlmi_IsSldNotification(void);

/************************** Variable Definitions *****************************/
static XPlmi_SsitEventStruct_t *SsitEvents =
		(XPlmi_SsitEventStruct_t *)(UINTPTR)XPLMI_SSIT_EVENTS_HANDLER_STRUCT_ADDR;;
static XPlmi_SsitEventVectorTable_t *EventVectorTable =
		(XPlmi_SsitEventVectorTable_t *)(UINTPTR)XPLMI_SSIT_EVENT_VECTOR_TABLE_ADDR;

/****************************************************************************/
/**
* @brief    This function is used to check if the SSIT interrupts are enabled
*
* @return   Returns SSIT interrupts enabled status
*
****************************************************************************/
u8 XPlmi_SsitIsIntrEnabled(void)
{
	return SsitEvents->IsIntrEnabled;
}

/****************************************************************************/
/**
* @brief    This function is used to get the local SLR index
*
* @return   Returns Local SLR index
*
****************************************************************************/
u8 XPlmi_GetSlrIndex(void)
{
	return SsitEvents->SlrIndex;
}

/****************************************************************************/
/**
* @brief    This function is used to get the combined mask of all slave SLRs
*
* @return   Returns combined mask of all slave SLRs
*
****************************************************************************/
u32 XPlmi_GetSlavesSlrMask(void)
{
	return SsitEvents->SlavesMask;
}

/****************************************************************************/
/**
* @brief    This function is used to accumulate the mask of all slave SLRs
*
* @return
* 			- None
*
****************************************************************************/
static void XPlmi_UpdateSlavesSlrMask(u32 SlavesMask)
{
	SsitEvents->SlavesMask |= SlavesMask;
}

/****************************************************************************/
/**
* @brief    This function is used to set the SSIT interrupts enabled status
*
* @param    Value is interrupts enabled status
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_SsitSetIsIntrEnabled(u8 Value)
{
	SsitEvents->IsIntrEnabled = Value;
}

/****************************************************************************/
/**
* @brief    This function is used to create the Task for handling the SSIT
*           events between Master and Slave SLRs
*
* @param    SlrIndex is the task corresponding SLR Index
*
* @return   Returns pointer to the Task Node created
*
****************************************************************************/
static XPlmi_TaskNode *XPlmi_SsitCreateTask(u8 SlrIndex)
{
	XPlmi_TaskNode *Task = NULL;

	/* Check if the task is already created */
	Task = XPlmi_GetTaskInstance(XPlmi_SsitEventHandler, (void *)(u32)SlrIndex,
				XPLMI_INVALID_INTR_ID);
	if (Task == NULL) {
		/* Create task if it is not already created */
		Task = XPlmi_TaskCreate(XPLM_TASK_PRIORITY_0, XPlmi_SsitEventHandler, (void *)(u32)SlrIndex);
		if (Task == NULL) {
			goto END;
		}
		Task->IntrId = XPLMI_INVALID_INTR_ID;
	}
END:
	return Task;
}

/****************************************************************************/
/**
* @brief	This function is used to initialize the event structure and create
* 			tasks for the events between Master and Slave SLRs
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLM_ERR_TASK_CREATE if failed to create a task.
*
****************************************************************************/
int XPlmi_SsitEventsInit(void)
{
	volatile int Status = XST_FAILURE;
	u32 Idx;
	volatile u32 SlrType;
	volatile u32 SlrTypeTmp;

	/** - Initialize all the SsitEvents structure members */
	for (Idx = 0U; Idx < XPLMI_SSIT_MAX_EVENTS; Idx++) {
		SsitEvents->Events[Idx].EventOrigin = 0x0U;
		SsitEvents->Events[Idx].EventHandler = NULL;
	}

	/**
	 * - Accumulated mask of all Slave SLRs
	 *    - will be non-zero on Master SLR
	 *    - 0x0 on Slave SLRs
	 */
	SsitEvents->SlavesMask = 0U;
	SsitEvents->IsIntrEnabled = (u8)FALSE;
	/** - Read SLR Type */
	SlrType = (u32)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);
	SlrTypeTmp = (u32)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	/** - SSIT events are not supported on Monolithic devices */
	if ((SlrType == XPLMI_SSIT_MONOLITIC) || (SlrTypeTmp == XPLMI_SSIT_MONOLITIC)) {
		Status = XST_SUCCESS;
		goto END;
	}

	/** - Set the SLR index based on the SLR Type */
	switch (SlrType) {
		case XPLMI_SSIT_SLAVE0_SLR_TOP:
		case XPLMI_SSIT_SLAVE0_SLR_NTOP:
			SsitEvents->SlrIndex = XPLMI_SSIT_SLAVE0_SLR_INDEX;
			break;
		case XPLMI_SSIT_SLAVE1_SLR_TOP:
		case XPLMI_SSIT_SLAVE1_SLR_NTOP:
			SsitEvents->SlrIndex = XPLMI_SSIT_SLAVE1_SLR_INDEX;
			break;
		case XPLMI_SSIT_SLAVE2_SLR_TOP:
			SsitEvents->SlrIndex = XPLMI_SSIT_SLAVE2_SLR_INDEX;
			break;
		case XPLMI_SSIT_MASTER_SLR:
			SsitEvents->SlrIndex = XPLMI_SSIT_MASTER_SLR_INDEX;
			break;
		default:
			SsitEvents->SlrIndex = XPLMI_SSIT_INVALID_SLR_INDEX;
			break;
	}

	/** - Return error if SLR type is invalid */
	if (SsitEvents->SlrIndex == XPLMI_SSIT_INVALID_SLR_INDEX) {
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		/**
		 * - If SLR Type is Master, then create 3 tasks for handling events
		 * from each Slave SLR
		 */
		SsitEvents->Task1 = XPlmi_SsitCreateTask(XPLMI_SSIT_SLAVE0_SLR_INDEX);
		SsitEvents->Task2 = XPlmi_SsitCreateTask(XPLMI_SSIT_SLAVE1_SLR_INDEX);
		SsitEvents->Task3 = XPlmi_SsitCreateTask(XPLMI_SSIT_SLAVE2_SLR_INDEX);

		if ((SsitEvents->Task1 == NULL) || (SsitEvents->Task2 == NULL) ||
				(SsitEvents->Task3 == NULL)) {
			Status = (int)XPLM_ERR_TASK_CREATE;
			XPlmi_Printf(DEBUG_GENERAL, "SSIT event task creation "
					"failed for Slave SLR events\r\n");
			goto END;
		}
	} else {
		/**
		 * - If SLR Type is Slave, then create a task for handling events
		 * from Master SLR
		 */
		SsitEvents->Task1 = XPlmi_SsitCreateTask(XPLMI_SSIT_MASTER_SLR_INDEX);
		SsitEvents->Task2 = NULL;
		SsitEvents->Task3 = NULL;
		if (SsitEvents->Task1 == NULL) {
			Status = (int)XPLM_ERR_TASK_CREATE;
			XPlmi_Printf(DEBUG_GENERAL, "SSIT event task creation "
					"failed for Master SLR events\r\n");
			goto END;
		}
	}

	/** - Register Sync Event */
	Status = XPlmi_SsitRegisterEvent(XPLMI_SLRS_SYNC_EVENT_INDEX,
			NULL, XPLMI_SSIT_ALL_SLAVE_SLRS_MASK);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/** - Register Message Event */
	Status = XPlmi_SsitRegisterEvent(XPLMI_SLRS_MESSAGE_EVENT_INDEX,
			XPlmi_SsitMsgEventHandler, XPLMI_SSIT_MASTER_SLR_MASK);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;
	/** - Register Single EAM Event (SEE) */
	Status = XPlmi_SsitRegisterEvent(XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX,
			XPlmi_SsitSingleEamEventHandler, XPLMI_SSIT_ALL_SLAVE_SLRS_MASK);

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to register any event between Master and
*           Slave SLRs
*
* @param    EventIndex is the Index of the event to be registered
* @param    Handler is the handler to be executed when this Event occurs
* @param    EventOrigin is the SLR origin which can trigger the event
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_SSIT_EVENT_VECTOR_TABLE_IS_FULL if vector table is full and
* 			PLM is trying to register a new event.
* 			- XPLMI_SSIT_WRONG_EVENT_ORIGIN_MASK on invalid event origin.
*
****************************************************************************/
int XPlmi_SsitRegisterEvent(u32 EventIndex, XPlmi_EventHandler_t Handler,
		u8 EventOrigin)
{
	int Status = XST_FAILURE;

	/** - Check if the EventIndex is beyond maximum supported events */
	if (EventIndex >= XPLMI_SSIT_MAX_EVENTS) {
		Status = (int)XPLMI_SSIT_EVENT_VECTOR_TABLE_IS_FULL;
		goto END;
	}

	/** - Check if the event origin is within supported SLRs mask */
	if (EventOrigin > XPLMI_SSIT_ALL_SLRS_MASK) {
		Status = (int)XPLMI_SSIT_WRONG_EVENT_ORIGIN_MASK;
		goto END;
	}

	/** - Register the event */
	SsitEvents->Events[EventIndex].EventOrigin = EventOrigin;
	SsitEvents->Events[EventIndex].EventHandler = Handler;
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to write the event buffer data and trigger
*           the Message Event
*
* @param    SlrIndex is the index of the SLR to which the event buffer need to
*			be written
* @param    ReqBuf is the buffer from where the event buffer data to be
*			written to Slave SLRs event buffer
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLMI_SSIT_EVENT_IS_PENDING if the same event triggers again.
* 			- XPLMI_SSIT_BUF_SIZE_EXCEEDS if SSIT request or response buffer
* 			size exceeds.
*
****************************************************************************/
int XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(u8 SlrIndex, u32* ReqBuf,
		u32 ReqBufSize)
{
	int Status = XST_FAILURE;
	u32 SlrAddr = 0x0U;

	/**
	 * - Writing to an event buffer can be done only if
	 *    - The SLR Type is Master SLR and,
	 *    - The SLR Index is any one of the Slave SLRs
	 */
	if ((SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) ||
		((SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) ||
				(SlrIndex > XPLMI_SSIT_MAX_SLAVE_SLRS))) {
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Return error if the Message Event is already pending */
	if (XPlmi_SsitIsEventPending(SlrIndex,
			XPLMI_SLRS_MESSAGE_EVENT_INDEX) == (u8)TRUE) {
		Status = (int)XPLMI_SSIT_EVENT_IS_PENDING;
		goto END;
	}

	/** - Maximum Req buffer data allowed is 8 words */
	if (ReqBufSize > XPLMI_SSIT_MAX_MSG_LEN) {
		Status = (int)XPLMI_SSIT_BUF_SIZE_EXCEEDS;
		goto END;
	}

	/** - Get the Message Buffer address of the corresponding Slave SLR */
	SlrAddr = XPLMI_GET_MSGBUFF_ADDR(SlrIndex);

	/** - Write the given data to event buffer */
	Status = XPlmi_MemCpy64((u64)SlrAddr, (u64)(u32)ReqBuf,
			(ReqBufSize * XPLMI_WORD_LEN));

	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Trigger the Message Event */
	Status = XPlmi_SsitTriggerEvent(SlrIndex, XPLMI_SLRS_MESSAGE_EVENT_INDEX);

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to read the event buffer of Master SLR in
*           Slave SLRs where the message event is received
*
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLMI_SSIT_BUF_SIZE_EXCEEDS if SSIT request or response buffer
* 			size exceeds.
*
****************************************************************************/
int XPlmi_SsitReadEventBuffer(u32* ReqBuf, u32 ReqBufSize)
{
	int Status = XST_FAILURE;
	u64 SlrAddr;
	u32 BufAddr;

	/**
	 * - This API is applicable to be called only in Slave SLRs.
	 * Read Event Buffer in Slave SLRs when the message event is received
	 */

	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		/* Invalid SLR Type */
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Get the message buffer address of the corresponding Slave SLR */
	BufAddr = XPLMI_GET_MSGBUFF_ADDR(SsitEvents->SlrIndex);
	SlrAddr = XPlmi_SsitGetSlrAddr(BufAddr, XPLMI_SSIT_MASTER_SLR_INDEX);

	/** - Maximum allowed message buffer data is 8 words */
	if (ReqBufSize > XPLMI_SSIT_MAX_MSG_LEN) {
		Status = (int)XPLMI_SSIT_BUF_SIZE_EXCEEDS;
		goto END;
	}

	/** - Read the Event Buffer written by Master in Slave SLRs */
	Status = XPlmi_MemCpy64((u64)(u32)ReqBuf, SlrAddr,
			(ReqBufSize * XPLMI_WORD_LEN));

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to trigger the given event to the given SLR
*
* @param    SlrIndex is the index of the SLR to which the event to be triggered
* @param    EventIndex is the Index of the event to be triggered
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLMI_SSIT_INVALID_EVENT on unregistered event.
* 			- XPLMI_SSIT_EVENT_IS_PENDING if the same event triggers again.
* 			- XPLMI_SSIT_INTR_NOT_ENABLED if SSIT interrupts are not enabled.
* 			- XPLMI_EVENT_NOT_SUPPORTED_BETWEEN_SLAVE_SLRS if a request fails to
* 			trigger an event between SLRs.
* 			- XPLMI_EVENT_NOT_SUPPORTED_FROM_SLR if the event is not supported
* 			to be triggered from the running SLR.
*
****************************************************************************/
int XPlmi_SsitTriggerEvent(u8 SlrIndex, u32 EventIndex)
{
	int Status = XST_FAILURE;
	u8 Index = XPLMI_GET_EVENT_ARRAY_INDEX(EventIndex);
	u32 BitMask;
	u8 SlrEvIndex;

	/**
	 * - Check if SSIT interrupts are enabled or not before triggering the event
	 */
	if (SsitEvents->IsIntrEnabled != (u8)TRUE) {
		Status = (int)XPLMI_SSIT_INTR_NOT_ENABLED;
		goto END;
	}

	/**
	 * - Check if the event trigger request is between Slave SLRs and
	 * return and error code as events between Slave SLRs are not supported
	 */
	if ((SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) &&
		(SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX)) {
		Status = (int)XPLMI_EVENT_NOT_SUPPORTED_BETWEEN_SLAVE_SLRS;
		goto END;
	}

	/** - Check if the event is allowed to be triggered from the current SLR */
	if ((XPLMI_BIT(SsitEvents->SlrIndex) &
			SsitEvents->Events[EventIndex].EventOrigin) == 0x0U) {
		Status = (int)XPLMI_EVENT_NOT_SUPPORTED_FROM_SLR;
		goto END;
	}

	/**
	 * - Event trigger logic varies for Master and Slave SLRs.
	 *    - To trigger an event from Master SLR,
	 *      - to Slave SLR0: Trigger SSIT_ERR[0]
	 *      - to Slave SLR1: Trigger SSIT_ERR[1]
	 *      - to Slave SLR2: Trigger SSIT_ERR[2]
	 *      - To trigger an event from Slave SLRs to Master SLR,
	 *    - Trigger SSIT_ERR[0]. This will be propagated to Master SLR as below
	 *        - From Slave SLR0 triggering SSIT_ERR[0] will trigger
	 *          SSIT_ERR0 in Master SLR in PMC_GLOBAL PMC_ERR2_STATUS
	 *        - From Slave SLR1 triggering SSIT_ERR[0] will trigger
	 *          SSIT_ERR1 in Master SLR in PMC_GLOBAL PMC_ERR2_STATUS
	 *        - From Slave SLR2 triggering SSIT_ERR[0] will trigger
	 *          SSIT_ERR2 in Master SLR in PMC_GLOBAL PMC_ERR2_STATUS
	 */
	if (SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) {
		BitMask = 0x1U;
		SlrEvIndex = 0x0U;
	} else if ((SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex > XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex <= XPLMI_SSIT_SLAVE2_SLR_INDEX)) {
		BitMask = XPLMI_BIT(SlrIndex - 1U);
		SlrEvIndex = (SlrIndex - (u8)1U);
	} else {
		/** - If SLR Type is neither Master nor Slave SLR, return an error */
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/**
	 * - If the trigger request is for event which is not registered,
	 * return error
	 */
	if (EventIndex >= XPLMI_SSIT_MAX_EVENTS) {
		Status = (int)XPLMI_SSIT_INVALID_EVENT;
		goto END;
	}

	/**
	 * - If the event is already pending between Master and Slave SLRs,
	 * do not trigger it again
	 */
	if (XPlmi_SsitIsEventPending(SlrIndex, EventIndex) == (u8)TRUE) {
		Status = (int)XPLMI_SSIT_EVENT_IS_PENDING;
		goto END;
	}

	/* Trigger an event */
	EventVectorTable[SlrEvIndex].Events32[Index] ^=
			XPLMI_BIT(EventIndex % XPLMI_SSIT_MAX_BITS);

	if (EventIndex != XPLMI_SLRS_SYNC_EVENT_INDEX) {
		/** - Trigger an SSIT interrupt for SLR */
		XSECURE_REDUNDANT_IMPL(XPlmi_Out32, PMC_GLOBAL_SSIT_ERR, BitMask);
		XSECURE_REDUNDANT_IMPL(XPlmi_Out32, PMC_GLOBAL_SSIT_ERR, 0x0U);
	}
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to get the global SLR address for the given
*           local address
*
* @param    Address is the local address for which the global SLR address need
*			to be calculated
* @param    SlrIndex is the index of the SLR for which the address need to be
*			calculated
*
* @return   Returns SSIT SLR global address for the given address
*
****************************************************************************/
u64 XPlmi_SsitGetSlrAddr(u32 Address, u8 SlrIndex)
{
	u64 SlrAddr = XPLMI_SSIT_MASTER_SLR_BASEADDR;

	/**
	 * - Formula to calculate any address in SLRs from other SLRs is:
	 *   - Dest SLR Base Address + (Local Address to Calculate - PMC Base Address)
	 */
	if (SlrIndex > XPLMI_SSIT_SLAVE2_SLR_INDEX) {
		SlrAddr = 0x0U;
		goto END;
	}

	SlrAddr = (SlrAddr + (u64)(XPLMI_SSIT_SLR_ADDR_DIFF * SlrIndex)) +
			(u64)(Address - XPLMI_PMC_BASEADDR);

END:
	return SlrAddr;
}

/****************************************************************************/
/**
* @brief    This function is used to get the event table index of both local
*           and remote SLRs
*
* @param    SlrIndex is the index of the SLR
* @param    LocalEvTableIndex is the local event table index
* @param    RemoteEvTableIndex is the remote event table index
*
* @return
* 			- none
*
****************************************************************************/
static void XPlmi_GetEventTableIndex(u8 SlrIndex, u8 *LocalEvTableIndex,
		u8 *RemoteEvTableIndex)
{
	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		/*
		 * If running SLR is Master,
		 *  Local event table index is the corresponding Slave SLR's index
		 *  Remote event table index is 0x0U
		 */
		*LocalEvTableIndex = (SlrIndex - (u8)1U);
		*RemoteEvTableIndex = 0x0U;
	} else {
		/*
		 * If running SLR is one of the Slave SLRs.
		 *  Local event table index is 0x0U
		 *  Remote event table index is the corresponding event table index
		 *   in master SLR
		 */
		*LocalEvTableIndex = 0x0U;
		*RemoteEvTableIndex = (SsitEvents->SlrIndex - (u8)1U);
	}
}

/****************************************************************************/
/**
* @brief    This function is used to check if the event is pending
*
* @param    SlrIndex is the SLR index to check if event is pending
* @param    EventIndex is the Index of the event to be checked
*
* @return
* 			- Returns XST_SUCCESS if success. Otherwise, returns an error code
*
****************************************************************************/
static u8 XPlmi_SsitIsEventPending(u8 SlrIndex, u32 EventIndex)
{
	u8 EventPending = (u8)TRUE;
	u64 SlrAddr;
	u32 SlrEventLut;
	u8 Index = XPLMI_GET_EVENT_ARRAY_INDEX(EventIndex);
	u32 EventMask = XPLMI_BIT(EventIndex % XPLMI_SSIT_MAX_BITS);

	u8 LocalEvTableIndex;
	u8 RemoteEvTableIndex;

	/* Get the event vector table address of the given SLR Mask */
	SlrAddr = XPlmi_SsitGetSlrAddr(XPLMI_SSIT_EVENT_VECTOR_TABLE_ADDR, SlrIndex);

	/* Get local and remote event table index values */
	XPlmi_GetEventTableIndex(SlrIndex, &LocalEvTableIndex,
			&RemoteEvTableIndex);

	/* Read the event table corresponding to the EventIndex */
	SlrEventLut = XPlmi_In64(SlrAddr +
			(u64)(RemoteEvTableIndex*sizeof(XPlmi_SsitEventVectorTable_t)) +
			(u64)(Index*4U));

	/* Check if the event is pending */
	if (((SlrEventLut ^ EventVectorTable[LocalEvTableIndex].Events32[Index]) &
		(EventMask)) == 0x0U) {
		EventPending = (u8)FALSE;
	}

	return EventPending;
}

/****************************************************************************/
/**
* @brief    This function is used to wait for the event to be acknowledged
*           by the given SLR after triggering
*
* @param    SlrIndex is the SLR index where the event is triggered
* @param    EventIndex is the Index of the event triggered
* @param    TimeOut is the timeout in us to wait for the acknowledgment
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
*
****************************************************************************/
int XPlmi_SsitWaitForEvent(u8 SlrIndex, u32 EventIndex, u32 TimeOut)
{
	int Status = (int)XPLMI_SSIT_EVENT_NOT_ACKNOWLEDGED;
	u32 TimeOutValue = TimeOut;

	if (((SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX)) ||
		((SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX)) ||
		(SlrIndex > XPLMI_SSIT_SLAVE2_SLR_INDEX)) {
		/** - If SLR Type is neither Master nor Slave SLR, return an error */
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Wait for the event processing to finish or until TimeOut occurs */
	while (TimeOutValue != 0x0U) {
		if (XPlmi_SsitIsEventPending(SlrIndex, EventIndex) == (u8)FALSE) {
			Status = XST_SUCCESS;
			goto END;
		}
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		--TimeOutValue;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to read the response (if applicable)
*           in Master SLR once the event is acknowledged by Slave SLR
*
* @param    SlrIndex is the SLR index from the response need to be read
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLMI_INVALID_SLR_INDEX on invalid SLR index.
* 			- XPLMI_SSIT_BUF_SIZE_EXCEEDS if SSIT request or response buffer
* 			size exceeds.
*
****************************************************************************/
int XPlmi_SsitReadResponse(u8 SlrIndex, u32* RespBuf, u32 RespBufSize)
{
	int Status = XST_FAILURE;
	u64 SlrAddr = XPlmi_SsitGetSlrAddr(XPLMI_SLR_EVENT_RESP_BUFFER_ADDR, SlrIndex);

	/** - Verify this API is called only in Master SLR, else return error code. */
	if (SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) {
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Check if the SlrAddr is valid or not, else return error code. */
	if (SlrAddr == 0x0U) {
		Status = (int)XPLMI_INVALID_SLR_INDEX;
		goto END;
	}

	/** - Verify response buffer data is 8 words or less, else return error code. */
	if (RespBufSize > XPLMI_SSIT_MAX_MSG_LEN) {
		Status = (int)XPLMI_SSIT_BUF_SIZE_EXCEEDS;
		goto END;
	}

	/* Read the response to the given RespBuf */
	Status = XPlmi_MemCpy64((u64)(u32)RespBuf, SlrAddr,
			(RespBufSize * XPLMI_WORD_LEN));

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to write the response (if applicable)
*           in Slave SLR once the event is handled and acknowledge the message
*           event
*
* @param    RespBuf is the Response Buffer data to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
* 			- XPLMI_SSIT_BUF_SIZE_EXCEEDS if SSIT request or response buffer
* 			size exceeds.
* 			- XPLMI_SSIT_EVENT_IS_NOT_PENDING if an event is not pending and
* 			the request comes to write to the response buffer.
*
****************************************************************************/
int XPlmi_SsitWriteResponseAndAckMsgEvent(u32 *RespBuf, u32 RespBufSize)
{
	int Status = XST_FAILURE;

	/** - This API can be called only in Slave SLRs */
	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Maximum allowed response buffer data is 8 words */
	if (RespBufSize > XPLMI_SSIT_MAX_MSG_LEN) {
		Status = (int)XPLMI_SSIT_BUF_SIZE_EXCEEDS;
		goto END;
	}

	/**
	 * - Response buffer need to be written only for the Message event and if
	 * the same event is pending
	 */
	if (XPlmi_SsitIsEventPending(XPLMI_SSIT_MASTER_SLR_INDEX,
			XPLMI_SLRS_MESSAGE_EVENT_INDEX) != (u8)TRUE) {
		Status = (int)XPLMI_SSIT_EVENT_IS_NOT_PENDING;
		goto END;
	}

	/** - Write the response to the response buffer in the Slave SLRs memory */
	Status = XPlmi_MemCpy64((u64)XPLMI_SLR_EVENT_RESP_BUFFER_ADDR,
			(u64)(u32)RespBuf, (RespBufSize * XPLMI_WORD_LEN));

	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPlmi_SsitAcknowledgeEvent(XPLMI_SSIT_MASTER_SLR_INDEX,
			XPLMI_SLRS_MESSAGE_EVENT_INDEX);

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is used to acknowledge the event after handling
*
* @param    SlrIndex is index of the SLR for which the event was triggered
* @param    EventIndex is the event to acknowledge
*
* @return
* 			- Returns XST_SUCCESS if success.
* 			- XPLMI_INVALID_SLR_TYPE on invalid SLR type.
*
****************************************************************************/
int XPlmi_SsitAcknowledgeEvent(u8 SlrIndex, u32 EventIndex)
{
	int Status = XST_FAILURE;
	u8 Index = XPLMI_GET_EVENT_ARRAY_INDEX(EventIndex);
	u8 SlrEvIndex;

	if ((SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) &&
		(SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX)) {
		SlrEvIndex = 0x0U;
	} else if ((SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex > XPLMI_SSIT_MASTER_SLR_INDEX) &&
			(SlrIndex <= XPLMI_SSIT_SLAVE2_SLR_INDEX)) {
		SlrEvIndex = (SlrIndex - (u8)1U);
	} else {
		/** - If SLR Type is neither Master nor Slave SLR, return an error */
		Status = (int)XPLMI_INVALID_SLR_TYPE;
		goto END;
	}

	/** - Acknowledge the event once it is processed */
	EventVectorTable[SlrEvIndex].Events32[Index] ^=
			XPLMI_BIT(EventIndex % XPLMI_SSIT_MAX_BITS);
	Status = XST_SUCCESS;

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function sends a message event with given request buffer
*           to a slave SLR, waits for event completion, reads the response
*           from the slave SLR and returns it to the caller
*
* @param    SlrIndex is the SLR index from which the response need to be read
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
* @param    WaitForEventCompletion is the maximum time to wait before the
*           requested message event is handled by a slave SLR [in microseconds]
*
* @return
* 			- Returns XST_SUCCESS if success. Otherwise, returns an error code
*
****************************************************************************/
int XPlmi_SsitSendMsgEventAndGetResp(u8 SlrIndex, u32 *ReqBuf, u32 ReqBufSize,
		u32 *RespBuf, u32 RespBufSize, u32 WaitForEventCompletion)
{
	int Status = XST_FAILURE;

	/** - Write message event buffer and trigger the event from master SLR */
	Status = XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(SlrIndex, ReqBuf, ReqBufSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Wait until message event is handled by a slave SLR */
	Status = XPlmi_SsitWaitForEvent(SlrIndex,
			XPLMI_SLRS_MESSAGE_EVENT_INDEX, WaitForEventCompletion);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** - Read response in master SLR - which is provided by a slave SLR */
	Status = XPlmi_SsitReadResponse(SlrIndex, RespBuf, RespBufSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function is the task handler for the SSIT events in both
*           Master and Slave SLRs
*
* @param    Data is the SLR Mask which was assigned to the task while creating
*
* @return
* 			- Returns XST_SUCCESS if success. Otherwise, returns an error code
*
****************************************************************************/
static int XPlmi_SsitEventHandler(void *Data)
{
	int Status = XST_FAILURE;
	u64 RemoteEventTableAddr;
	u8 LocalEvTableIndex;
	u8 RemoteEvTableIndex;
	XPlmi_SsitEventVectorTable_t RemoteEventTable;
	u8 Index = 0x0U;
	u32 Idx = 0x0U;
	u8 SlrIndex = (u8)(u32)Data;
	u32 EventMask;

	/* Get local and remote event table index */
	XPlmi_GetEventTableIndex(SlrIndex, &LocalEvTableIndex, &RemoteEvTableIndex);

	/* Get the event table address of the remote SLR */
	RemoteEventTableAddr = XPlmi_SsitGetSlrAddr(
			XPLMI_SSIT_EVENT_VECTOR_TABLE_ADDR, SlrIndex);
	RemoteEventTableAddr += (u64)(RemoteEvTableIndex *
			sizeof(XPlmi_SsitEventVectorTable_t));

	/* Read the Event Table of the remote SLR */
	Status = XPlmi_MemCpy64((u64)(u32)&RemoteEventTable, RemoteEventTableAddr,
			sizeof(XPlmi_SsitEventVectorTable_t));

	/* Loop through the event table and execute the pending event handlers */
	for ( ; Index < XPLMI_SSIT_MAX_EVENT32_INDEX; ++Index) {
		if ((RemoteEventTable.Events32[Index] ^
				EventVectorTable[LocalEvTableIndex].Events32[Index]) != 0x0U) {
			for ( ; Idx < XPLMI_SSIT_MAX_EVENTS; ++Idx) {
				/* Event Mask of the corresponding event */
				EventMask = XPLMI_BIT(Idx % XPLMI_SSIT_MAX_BITS);
				/* Check if the event is pending */
				if ((((RemoteEventTable.Events32[Index] ^
						EventVectorTable[LocalEvTableIndex].Events32[Index]) &
					(EventMask)) != 0x0U) &&
					(SsitEvents->Events[Idx].EventHandler != NULL)) {
					/* Call the handler if the event is pending */
					Status = SsitEvents->Events[Idx].EventHandler(Data);
					if (Status != XST_SUCCESS) {
						XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Error 0x%x while executing "
								"event 0x%x\r\n", Status, Idx);
					}
				}
			}
		} else {
			Idx += XPLMI_SSIT_MAX_BITS;
		}
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function is the event handler for the SSIT message events
 *          between Master and Slave SLR.
 *
 * @param	Data is the SLR Type from where the event occurred
 *
 * @return	Returns the Status of SSIT Message event
 *		- XPLMI_ERR_SSIT_MSG_EVENT_NO_ACCESS if MSG Event access is
 *		  not allowed
 *
 *****************************************************************************/
static int XPlmi_SsitMsgEventHandler(void *Data)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 MsgBuf[XPLMI_SSIT_MAX_MSG_LEN] = {0U};
	XPlmi_Cmd Cmd = {0U};

	(void)Data;

	/* Read the event buffer */
	Status = XPlmi_SsitReadEventBuffer(MsgBuf, XPLMI_SSIT_MAX_MSG_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/* Set the Cmd structure based on the data */
	Cmd.CmdId = MsgBuf[0U];

	Cmd.Len = (Cmd.CmdId >> 16U) & 255U;
	if (Cmd.Len > XPLMI_MAX_IPI_CMD_LEN) {
		Cmd.Len = MsgBuf[1U];
		Cmd.Payload = (u32 *)&MsgBuf[2U];
	} else {
		Cmd.Payload = (u32 *)&MsgBuf[1U];
	}

	/*
	 * Use Subsystem Id as 0U - to indicate this command is likely
	 * a forwarded command from master SLR
	 */
	Cmd.SubsystemId = 0U;
	Cmd.IpiReqType = (Cmd.CmdId & IPI_CMD_HDR_SECURE_BIT_MASK) >>
				IPI_CMD_HDR_SECURE_BIT_SHIFT;

	/* Check Access permission of the ssit event */
	XSECURE_REDUNDANT_CALL(Status, StatusTmp, XPlmi_ValidateIpiCmd,
			&Cmd, XPLMI_SSIT_EVENT_IPI_INDEX);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_MSG_EVENT_NO_ACCESS,
				Status | StatusTmp);
		goto END;
	}

	/* Call the received command handler */
	Status = XPlmi_CmdExecute(&Cmd);

END:
	/* Write the response to the response buffer */
	Cmd.Response[0U] = (u32)Status;
	Status = XPlmi_SsitWriteResponseAndAckMsgEvent(Cmd.Response,
			XPLMI_SSIT_MAX_MSG_LEN);

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function is the event handler for the SSIT Single EAM Event
 *          between Master and Slave SLRs.
 *
 * @param   Data is the SLR index where the event occurred
 *
 * @return  Returns the Status of SSIT Single EAM event
 *
 *****************************************************************************/
int XPlmi_SsitSingleEamEventHandler(void *Data)
{
	int Status = XST_FAILURE;
	u8 SlrIndex = (u8)(UINTPTR)Data;

	/** - This API can be called only in Master SLR */
	if (SsitEvents->SlrIndex != XPLMI_SSIT_MASTER_SLR_INDEX) {
		Status = (int)XPLMI_EVENT_NOT_SUPPORTED_FROM_SLR;
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "Acknowledging SSIT EAM event from master for Slr %d\r\n",
			SlrIndex);
	/**
	 * - Acknowledge the sync event once it is received
	 * from a Slave SLR
	 */
	Status = XPlmi_SsitAcknowledgeEvent(SlrIndex, XPLMI_SLRS_SINGLE_EAM_EVENT_INDEX);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Trigger EAM event locally in Master SLR
	 */
	XPlmi_Out32(XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_TRIG,
			XPLMI_SSIT_SINGLE_EAM_EVENT_ERR_MASK);

END:
	return Status;
}

/****************************************************************************/
/**
* @brief    This function handles SSIT Errors 0/1/2 in Master and Slave SLRs.
*
* @param    Data is the IoModule interrupt ID which is passed to the handler
*
* @return
* 			- None
*
****************************************************************************/
void XPlmi_SsitErrHandler(void *Data)
{
	XPlmi_TaskNode *Task = NULL;
	u32 Id = (u32)(UINTPTR)Data;
	volatile u32 SldNotification;
	volatile u32 SldNotificationTmp;

	XPlmi_PlmIntrClear(Id);
	if (SsitEvents->IsIntrEnabled != (u8)TRUE) {
		goto END;
	}

	/**
	 * - If the SLR Type is Master,
	 *   - Trigger Task1 which is created to handle events from
	 *     Slave SLR0 to Master SLR
	 *   - Trigger Task2 which is created to handle events from
	 *     Slave SLR1 to Master SLR
	 *   - Trigger Task3 which is created to handle events from
	 *     Slave SLR2 to Master SLR
	 * - If the SLR Type is Slave SLRs,
	 *   - Trigger Task1 which is created to handle events from
	 *     Master SLR to Slave SLR
	 */
	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		if (Id == XPLMI_IOMODULE_SSIT_ERR0) {
			Task = SsitEvents->Task1;
		} else if (Id == XPLMI_IOMODULE_SSIT_ERR1) {
			Task = SsitEvents->Task2;
		} else if (Id == XPLMI_IOMODULE_SSIT_ERR2) {
			Task = SsitEvents->Task3;
		} else {
			/* Do nothing */
		}
	} else if (SsitEvents->SlrIndex != XPLMI_SSIT_INVALID_SLR_INDEX) {
		/** - For Slave SLRs, detect SLD notification if it's a long pulse */
		XSECURE_TEMPORAL_IMPL(SldNotification, SldNotificationTmp, XPlmi_IsSldNotification);
		if ((SldNotification == (u32)TRUE) || (SldNotificationTmp == (u32)TRUE)){
			XPlmi_TriggerTamperResponse(XPLMI_RTCFG_TAMPER_RESP_SLD_1_MASK,
				XPLMI_TRIGGER_TAMPER_TASK);
			goto END;
		}
		Task = SsitEvents->Task1;
	}

	if (Task != NULL) {
		/* Trigger the task */
		XPlmi_TaskTriggerNow(Task);
	}

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function is the event handler for the SSIT sync events
 *          between Master and Slave SLR.
 *
 * @param	SlavesMask is the SLR Mask for which the sync need to be executed
 * @param   TimeOut is the Timeout in us to wait for sync
 * @param   IsWait tells if the command is ssync_sync or wait for slaves SLRs
 *
 * @return
 * 			- Returns the Status of SSIT Sync event
 * 			- XPLMI_ERR_SSIT_SLAVE_SYNC if master timedout on slave sync.
 *****************************************************************************/
static int XPlmi_SsitSyncEventHandler(u32 SlavesMask, u32 TimeOut, u8 IsWait)
{
	int Status = XST_FAILURE;
	u32 SlavesReady = 0U;
	u32 TimeOutVal = TimeOut;

	if (SsitEvents->SlrIndex == XPLMI_SSIT_MASTER_SLR_INDEX) {
		/* Check if the SSIT sync event is pending from Slave SLRs */
		while (((SlavesReady & SlavesMask) != SlavesMask) && (TimeOutVal != 0x0U)) {
			usleep(1U);
			XPlmi_SetPlmLiveStatus();
			if ((SlavesMask & SSIT_SLAVE_0_MASK) == SSIT_SLAVE_0_MASK) {
				/* Check if Slave SLR0 initiated a sync */
				if (XPlmi_SsitIsEventPending(XPLMI_SSIT_SLAVE0_SLR_INDEX,
						XPLMI_SLRS_SYNC_EVENT_INDEX) == (u8)TRUE) {
					SlavesReady |= SSIT_SLAVE_0_MASK;
				}
			}
			if ((SlavesMask & SSIT_SLAVE_1_MASK) == SSIT_SLAVE_1_MASK) {
				/* Check if Slave SLR1 initiated a sync */
				if (XPlmi_SsitIsEventPending(XPLMI_SSIT_SLAVE1_SLR_INDEX,
						XPLMI_SLRS_SYNC_EVENT_INDEX) == (u8)TRUE) {
					SlavesReady |= SSIT_SLAVE_1_MASK;
				}
			}
			if ((SlavesMask & SSIT_SLAVE_2_MASK) == SSIT_SLAVE_2_MASK) {
				/* Check if Slave SLR2 initiated a sync */
				if (XPlmi_SsitIsEventPending(XPLMI_SSIT_SLAVE2_SLR_INDEX,
						XPLMI_SLRS_SYNC_EVENT_INDEX) == (u8)TRUE) {
					SlavesReady |= SSIT_SLAVE_2_MASK;
				}
			}
			--TimeOutVal;
		}

		if (0x0U != TimeOutVal) {
			if (IsWait == (u8)FALSE) {
			XPlmi_Printf(DEBUG_INFO, "Acknowledging from master\r\n");
				/*
				 * Acknowledge the sync event once it is received
				 * from each Slave SLR
				 */
				if ((SlavesMask & SSIT_SLAVE_0_MASK) == SSIT_SLAVE_0_MASK) {
					(void)XPlmi_SsitAcknowledgeEvent(
							XPLMI_SSIT_SLAVE0_SLR_INDEX,
							XPLMI_SLRS_SYNC_EVENT_INDEX);
				}
				if ((SlavesMask & SSIT_SLAVE_1_MASK) == SSIT_SLAVE_1_MASK) {
					(void)XPlmi_SsitAcknowledgeEvent(
							XPLMI_SSIT_SLAVE1_SLR_INDEX,
							XPLMI_SLRS_SYNC_EVENT_INDEX);
				}
				if ((SlavesMask & SSIT_SLAVE_2_MASK) == SSIT_SLAVE_2_MASK) {
					(void)XPlmi_SsitAcknowledgeEvent(
							XPLMI_SSIT_SLAVE2_SLR_INDEX,
							XPLMI_SLRS_SYNC_EVENT_INDEX);
				}
			}
			XPlmi_Printf(DEBUG_INFO, "SSIT Sync/Wait Slaves successful\n\r");
			Status = XST_SUCCESS;
		} else {
			/*
			 * Return error in case the sync event is not received
			 * from Slave SLRs
			 */
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Slaves did not initiate sync. "
					"SSIT Sync/Wait Slaves event timed out in Master\r\n");
			Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		}
	} else {
		/*
		 * For Slave SLRs,
		 * Trigger the SSIT sync event
		 * Wait for the event to be acknowledged from Master SLR
		 */
		Status = XPlmi_SsitTriggerEvent(XPLMI_SSIT_MASTER_SLR_INDEX,
				XPLMI_SLRS_SYNC_EVENT_INDEX);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		XPlmi_Printf(DEBUG_INFO, "SSIT Sync initiated\n\r");
		while (XPlmi_SsitIsEventPending(XPLMI_SSIT_MASTER_SLR_INDEX,
				XPLMI_SLRS_SYNC_EVENT_INDEX) == (u8)TRUE) {
			/* Do Nothing */
		}
		XPlmi_Printf(DEBUG_INFO, "SSIT Sync Master successful\n\r");
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function acknowledges all SSIT ERR IRQ bits in iomodule
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_AckrSsitIrq(void)
{
	XPlmi_Out32(PMC_PMC_MB_IO_IRQ_ACK, PMC_PMC_MB_IO_SSIT_IRQ_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function clears latched SSIT errors in PMC_ERROR2_STATUS
 *			register
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
static void XPlmi_ClearSsitErrs(void)
{
	/* Clear latched SSIT Errors */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK);
}

/*****************************************************************************/
/**
 * @brief	This function detects SLD notification on slave SLRs
 *
 * @return	TRUE if SLD notification is detected, else FALSE
 *
 *****************************************************************************/
static u32 XPlmi_IsSldNotification(void)
{
	int Status = XST_FAILURE;
	u32 IsSld = FALSE;
	u32 IoModuleIsr;

	if (XPlmi_SldState() != XPLMI_SLD_NOT_TRIGGERED) {
		goto END;
	}
	IoModuleIsr = XPlmi_In32(PMC_PMC_MB_IO_IRQ_ISR);
	if ((IoModuleIsr & PMC_PMC_MB_IO_SSIT_IRQ_MASK) == PMC_PMC_MB_IO_SSIT_IRQ_MASK) {
		Status = XPlmi_UtilPollNs(PMC_PMC_MB_IO_IRQ_ISR, PMC_PMC_MB_IO_SSIT_IRQ_MASK,
				0x0, XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_NS, XPlmi_AckrSsitIrq);
		if (Status != XST_SUCCESS) {
			IsSld = TRUE;
		}
	}
END:
	return IsSld;
}

#else
/****************************************************************************/
/**
* @brief    This function is used to get the local SLR index
*
* @return   Returns Local SLR index
*
****************************************************************************/
u8 XPlmi_GetSlrIndex(void)
{
	return 0x0U;
}

/****************************************************************************/
/**
* @brief    This function is used to get the combined mask of all slave SLRs
*
* @return   Returns combined mask of all slave SLRs
*
****************************************************************************/
u32 XPlmi_GetSlavesSlrMask(void)
{
	return 0x0U;
}

/****************************************************************************/
/**
* @brief    This function is used to register any event between Master and
*           Slave SLRs
*
* @param    EventIndex is the Index of the event to be registered
* @param    Handler is the handler to be executed when this Event occurs
* @param    EventOrigin is the SLR origin which can trigger the event
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitRegisterEvent(u32 EventIndex, XPlmi_EventHandler_t Handler,
		u8 EventOrigin)
{
	(void)EventIndex;
	(void)Handler;
	(void)EventOrigin;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to write the event buffer data and trigger
*           the Message Event
*
* @param    SlrIndex is the index of the SLR to which the event buffer need to
*			be written
* @param    ReqBuf is the buffer from where the event buffer data to be
*			written to Slave SLRs event buffer
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitWriteEventBufferAndTriggerMsgEvent(u8 SlrIndex, u32* ReqBuf,
		u32 ReqBufSize)
{
	(void)SlrIndex;
	(void)ReqBuf;
	(void)ReqBufSize;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to read the event buffer of Master SLR in
*           Slave SLRs where the message event is received
*
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitReadEventBuffer(u32* ReqBuf, u32 ReqBufSize)
{
	(void)ReqBuf;
	(void)ReqBufSize;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to trigger the given event to the given SLR
*
* @param    SlrIndex is the index of the SLR to which the event to be triggered
* @param    EventIndex is the Index of the event to be triggered
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitTriggerEvent(u8 SlrIndex, u32 EventIndex)
{
	(void)SlrIndex;
	(void)EventIndex;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to wait for the event to be acknowledged
*           by the given SLR after triggering
*
* @param    SlrIndex is the SLR index where the event is triggered
* @param    EventIndex is the Index of the event triggered
* @param    TimeOut is the timeout in us to wait for the acknowledgment
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitWaitForEvent(u8 SlrIndex, u32 EventIndex, u32 TimeOut)
{
	(void)SlrIndex;
	(void)EventIndex;
	(void)TimeOut;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to read the response (if applicable)
*           in Master SLR once the event is acknowledged by Slave SLR
*
* @param    SlrIndex is the SLR index from the response need to be read
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitReadResponse(u8 SlrIndex, u32* RespBuf, u32 RespBufSize)
{
	(void)SlrIndex;
	(void)RespBuf;
	(void)RespBufSize;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to write the response (if applicable)
*           in Slave SLR once the event is handled and acknowledge the message
*           event
*
* @param    RespBuf is the Response Buffer data to be written
* @param    RespBufSize is the size of the response buffer
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitWriteResponseAndAckMsgEvent(u32 *RespBuf, u32 RespBufSize)
{
	(void)RespBuf;
	(void)RespBufSize;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to acknowledge the event after handling
*
* @param    SlrIndex is index of the SLR for which the event was triggered
* @param    EventIndex is the event to acknowledge
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitAcknowledgeEvent(u8 SlrIndex, u32 EventIndex)
{
	(void)SlrIndex;
	(void)EventIndex;

	return XST_SUCCESS;
}

/****************************************************************************/
/**
* @brief    This function is used to get the global SLR address for the given
*           local address
*
* @param    Address is the local address for which the global SLR address need
*			to be calculated
* @param    SlrIndex is the index of the SLR for which the address need to be
*			calculated
*
* @return
* 			- Returns 0x0 for non-SSIT designs
*
****************************************************************************/
u64 XPlmi_SsitGetSlrAddr(u32 Address, u8 SlrIndex)
{
	(void)Address;
	(void)SlrIndex;

	return 0x0U;
}

/****************************************************************************/
/**
* @brief    This function sends a message event with given request buffer
*           to a slave SLR, waits for event completion, reads the response
*           from the slave SLR and returns it to the caller
*
* @param    SlrIndex is the SLR index from which the response need to be read
* @param    ReqBuf is the buffer to which the event buffer data to be written
* @param    ReqBufSize is the size of the ReqBuf
* @param    RespBuf is the Response Buffer to where the response to be written
* @param    RespBufSize is the size of the response buffer
* @param    WaitForEventCompletion is the maximum time to wait before the
*           requested message event is handled by a slave SLR [in microseconds]
*
* @return
* 			- Returns XST_SUCCESS for non-SSIT designs
*
****************************************************************************/
int XPlmi_SsitSendMsgEventAndGetResp(u8 SlrIndex, u32 *ReqBuf, u32 ReqBufSize,
		u32 *RespBuf, u32 RespBufSize, u32 WaitForEventCompletion)
{
	(void)SlrIndex;
	(void)ReqBuf;
	(void)ReqBufSize;
	(void)RespBuf;
	(void)RespBufSize;
	(void)WaitForEventCompletion;

	return XST_SUCCESS;
}
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

/*****************************************************************************/
/**
 * @brief	This function checks which Slave SLR this code is running on and
 * 	returns corresponding SSIT mask of PMC GLOBAL PMC_ERROR2_STATUS register
 *
 * @return	return 0 if error occurred. Else return one of the ERR_MASK.
 *
 *****************************************************************************/
static u32 XPlmi_SsitGetSlaveErrorMask(void)
{
	u32 SlrErrMask = 0U;
	u8 SlrType = (u8)(XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	switch (SlrType){
	case XPLMI_SSIT_SLAVE0_SLR_TOP:
	case XPLMI_SSIT_SLAVE0_SLR_NTOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK;
		break;
	case XPLMI_SSIT_SLAVE1_SLR_TOP:
	case XPLMI_SSIT_SLAVE1_SLR_NTOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK;
		break;
	case XPLMI_SSIT_SLAVE2_SLR_TOP:
		SlrErrMask = PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK;
		break;
	default:
		break;
	}

	return SlrErrMask;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Sync Master command execution.
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return	Returns the Status of SSIT Sync Master command
 *
 *****************************************************************************/
int XPlmi_SsitSyncMaster(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 ErrStatus = (u32)XST_FAILURE;
	u32 SlrErrMask = XPlmi_SsitGetSlaveErrorMask();
	XPLMI_EXPORT_CMD(XPLMI_SSIT_SYNC_MASTER_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_ZERO, XPLMI_CMD_ARG_CNT_ZERO);

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/** - Treat SSIT sync command as event, if SSIT interrupts are enabled. */
	if (SsitEvents->IsIntrEnabled == (u8)TRUE) {
		Status = XPlmi_SsitSyncEventHandler(0x0U, 0x0U, 0x0U);
		goto END;
	}
#endif

	if (0U != SlrErrMask){
		/** - Initiate synchronization to PLM running on master SLR */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK);

		/** - Wait for Master SLR to reach synchronization point */
		ErrStatus = Xil_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		while ((ErrStatus & SlrErrMask) != SlrErrMask) {
			ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		}

		/** - Complete synchronization from slave */
		XPlmi_UtilRMW(PMC_GLOBAL_SSIT_ERR, PMC_GLOBAL_SSIT_ERR_IRQ_OUT_0_MASK, 0U);
		ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
		while ((ErrStatus & SlrErrMask) == SlrErrMask) {
			ErrStatus = XPlmi_In32((UINTPTR)PMC_GLOBAL_PMC_ERR2_STATUS);
			/** - Clear existing status to know the actual status from Master SLR */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, SlrErrMask);
		}

		XPlmi_Printf(DEBUG_INFO, "SSIT Sync Master successful\n\r");
		Status = XST_SUCCESS;
	} else {
		XPlmi_Printf(DEBUG_GENERAL, "SSIT sync master error. Unknown SLR type.\n\r");
	}

	/* Clear existing status */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS, PMC_GLOBAL_SSIT_ERR_MASK);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
END:
#endif
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Sync Slaves command execution.
 *  		Command payload parameters are
 *				* Slaves Mask
 *				* Timeout
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- Returns the Status of SSIT Sync Slaves command
 * 			- XPLMI_ERR_SSIT_SLAVE_SYNC if master timedout on slave sync.
 *
 *****************************************************************************/
int XPlmi_SsitSyncSlaves(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	u32 PmcErrStatus2;
	XPLMI_EXPORT_CMD(XPLMI_SSIT_SYNC_SLAVES_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/** - Accumulate slaves mask to track the number of slave SLRs. */
	XPlmi_UpdateSlavesSlrMask(SlavesMask);

	/** - Treat SSIT sync command as event, if SSIT interrupts are enabled. */
	if (SsitEvents->IsIntrEnabled == (u8)TRUE) {
		Status = XPlmi_SsitSyncEventHandler(SlavesMask, TimeOut, (u8)FALSE);
		goto END;
	}
#endif

	/** - Wait until all Slaves initiate synchronization point. */
	while (((SlavesReady & SlavesMask) != SlavesMask) && (TimeOut != 0x0U)) {
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}
		--TimeOut;
	}

	if (0x0U != TimeOut) {
		XPlmi_Printf(DEBUG_INFO, "Acknowledging from master\r\n");
		/** - Acknowledge synchronization. */
		XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, SlavesMask);

		/* Use 100us for Acknowledge synchronization. */
		TimeOut = 100U;
		while (((SlavesReady & SlavesMask) != 0x0U) && (TimeOut != 0x0U)) {
			usleep(1U);
			XPlmi_SetPlmLiveStatus();
			PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_0_MASK);
			}
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_1_MASK);
			}
			if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) == (u32)FALSE) {
				SlavesReady &= (~SSIT_SLAVE_2_MASK);
			}

			/** - Clear existing status to know the actual status from Slave SLRs. */
			XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
				(SlavesReady << PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_SHIFT));
			--TimeOut;
		}
	}

	/** - If the timeout is exceeded, return an error code. */
	if (0x0U == TimeOut) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "Slaves did not initiate sync. "
				"SSIT Sync Slaves command timed out in Master\r\n");
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	/** - De-assert synchronization acknowledgement from master. */
	XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, 0);
	XPlmi_Printf(DEBUG_INFO, "SSIT Sync Slaves successful\n\r");
	Status = XST_SUCCESS;

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function provides SSIT Wait Slaves command execution.
 * 			 Command payload parameters are
 *				* Slaves Mask
 *				* Timeout
 *
 * @param	Cmd is pointer to the command structure
 *
 * @return
 * 			- Returns the Status of SSIT Wait Slaves command
 * 			- XPLMI_ERR_SSIT_SLAVE_SYNC if master timedout on slave sync.
 *
 *****************************************************************************/
int XPlmi_SsitWaitSlaves(XPlmi_Cmd *Cmd)
{
	int Status = XST_FAILURE;
	u32 SlavesMask = Cmd->Payload[0U];
	u32 TimeOut = Cmd->Payload[1U];
	u32 SlavesReady = 0U;
	volatile u32 PmcErrStatus2;
	XPLMI_EXPORT_CMD(XPLMI_SSIT_WAIT_SLAVES_CMD_ID, XPLMI_MODULE_GENERIC_ID,
		XPLMI_CMD_ARG_CNT_TWO, XPLMI_CMD_ARG_CNT_TWO);

	XPlmi_Printf(DEBUG_INFO, "%s %p\n\r", __func__, Cmd);

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/** - Treat SSIT sync command as event, if SSIT interrupts are enabled. */
	if (SsitEvents->IsIntrEnabled == (u8)TRUE) {
		Status = XPlmi_SsitSyncEventHandler(SlavesMask, TimeOut, (u8)TRUE);
		goto END;
	}
#endif

	/** - Clear any existing SSIT errors in PMC_ERR1_STATUS register. */
	XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS, PMC_GLOBAL_SSIT_ERR_MASK);

	/** - Wait until all Slaves initiate synchronization point. */
	while (((SlavesReady & SlavesMask) != SlavesMask) && (TimeOut != 0x0U)) {
		usleep(1U);
		XPlmi_SetPlmLiveStatus();
		PmcErrStatus2 = XPlmi_In32(PMC_GLOBAL_PMC_ERR2_STATUS);
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR0_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_0_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR1_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_1_MASK;
		}
		if ((PmcErrStatus2 & PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERR2_MASK) != (u32)FALSE) {
			SlavesReady |= SSIT_SLAVE_2_MASK;
		}

		--TimeOut;
	}

	/** - If the timeout is exceeded, return an error code. */
	if (TimeOut == 0x0U) {
		XPlmi_Printf(DEBUG_PRINT_ALWAYS,
			"Received error from Slave SLR or Timed out\r\n");
		Status = XPlmi_UpdateStatus(XPLMI_ERR_SSIT_SLAVE_SYNC, Status);
		goto END;
	}

	XPlmi_Printf(DEBUG_INFO, "SSIT Wait Master successful\n\r");
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function handles invalid commands
 *
 * @param	Payload  holds the inputs of the slaves
 * @param	RespBuf  Stores the response of the slaves
 *
 * @return
 * 			- XST_SUCCESS on success.
 * 			- XPLMI_ERR_CMD_APIID on unregistered command ID.
 * 			- XPLMI_SSIT_INTR_NOT_ENABLED if interrupts are not enabled.
 *
 *****************************************************************************/
int XPlmi_SendIpiCmdToSlaveSlr(u32 * Payload, u32 * RespBuf)
{
	int Status = XST_FAILURE;

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	u32 SlrIndex = 0U;
	u32 SlrType = (XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK);

	if (RespBuf == NULL) {
		goto END;
	}

	SlrIndex = (Payload[0U] & XPLMI_CMD_SLR_ID_MASK) >> XPLMI_SLR_INDEX_SHIFT;
	Payload[0U] =  Payload[0U] & XPLMI_SLR_ID_ZEROISE;

	if (((SlrIndex >= 1U) && (SlrIndex <= 3U)) && (SlrType != XPLMI_SSIT_MONOLITIC)) {
		Status = XPlmi_SsitSendMsgEventAndGetResp((u8)SlrIndex, Payload, XPLMI_SSIT_MAX_MSG_LEN,
				RespBuf, XPLMI_SSIT_MAX_MSG_LEN, XPLMI_SLV_EVENT_TIMEOUT);
	} else {
		Status = XPlmi_UpdateStatus(XPLMI_ERR_CMD_APIID, 0);
	}
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = (int)RespBuf[0U];

END:
#else
	(void)Payload;
	(void)RespBuf;
	Status = (int)XPLMI_SSIT_INTR_NOT_ENABLED;
#endif /* PLM_ENABLE_PLM_TO_PLM_COMM */

	return Status;
}
/*****************************************************************************/
/**
 * @brief	This function notifies other SLRs about Secure Lockdown or tamper
 *			condition in SSIT devices.
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_NotifySldSlaveSlrs(void)
{
	u32 SlrType= XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK;

	/** - Do nothing for monolithic devices. */
	if (SlrType == XPLMI_SSIT_MONOLITIC) {
		goto END;
	}

#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	/** - From Master SLR, Notify Other SLRs about Tamper. */
	if ((SlrType == XPLMI_SSIT_MASTER_SLR) &&
		(XPlmi_SsitIsIntrEnabled() == (u8)TRUE)) {
		XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, SSIT_SLAVE_0_MASK |
				SSIT_SLAVE_1_MASK | SSIT_SLAVE_2_MASK);
	}
#endif

END:
	return;
}

/*****************************************************************************/
/**
 * @brief	This function performs handshake between SLRs on SSIT ERR lines.
 *			This has to be done on SSIT devices before doing secure lockdown
 *
 * @return
 * 			- None
 *
 *****************************************************************************/
void XPlmi_InterSlrSldHandshake(void)
{
#ifdef PLM_ENABLE_PLM_TO_PLM_COMM
	u32 SlrType= XPlmi_In32(PMC_TAP_SLR_TYPE) & PMC_TAP_SLR_TYPE_VAL_MASK;

	/** - Do nothing for monolithic devices. */
	if ((SlrType == XPLMI_SSIT_MONOLITIC) ||
		(XPlmi_SsitIsIntrEnabled() != (u8)TRUE)) {
		goto END;
	}

	if (SlrType == XPLMI_SSIT_MASTER_SLR) {
		/** - If Master SLR, Clear latched SSIT Errors. */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR1_STATUS,
			PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERRX_MASK);
		/**
		 * - Wait for all SLRs to be ready.
		 */
		XPlmi_UtilPollForMask(PMC_GLOBAL_PMC_ERR1_STATUS,
				PMC_GLOBAL_PMC_ERR1_STATUS_SSIT_ERRX_MASK,
				XPLMI_WAIT_FOR_SLAVE_SLRS_ACK_TIMEOUT);
		/**
		 * - Wait for twice the minimal long pulse time used to
		 * identify SLD notification.
		 */
		usleep(2 * XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_US);
		/**
		 * - De-Assert all SSIT ERR lines to indicate Slave SLRs
		 * can start the SLD.
		 */
		XPlmi_Out32(PMC_GLOBAL_SSIT_ERR, 0x0);
	}
	else {
		/** - Otherwise, Indicate ready to start secure lockdown. */
		XPlmi_Out32(PMC_GLOBAL_SSIT_ERR,
			PMC_GLOBAL_SSIT_ERR_IRQ_OUT_2_MASK);
		/** - Clear latched SSIT Errors */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK);
		/** - Wait for primary SLR to be ready. */
		XPlmi_UtilPollNs(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK,
			XPLMI_SLD_NOTIFY_MINIMAL_LONG_PULSE_NS, NULL);
		/** - Clear latched SSIT Errors. */
		XPlmi_Out32(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK);
		/** - Wait for all SLRs to be ready. */
		XPlmi_UtilPoll(PMC_GLOBAL_PMC_ERR2_STATUS,
			PMC_GLOBAL_PMC_ERR2_STATUS_SSIT_ERRX_MASK, 0U,
			XPLMI_WAIT_FOR_ALL_SLRS_READY_TIMEOUT,
			XPlmi_ClearSsitErrs);
	}

END:
#endif
	return;
}
