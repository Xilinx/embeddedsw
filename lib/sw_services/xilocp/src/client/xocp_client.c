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
*       am   01/10/23 Added client side API for dme
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
	u64 ExtHashAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if ((PcrNum < XOCP_PCR_2) || (PcrNum > XOCP_PCR_7)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_EXTENDPCR);
	Payload[1U] = PcrNum;
	Payload[2U] = (u32)ExtHashAddr;
	Payload[3U] = (u32)(ExtHashAddr >> 32);
	Payload[4U] = Size;

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
int XOcp_GetPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u64 PcrBufAddr,
	u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GETPCR);
	Payload[1U] = PcrMask;
	Payload[2U] = (u32)PcrBufAddr;
	Payload[3U] = (u32)(PcrBufAddr >> 32);
	Payload[4U] = PcrBufSize;

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
		sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get the PCR log.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   LogAddr - Log buffer address to store the log
 * @param   NumOfLogEntries - Number of log entries to read
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetHwPcrLog(XOcp_ClientInstance *InstancePtr, u64 LogAddr, u32 NumOfLogEntries)
{
	volatile int Status = XST_FAILURE;
        u32 Payload[XOCP_PAYLOAD_LEN_4U];

        if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
                goto END;
        }

        /** Fill IPI Payload */
        Payload[0U] = OcpHeader(0U, XOCP_API_GETPCRLOG);
        Payload[1U] = (u32)LogAddr;
        Payload[2U] = (u32)(LogAddr >> 32);
        Payload[3U] = NumOfLogEntries;

        Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
                sizeof(Payload)/sizeof(u32));

END:
        return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to fill the DME structure and
 *          generates the response with signature
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   NonceAddr - pointer to 48 bytes buffer which holds the Nonce,
 *          which shall be used to fill one of the member of DME sturcture
 * @param   DmeStructResAddr - pointer to 224 bytes buffer, which is used to
 *          store the response DME structure of type XOcp_DmeResponseStructure
 *
 * @return
 *          - XST_SUCCESS - Upon success
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GenDmeResp(XOcp_ClientInstance *InstancePtr, u64 NonceAddr,
	u64 DmeStructResAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GENDMERESP);
	Payload[1U] = (u32)NonceAddr;
	Payload[2U] = (u32)(NonceAddr >> 32);
	Payload[3U] = (u32)DmeStructResAddr;
	Payload[4U] = (u32)(DmeStructResAddr >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
		sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get 509 certificate.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   GetX509CertAddr - Address of XOcp_X509Cert structure.
  *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetX509Cert(XOcp_ClientInstance *InstancePtr, u64 GetX509CertAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GetX509Cert);
	Payload[1U] = (u32)GetX509CertAddr;
	Payload[2U] = (u32)(GetX509CertAddr >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
					sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get 509 certificate.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   AttestWithDevAk - Address of XOcp_AttestWithDevAk structure.
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ClientAttestWithDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttestWithDevAk)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_3U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_AttestWithDevAk);
	Payload[1U] = (u32)AttestWithDevAk;
	Payload[2U] = (u32)(AttestWithDevAk >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
			sizeof(Payload)/sizeof(u32));

END:
	return Status;
}