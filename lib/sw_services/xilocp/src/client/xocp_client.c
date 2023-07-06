/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
* 1.2   kpt  06/02/23 Updated XOcp_GetHwPcrLog
*       kal  06/02/23 Added client side API for SW PCR
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
 * @param   PcrNum - Variable of XOcp_HwPcr enum to select the PCR to
 *          be extended
 * @param   ExtHashAddr - Address of the buffer which holds the hash to be
 *          extended
 *
 * @return
 *          - XST_SUCCESS - If PCR extend is success
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ExtendHwPcr(XOcp_ClientInstance *InstancePtr, XOcp_HwPcr PcrNum,
	u64 ExtHashAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_EXTEND_HWPCR);
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
 * @param   PcrMask - Mask that tells which PCR to be read based on the
 * 		bits set (bit 0 corresponds to PCR 0)and how many PCRs
 * 		to be read using the number of bits set.
 * @param   PcrBufAddr - Address of the buffer to store the
 *          	requested PCR contents
 * @param   PcrBufSize - Buffer size to read the PCR values
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetHwPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u64 PcrBufAddr,
	u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GET_HWPCR);
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
 * @brief   This function sends IPI request to get the log and status of HWPCR
 *
 * @param   InstancePtr      - Pointer to the client instance
 * @param   HwPcrEventAddr   - Pointer to the XOcp_HwPcrEvent structure
 * @param   HwPcrLogInfoAddr - Pointer to the XOcp_HwPcrLogInfo structure
 * @param   NumOfLogEntries  - Number of log entries to read
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetHwPcrLog(XOcp_ClientInstance *InstancePtr, u64 HwPcrEventAddr, u64 HwPcrLogInfoAddr,
		u32 NumOfLogEntries)
{
	volatile int Status = XST_FAILURE;
        u32 Payload[XOCP_PAYLOAD_LEN_6U];

        if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
                goto END;
        }

        /** Fill IPI Payload */
        Payload[0U] = OcpHeader(0U, XOCP_API_GET_HWPCRLOG);
        Payload[1U] = (u32)HwPcrEventAddr;
        Payload[2U] = (u32)(HwPcrEventAddr >> 32);
	Payload[3U] = (u32)HwPcrLogInfoAddr;
        Payload[4U] = (u32)(HwPcrLogInfoAddr >> 32);
        Payload[5U] = NumOfLogEntries;

        Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
                sizeof(Payload)/sizeof(u32));

END:
        return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to extend the SW PCR with
 *          provided hash/data.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   ExtendParams - Pointer to the XOcp_SwPcrExtendParams structure
 *
 * @return
 *          - XST_SUCCESS - If PCR extend is success
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ExtendSwPcr(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrExtendParams *ExtendParams)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_3U];
	u64 ExtendParamsAddr = (u64)(UINTPTR)ExtendParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_EXTEND_SWPCR);
	Payload[1U] = (u32)ExtendParamsAddr;
	Payload[2U] = (u32)(ExtendParamsAddr >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
		sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get the SW PCR value from
 *          requested PCR.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   PcrMask - Mask that tells which PCR to be read based on the
 * 		bits set (bit 0 corresponds to PCR 0)and how many PCRs
 * 		to be read using the number of bits set.
 * @param   PcrBuf - Pointer to the PCR buffer to store the
 *          	requested PCR contents
 * @param   PcrBufSize - Buffer size to read the PCR values
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetSwPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u8 *PcrBuf,
	u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_5U];
	u64 PcrBufAddr = (u64)(UINTPTR)PcrBuf;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GET_SWPCR);
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
 * @brief   This function sends IPI request to get the SW PCR log.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   LogParams - Pointer to the XOcp_SwPcrLogReadData structure.
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetSwPcrLog(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrLogReadData *LogParams)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_3U];
	u64 LogBufAddr = (u64)(UINTPTR)LogParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GET_SWPCRLOG);
	Payload[1U] = (u32)LogBufAddr;
	Payload[2U] = (u32)(LogBufAddr >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
				sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to get the SW PCR data for the
 * 		specified PCR.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   DataParams - Pointer of the XOcp_SwPcrReadData structure.
 *
 * @return
 *          - XST_SUCCESS - If PCR contents are copied
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_GetSwPcrData(XOcp_ClientInstance *InstancePtr, XOcp_SwPcrReadData *DataParams)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XOCP_PAYLOAD_LEN_3U];
	u64 DataBufAddr = (u64)(UINTPTR)DataParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GET_SWPCRDATA);
	Payload[1U] = (u32)DataBufAddr;
	Payload[2U] = (u32)(DataBufAddr >> 32);

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
	Payload[0U] = OcpHeader(0U, XOCP_API_GETX509CERT);
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
	Payload[0U] = OcpHeader(0U, XOCP_API_ATTESTWITHDEVAK);
	Payload[1U] = (u32)AttestWithDevAk;
	Payload[2U] = (u32)(AttestWithDevAk >> 32);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,
			sizeof(Payload)/sizeof(u32));

END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @brief	This function sends IPI request to generate shared secret using
 * 		Elliptic Curve Diffie–Hellman Key Exchange (ECDH). The private
 * 		key used to generate the shared secret is internal DevAK
 * 		private key which is determined by the subsystem from where the
 * 		command is originating.
 *
 * @param	InstancePtr		Pointer to the client instance
 * @param	PubKey			Pointer to the buffer which contains the
 * 					public key to be used for calculating
 * 					shared secret using ECDH.
 * @param	SharedSecret		Pointer to the output buffer which shall
 * 					be used to store shared secret
 *
 * @return
 *		 - XST_SUCCESS  On Success
 *		 - Errorcode  On failure
 *
 ******************************************************************************/
int XSecure_GenSharedSecretwithDevAk(XOcp_ClientInstance *InstancePtr, const u8* PubKey, u8 *SharedSecret)
{
	int Status = XST_FAILURE;
	u32 Payload[XMAILBOX_PAYLOAD_LEN_5U];
	u64 PubKeyAddr = (u64)(UINTPTR)PubKey;
	u64 SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/* Fill IPI Payload */
	Payload[0U] = OcpHeader(0U, XOCP_API_GEN_SHARED_SECRET);
	Payload[1U] = (u32)PubKeyAddr;
	Payload[2U] = (u32)(PubKeyAddr >> XOCP_ADDR_HIGH_SHIFT);
	Payload[3U] = (u32)SharedSecretAddr;
	Payload[4U] = (u32)(SharedSecretAddr >> XOCP_ADDR_HIGH_SHIFT);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, sizeof(Payload)/sizeof(u32));

END:
	return Status;
}
