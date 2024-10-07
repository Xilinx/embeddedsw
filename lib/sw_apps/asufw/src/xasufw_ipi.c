/**************************************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xasufw_ipi.c
 *
 * This file contains the IPI code for ASUFW.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.0   ma   01/02/24 Initial release
 *       ma   03/16/24 Added error codes at required places
 *       ma   07/08/24 Add task based approach at queue level
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_ipi.h"
#include "xasufw_queuescheduler.h"
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_util.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_IPI_MAX_MSG_LEN          8U /**< Maximum IPI buffer length */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/
#if defined(XPAR_XIPIPSU_0_BASEADDR)
static XIpiPsu IpiInst; /**< Instance of IPI Driver */

/*************************************************************************************************/
/**
 * @brief	This function initializes IPI driver, enables interrupts and initializes
 * 		shared memory.
 *
 * @return
 *		- XASUFW_SUCCESS, if initialization of IPI and shared memory is successful.
 *		- XASUFW_IPI_LOOKUP_CONFIG_FAILED, if IPI lookup config failed fails.
 *
 *************************************************************************************************/
s32 XAsufw_IpiInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/** Load Config for ASU IPI. */
	IpiCfgPtr = XIpiPsu_LookupConfig(XASUFW_IPI_DEVICE_ID);
	if (IpiCfgPtr == NULL) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_LOOKUP_CONFIG_FAILED, Status);
		goto END;
	}

	/** Initialize the IPI driver. */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr, IpiCfgPtr->BaseAddress);

	/** Enable IPI interrupt from PMC. */
	XIpiPsu_InterruptEnable(&IpiInst, XASUFW_IPI_PMC_MASK);

	XAsufw_Printf(DEBUG_DETAILED, "IPI interrupts are enabled\r\n");

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function enables the IPI interrupt for the given IPI bit mask.
 *
 * @param	IpiBitMask  Bit mask of the IPI channel
 *
 *************************************************************************************************/
void XAsufw_EnableIpiInterrupt(u16 IpiBitMask)
{
	XIpiPsu_InterruptEnable(&IpiInst, IpiBitMask);
}

/*************************************************************************************************/
/**
 * @brief	This function a handler for IPI interrupts. This function simply disables the
 * 		interrupts when an IPI interrupt is received. Commands execution will be done as
 * 		part of task dispatch loop.
 *
 * @param	Data    Private data (Interrupt number in this case).
 *
 *************************************************************************************************/
void XAsufw_IpiHandler(const void *Data)
{
	u32 IpiIsr = XAsufw_ReadReg(IPI_ASU_ISR);
	u32 Count = 0U;
	u32 IpiMask;

	(void)Data;
	XAsufw_Printf(DEBUG_DETAILED, "Received IPI interrupt: 0x%x\r\n", IpiIsr);

	/** Trigger Queue tasks of the IPI channels on which the new request is received. */
	while (IpiIsr != 0U) {
		IpiMask = IpiIsr & (0x1U << Count);
		if (IpiMask != 0U) {
			XAsufw_TriggerQueueTask(IpiMask);
			XAsufw_WriteReg(IPI_ASU_ISR, IpiMask);
		}
		IpiIsr = IpiIsr & (~IpiMask);
		++Count;
	}
}

/*************************************************************************************************/
/**
 * @brief	This function writes the given message to the ASU-PMC IPI buffer and triggers an
 * 		IPI interrupt to PLM.
 *
 * @param	MsgBufPtr  IPI message to be written to ASU-PMC message buffer.
 * @param	MsgBufLen  IPI message length.
 *
 * @return
 *		- XASUFW_SUCCESS, if IPI write to PLM is successful.
 *		- XASUFW_IPI_POLL_FOR_ACK_FAILED, if IPI Poll for acknowledgement fails.
 *		- XASUFW_IPI_WRITE_MESSAGE_FAILED, if IPI write message fails.
 *		- XASUFW_IPI_TRIGGER_FAILED, if IPI trigger fails.
 *
 *************************************************************************************************/
s32 XAsufw_SendIpiToPlm(const u32 *MsgBufPtr, u32 MsgBufLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate the inputs. */
	if ((NULL == MsgBufPtr) || (MsgBufLen == 0U) || (MsgBufLen > XASUFW_IPI_MAX_MSG_LEN)) {
		Status = XASUFW_IPI_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	/** Check if there is any pending IPI message. */
	/* TODO: Need to change timeout value */
	Status = XIpiPsu_PollForAck(&IpiInst, XASUFW_IPI_PMC_MASK, 0x1FFFFFFFU);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_POLL_FOR_ACK_FAILED, Status);
		goto END;
	}

	/** Write IPI message to ASU-PMC message buffer. */
	Status = XIpiPsu_WriteMessage(&IpiInst, XASUFW_IPI_PMC_MASK, MsgBufPtr, MsgBufLen,
				      XIPIPSU_BUF_TYPE_MSG);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_WRITE_MESSAGE_FAILED, Status);
		goto END;
	}

	/** Trigger an IPI interrupt to PLM. */
	Status = XIpiPsu_TriggerIpi(&IpiInst, XASUFW_IPI_PMC_MASK);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_TRIGGER_FAILED, Status);
		goto END;
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads the IPI response from PLM.
 *
 * @param	RespBufPtr Pointer tot he response buffer where the response to be copied.
 * @param	RespBufLen Length of the response to be copied.
 *
 * @return
 *		- XASUFW_SUCCESS, if IPI read response from PLM is successful.
 *		- XASUFW_IPI_INVALID_INPUT_PARAMETERS, if input arguments for IPI send/receive is
 * 			invalid.
 *		- XASUFW_IPI_POLL_FOR_ACK_FAILED, if IPI Poll for ack fails.
 *		- XASUFW_IPI_READ_MESSAGE_FAILED, if IPI read message fails.
 *
 *************************************************************************************************/
s32 XAsufw_ReadIpiRespFromPlm(u32 *RespBufPtr, u32 RespBufLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate inputs. */
	if ((NULL == RespBufPtr) || (RespBufLen == 0U) || (RespBufLen > XASUFW_IPI_MAX_MSG_LEN)) {
		Status = XASUFW_IPI_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	/** Check if the IPI interrupt is processed. */
	/* TODO: Need to change timeout value */
	Status = XIpiPsu_PollForAck(&IpiInst, XASUFW_IPI_PMC_MASK, 0x1FFFFFFFU);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_POLL_FOR_ACK_FAILED, Status);
		goto END;
	}

	/** Read IPI response from PLM. */
	Status = XIpiPsu_ReadMessage(&IpiInst, XASUFW_IPI_PMC_MASK, RespBufPtr, RespBufLen,
				     XIPIPSU_BUF_TYPE_RESP);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_READ_MESSAGE_FAILED, Status);
	}

END:
	return Status;
}
#endif
/** @} */
