/**************************************************************************************************
* Copyright (c) 2024 - 2026, Advanced Micro Devices, Inc. All Rights Reserved.
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
 * 1.1   am   05/18/25 Fixed implicit conversion of operands
 *       rmv  07/16/25 Added "XAsufw_ReadIpiMsgFromPlm" function and PLM event handling
 *       vm   10/05/26 Bounded the wait for a PLM interrupt acknowledgement and kept the
 *                     keep-alive counter advancing while it waits
 *
 * </pre>
 *
 *************************************************************************************************/
/**
* @addtogroup xasufw_application ASUFW Server Functionality
* @{
*/
/*************************************** Include Files *******************************************/
#include "xasufw_ipi.h"
#include "xasufw_queuescheduler.h"
#include "xasufw_debug.h"
#include "xasufw_status.h"
#include "xasufw_util.h"
#include "xasufw_plmeventschedular.h"
#include "xasufw_hw.h"

/************************************ Constant Definitions ***************************************/
#define XASUFW_IPI_ACK_MAX_POLLS		(100000000U)
						/**< Upper bound on the polls of the observation
						 * register while PLM is given time to acknowledge
						 * an interrupt, about 135 seconds on a board, which
						 * is the tolerance of a slow PLM the driver poll it
						 * replaces allowed */

#define XASUFW_IPI_ACK_OBS_VALUE		(0U)
						/**< Value the PMC bit of the observation register
						 * takes once PLM has acknowledged. It is the
						 * condition XIpiPsu_PollForAck() in the ipipsu
						 * driver waits for */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/
#if defined(XPAR_XIPIPSU_0_BASEADDR)
static s32 XAsufw_WaitForPlmAck(void);
#endif

/************************************ Variable Definitions ***************************************/
#if defined(XPAR_XIPIPSU_0_BASEADDR)
static XIpiPsu IpiInst; /**< Instance of IPI Driver */

/*************************************************************************************************/
/**
 * @brief	This function waits for PLM to acknowledge an interrupt while keeping the ASUFW
 *		keep-alive counter advancing.
 *
 * @return
 *		- XASUFW_SUCCESS, if PLM acknowledged the interrupt within the maximum wait.
 *		- XASUFW_FAILURE, if PLM did not acknowledge the interrupt within the maximum
 *		wait.
 *
 * @note	The observation register holds the destination bit set until PLM acknowledges, so
 *		the acknowledgement is awaited as a register event. The driver poll,
 *		XIpiPsu_PollForAck() in the ipipsu driver, waits for the same condition on the
 *		same register, and is not used only because it cannot advance the keep-alive
 *		counter. The register offset is taken from the driver header, so moving the
 *		register breaks this build, but the awaited value is not, and
 *		XASUFW_IPI_ACK_OBS_VALUE has to be revisited if the driver ever waits for a
 *		different one.
 *		Sending a message and reading a response are waited on in task context, where
 *		the counter can advance. Reading a PLM event is waited on in the IPI interrupt
 *		handler, where interrupts are disabled and the millisecond tick cannot move, so
 *		that caller takes the bound below but keeps the pre-existing behaviour of
 *		leaving the counter still for the length of the wait.
 *
 *************************************************************************************************/
static s32 XAsufw_WaitForPlmAck(void)
{
	return XAsufw_WaitForEventKeepAlive((IpiInst.Config.BaseAddress + XIPIPSU_OBS_OFFSET),
					    IPI_ASU_ISR_PMC_MASK, XASUFW_IPI_ACK_OBS_VALUE,
					    XASUFW_IPI_ACK_MAX_POLLS);
}

/*************************************************************************************************/
/**
 * @brief	This function initializes IPI driver, enables interrupts and initializes
 * 		shared memory.
 *
 * @return
 *		- XASUFW_SUCCESS, if initialization of IPI and shared memory is successful.
 *
 *************************************************************************************************/
s32 XAsufw_IpiInit(void)
{
	s32 Status = XASUFW_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/** Load Config for ASU IPI. */
	IpiCfgPtr = XIpiPsu_LookupConfig(XASUFW_IPI_DEVICE_ID);
	if (IpiCfgPtr == NULL) {
		goto END;
	}

	/** Initialize the IPI driver. */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr, IpiCfgPtr->BaseAddress);
	if (XASUFW_SUCCESS != Status) {
		goto END;
	}
	/** Enable IPI interrupt from PMC. */
	XIpiPsu_InterruptEnable(&IpiInst, IPI_ASU_ISR_PMC_MASK);

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
	XIpiPsu_InterruptEnable(&IpiInst, (u32)IpiBitMask);
}

/*************************************************************************************************/
/**
 * @brief	This function is a handler for IPI interrupts. It triggers the corresponding IPI task
 * when the interrupt is received and clears the IPI interrupt at ASU.
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
		IpiMask = IpiIsr & ((u32)0x1U << Count);
		if (IpiMask != 0U) {
			/**
			 * If IPI is received from PLM, trigger the PLM notification handler else
			 * handles the client request.
			 */
			if (IpiMask == IPI_ASU_ISR_PMC_MASK) {
				XAsufw_HandlePlmEvent();
			} else {
				XAsufw_TriggerQueueTask(IpiMask);
			}
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
	Status = XAsufw_WaitForPlmAck();
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_POLL_FOR_ACK_FAILED, Status);
		goto END;
	}

	/** Write IPI message to ASU-PMC message buffer. */
	Status = XIpiPsu_WriteMessage(&IpiInst, IPI_ASU_ISR_PMC_MASK, MsgBufPtr, MsgBufLen,
				      XIPIPSU_BUF_TYPE_MSG);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_WRITE_MESSAGE_FAILED, Status);
		goto END;
	}

	/** Trigger an IPI interrupt to PLM. */
	Status = XIpiPsu_TriggerIpi(&IpiInst, IPI_ASU_ISR_PMC_MASK);
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
	Status = XAsufw_WaitForPlmAck();
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_POLL_FOR_ACK_FAILED, Status);
		goto END;
	}

	/** Read IPI response from PLM. */
	Status = XIpiPsu_ReadMessage(&IpiInst, IPI_ASU_ISR_PMC_MASK, RespBufPtr, RespBufLen,
				     XIPIPSU_BUF_TYPE_RESP);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_READ_MESSAGE_FAILED, Status);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function reads IPI message from PLM.
 *
 * @param	MsgBufPtr	Pointer to the message buffer.
 * @param	MsgBufLen	Message buffer length.
 *
 * @return
 *	- XASUFW_SUCCESS, if IPI read message from PLM is successful.
 *	- XASUFW_FAILURE, in case of failure.
 *	- XASUFW_IPI_INVALID_INPUT_PARAMETERS, if input arguments for IPI receive is invalid.
 *	- XASUFW_IPI_POLL_FOR_ACK_FAILED, if IPI polling for acknowledgment fails.
 *	- XASUFW_IPI_READ_MESSAGE_FAILED, if IPI read message fails.
 *
 *************************************************************************************************/
s32 XAsufw_ReadIpiMsgFromPlm(u32 *MsgBufPtr, u32 MsgBufLen)
{
	s32 Status = XASUFW_FAILURE;

	/** Validate inputs. */
	if ((NULL == MsgBufPtr) || (MsgBufLen == 0U) || (MsgBufLen > XASUFW_IPI_MAX_MSG_LEN)) {
		Status = XASUFW_IPI_INVALID_INPUT_PARAMETERS;
		goto END;
	}

	/** Check if the IPI interrupt is processed. */
	Status = XAsufw_WaitForPlmAck();
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_POLL_FOR_ACK_FAILED, Status);
		goto END;
	}

	/** Read IPI message from PLM. */
	Status = XIpiPsu_ReadMessage(&IpiInst, IPI_ASU_ISR_PMC_MASK, MsgBufPtr, MsgBufLen,
			XIPIPSU_BUF_TYPE_MSG);
	if (XASUFW_SUCCESS != Status) {
		Status = XAsufw_UpdateErrorStatus(XASUFW_IPI_READ_MESSAGE_FAILED, Status);
	}

END:
	return Status;
}

#endif
/** @} */
