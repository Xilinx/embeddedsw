/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_client.c
*
* This file contains the implementation of the client interface functions for
* OCP hardware interface API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.1   am   12/21/22 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xocp_client.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/************************** Function Definitions *****************************/

/*****************************************************************************/
/**
* @brief    This function sets the instance of mailbox
*
* @param    InstancePtr - Pointer to the client instance
* @param    MailboxPtr - Pointer to the mailbox instance
*
* @return
*           - XST_SUCCESS - On successful initialization
*           - XST_FAILURE - On failure
*
******************************************************************************/
int XOcp_ClientInit(XOcp_ClientInstance* const InstancePtr,
	XMailbox* const MailboxPtr)
{
	int Status = XST_FAILURE;

	/**
	 * Uses XMailbox instance to initiate the communication between
	 * client and server.
	 */
	if (InstancePtr != NULL) {
		InstancePtr->MailboxPtr = MailboxPtr;
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to extend the PCR with
 *          provided hash by requesting ROM service
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   PcrNum - Variable of XOcp_RomHwPcr enum to select the PCR to
 *          be extended
 * @param   ExtHashAddr - Address of the buffer which holds the hash to be
 *          extended
 *
 * @return
 *          - XST_SUCCESS - If PCR extend is success
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ExtendPcr(XOcp_ClientInstance *InstancePtr, XOcp_RomHwPcr PcrNum,
	u64 ExtHashAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_EXTENDPCR);
	Payload[1U] = (u32)ExtHashAddr;
	Payload[2U] = (u32)(ExtHashAddr >> 32);
	Payload[3U] = PcrNum;

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
		sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get the PCR value from
 *          requested PCR.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   PcrNum - Variable of XOcp_RomHwPcr enum to select the PCR to
 *          be extended
 * @param   PcrBufAddr - Address of the 48 bytes buffer to store the
 *          requested PCR contents
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetPcr(XOcp_ClientInstance *InstancePtr, XOcp_RomHwPcr PcrNum,
	u64 PcrBufAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_4U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GETPCR);
	Payload[1U] = (u32)PcrBufAddr;
	Payload[2U] = (u32)(PcrBufAddr >> 32);
	Payload[3U] = PcrNum;

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
		sizeof(Payload)/sizeof(u32));

END:
	return Status;
}