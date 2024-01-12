/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
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
 *
 * </pre>
 *
 *************************************************************************************************/

/**
 * @addtogroup xplmi_client_apis XilPlmi Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xplmi_client.h"

/************************************ Constant Definitions ***************************************/

#define XPLMI_ADDR_HIGH_SHIFT (32U) /**< Shift value to get higher 32 bit address */

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
	u32 Payload[XMAILBOX_PAYLOAD_LEN_1U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XPLMI_HEADER(XPLMI_HEADER_LEN_0, (u32)XPLMI_GET_DEVICE_CMD_ID);

    /**
	 * - Send an IPI request to the PLM by using the XPlmi_GetDeviceID CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XPlmi_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));
	DeviceIdCode->IdCode = InstancePtr->Response[1];
	DeviceIdCode->ExtIdCode = InstancePtr->Response[2];

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
	u32 Payload[XMAILBOX_PAYLOAD_LEN_4U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XPLMI_HEADER(XPLMI_HEADER_LEN_3, (u32)XPLMI_GET_BOARD_CMD_ID);
    Payload[1U] = (u32)(Addr >> XPLMI_ADDR_HIGH_SHIFT);
	Payload[2U] = (u32)(Addr);
    Payload[3U] = Size;

    /**
	 * - Send an IPI request to the PLM by using the XPlmi_GetBoard CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XPlmi_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));
	*ResponseLength = InstancePtr->Response[1];

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
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}
	Payload[0U] = PACK_XPLMI_HEADER(XPLMI_HEADER_LEN_1, (u32)XPLMI_TAMPER_TRIGGER_CMD_ID);
    Payload[1U] = TamperResponse;

	/**
	 * - Send an IPI request to the PLM by using the XPlmi_TamperTrigger CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	XPlmi_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to Event Logging
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	sub_cmd		To configure the debug information
 * @param	Addr		Address where the event has to happen
 * @param	Len			Length of the buffer
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 *************************************************************************************************/
int XPlmi_EventLogging(XPlmi_ClientInstance *InstancePtr, u32 sub_cmd, u64 Addr, u32 Len)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XPLMI_HEADER(XPLMI_HEADER_LEN_4, (u32)XPLMI_EVENT_LOGGING_CMD_ID);
    Payload[1U] = sub_cmd;
    Payload[2U] = (u32)Addr >> XPLMI_ADDR_HIGH_SHIFT;
	Payload[3U] = (u32)(Addr);
	Payload[4U] = Len;

	/**
	 * - Send an IPI request to the PLM by using the XPlmi_EventLogging CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XPlmi_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));
END:
	return Status;
}