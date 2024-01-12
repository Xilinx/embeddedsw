/**************************************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
 *
 * @file xloader_plat_client.c
 *
 * This file contains the implementation of the client interface functions
 *
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
 * @addtogroup xloader_client_apis XilLoader Client APIs
 * @{
 */

/*************************************** Include Files *******************************************/

#include "xloader_plat_client.h"
#include "xloader_defs.h"

/************************************ Constant Definitions ***************************************/

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

/************************************ Variable Definitions ***************************************/

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to configure jtag status.
 *
 * @param	InstancePtr Pointer to the client instance.
 * @param	Flag 		To enable / disable jtag.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ConfigureJtagState(XLoader_ClientInstance *InstancePtr, u32 Flag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XLOADER_HEADER(XLOADER_HEADER_LEN_1, XLOADER_CMD_ID_CONFIG_JTAG_STATE);
	Payload[1U] = Flag;

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ConfigureJtagState CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));

END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief	This function sends IPI request to read DDR crypto performance counters.
 *
 * @param	InstancePtr 	Pointer to the client instance.
 * @param	Id				DDR device id.
 * @param	CryptoCounters	To read DDR crypto counters.
 *
 * @return
 *			 - XST_SUCCESS on success.
 *			 - XST_FAILURE on failure.
 *
 **************************************************************************************************/
int XLoader_ReadDdrCryptoPerfCounters(XLoader_ClientInstance *InstancePtr, u32 NodeId,
		XLoader_DDRCounters *CryptoCounters)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_2U];

    /**
	 * - Performs input parameters validation. Return error code if input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	Payload[0U] = PACK_XLOADER_HEADER(XLOADER_HEADER_LEN_1,
					XLOADER_CMD_ID_READ_DDR_CRYPTO_COUNTERS);
	Payload[1U] = NodeId;

	/**
	 * - Send an IPI request to the PLM by using the XLoader_ReadDdrCryptoPerfCounters CDO command
	 * Wait for IPI response from PLM with a timeout.
	 * - If the timeout exceeds then error is returned otherwise it returns the status of the IPI
	 * response.
	 */
	Status = XLoader_ProcessMailbox(InstancePtr, Payload, sizeof(Payload) / sizeof(u32));
	CryptoCounters->DDRCounter0 =  InstancePtr->Response[1];
	CryptoCounters->DDRCounter1 =  InstancePtr->Response[2];
	CryptoCounters->DDRCounter2 =  InstancePtr->Response[3];
	CryptoCounters->DDRCounter3 =  InstancePtr->Response[4];

END:
	return Status;
}