/**************************************************************************************************
* Copyright (C) 2024 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xplmi_client.c
 *
 * This file contains the implementation of the client interface functions.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  dd   01/09/24 Initial release
 *       pre  07/10/24 Added support for configure secure communication command and also added
 *                     SSIT support
 *       pre  09/30/24 Added XPlmi_GetSecureCommStatus API
 *       pre  10/19/24 Fixed compilation warning
 *       pre  01/13/25 Added command to set access status of DDRMC main registers
 *       obs  03/17/25 Fixed GCC warnings
 * 2.00  sk   01/23/26 Updated event logging for all sub commands
 * 2.4   gnr  03/18/26 Updated the Payload assignments with XPLMI_PACK_PAYLOAD macros
 *       tbk  02/24/26 Added SMC support for client applications
 *       tbk  05/19/26 Unified response indexing; helper now handles SMC/mailbox shift
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xilplmi_client_apis XilPlmi Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xplmi_client.h"
#include "xplmi_generic.h"

/************************************ Constant Definitions ***************************************/
#define XPLMI_ADDR_HIGH_SHIFT              (32U) /**< Shift value to get higher 32 bit address */

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to provides Get device id
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DeviceIdCode used to store the Id code register values.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
  ************************************************************************************************/
int XPlmi_GetDeviceID(XPlmi_ClientInstance *InstancePtr, XLoader_DeviceIdCode *DeviceIdCode)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Validate input parameters */
	if (DeviceIdCode == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD0(Payload, (u32)XPLMI_GET_DEVICE_CMD_ID);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
			(u32)RESPONSE_ARG_CNT);
	if (Status == XST_SUCCESS) {
		DeviceIdCode->IdCode = Response[0U];
		DeviceIdCode->ExtIdCode = Response[1U];
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to Get board
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	Addr 			Address where PLM has to copy the board details
 * @param	Size	 		Max size available at destination for PLM to copy in words
 * @param	ResponseLength	Length of board data that PLM copied
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_GetBoard(XPlmi_ClientInstance *InstancePtr, u64 Addr, u32 Size, u32 *ResponseLength)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Validate input parameters */
	if (ResponseLength == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD3(Payload, (u32)XPLMI_GET_BOARD_CMD_ID, (u32)(Addr >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Addr), Size);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
			(u32)RESPONSE_ARG_CNT);
	if (Status == XST_SUCCESS) {
		*ResponseLength = Response[0U];
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to Tamper Trigger
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	TamperResponse	To select the valid tamper response
 *
 * @return
 *			 - Handoff to the Rom on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_TamperTrigger (XPlmi_ClientInstance *InstancePtr, u32 TamperResponse)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD1(Payload, (u32)XPLMI_TAMPER_TRIGGER_CMD_ID, TamperResponse);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to Event Logging
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	sub_cmd	To configure the debug information
 * @param	Arg Arguments to be passed for the sub command,
 * 		for all sub commands Arg holds Address, whereas
 *		uart sub command Arg holds Arg1, Arg2 in upper and
 *		lower 32bit respectively
 * @param	Len Length of the buffer
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_EventLogging(XPlmi_ClientInstance *InstancePtr, u32 sub_cmd, u64 Arg, u32 Len)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT] = {0U};
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Performs input parameters validation. Return error code if input parameters are invalid */
	if (InstancePtr == NULL) {
		goto END;
	}

	/** Fill IPI Payload based on sub command */
	switch (sub_cmd) {
		case XPLMI_LOGGING_CMD_CONFIG_LOG_LEVEL:
			XPLMI_PACK_PAYLOAD2(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, Arg);
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_LOG_MEM:
			XPLMI_PACK_PAYLOAD4(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, (u32)(Arg >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Arg), Len);
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_DATA:
			XPLMI_PACK_PAYLOAD3(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, (u32)(Arg >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Arg));
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO:
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_TRACE_MEM:
			XPLMI_PACK_PAYLOAD4(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, (u32)(Arg >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Arg), Len);
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_DATA:
			XPLMI_PACK_PAYLOAD3(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, (u32)(Arg >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Arg));
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO:
			Status = XST_SUCCESS;
			break;
		case XPLMI_LOGGING_CMD_CONFIG_UART:
			XPLMI_PACK_PAYLOAD3(Payload, (InstancePtr->SlrIndex << XPLMI_SLR_INDEX_SHIFT) |
			(u32)XPLMI_EVENT_LOGGING_CMD_ID, sub_cmd, (u32)(Arg >> XPLMI_ADDR_HIGH_SHIFT), (u32)(Arg));
			Status = XST_SUCCESS;
			break;
		default:
			xil_printf("Received invalid event logging command: 0x%x\n\r", sub_cmd);
			Status = XST_FAILURE;
			break;
	}

	if (Status != XST_SUCCESS) {
		goto END;
	}

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
			(u32)RESPONSE_ARG_CNT);
	if ((Status == XST_SUCCESS) && ((sub_cmd == XPLMI_LOGGING_CMD_RETRIEVE_LOG_BUFFER_INFO) ||
			(sub_cmd == XPLMI_LOGGING_CMD_RETRIEVE_TRACE_BUFFER_INFO))) {
		xil_printf("Received Data: 0x%x 0x%x 0x%x 0x%x 0x%x,\n\r",
			Response[0U], Response[1U], Response[2U], Response[3U], Response[4U]);
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to configure secure communication
 *
 * @param	InstancePtr             Pointer to the client instance
 * @param	SsitSecCommDataPtr      Pointer to structure which contains SLR index, IV1, IV2 and key
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_ConfigSecureComm(XPlmi_ClientInstance *InstancePtr, XPlmi_SsitSecComm *SsitSecCommDataPtr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Validate input parameters */
	if (SsitSecCommDataPtr == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD3(Payload, (u32)XPLMI_CONFIG_SECCOMM_CMD_ID, SsitSecCommDataPtr->SlrIndex,
			(u32)((u64)(UINTPTR)&SsitSecCommDataPtr->IVsandKey >> XPLMI_ADDR_HIGH_SHIFT),
			(u32)((UINTPTR)&SsitSecCommDataPtr->IVsandKey));

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to get secure communication status
 *
 * @param	InstancePtr  Pointer to the client instance
 * @param   SlrIndex     SLR number for which secure communication establishment status is needed
 * @param   SecCommStatus Pointer to variable to which secure communication status is to be written
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_GetSecureCommStatus(XPlmi_ClientInstance *InstancePtr, u32 SlrIndex, u32 *SecCommStatus)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 Response[RESPONSE_ARG_CNT] = {0U};

	/** Validate input parameters */
	if (SecCommStatus == NULL) {
		goto END;
	}

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD1(Payload, (u32)XPLMI_GETSECCOMM_STATUS_CMD_ID, SlrIndex);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, Response,
			(u32)RESPONSE_ARG_CNT);
	if (Status == XST_SUCCESS) {
		*SecCommStatus = Response[0U];
	}

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to set DDRMC main registers status command
 *
 * @param	InstancePtr  Pointer to the client instance
 * @param   DDRMCNum     DDRMC number
 * @param   RegSts       To be set status
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_SetDDRMCMainRegSts(XPlmi_ClientInstance *InstancePtr, u32 DDRMCNum, u32 RegSts)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Fill IPI Payload */
	XPLMI_PACK_PAYLOAD2(Payload, (u32)XPLMI_DDRMC_MAINREG_STS_SET_CMD_ID, DDRMCNum, RegSts);

	/** Send request to PLM using generic API */
	Status = XPlmi_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

	return Status;
}

/*************************************************************************************************/
/**
 * @brief	Adds the SLR Index.
 *
 * @param  InstancePtr is a pointer to instance XPlmi_ClientInstance
 * @param  SlrIndex - SLR index number
 *
 * @return	- XST_SUCCESS - On valid input SlrIndex.
 *		    - XST_FAILURE - On non valid input SlrIndex
 *
 *************************************************************************************************/
int XPlmi_InputSlrIndex(XPlmi_ClientInstance *InstancePtr, u32 SlrIndex)
{
	int Status = XST_FAILURE;
	if (SlrIndex <= XPLMI_SLR_INDEX_3) {
		InstancePtr->SlrIndex = SlrIndex;
	    Status = XST_SUCCESS;
	}
	return Status;
}

/** @} End of xilplmi_client_apis group */
