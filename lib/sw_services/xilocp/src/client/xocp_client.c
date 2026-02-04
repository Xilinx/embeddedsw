/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xocp_client.c
* @addtogroup xocp_client_apis XilOcp Client APIs
* @{
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
* 1.3   kal  12/09/23 Added a check for DataAddr if size > 48 bytes SWPCR
*       am   02/06/24 Fixed Doxygen warning
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
 * @param   Size - Size of extended hash
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_EXTEND_HWPCR command and send the request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_EXTEND_HWPCR, PcrNum, ExtHashAddr,
				(ExtHashAddr >> XOCP_ADDR_HIGH_SHIFT),
				Size);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GET_HWPCR command and send the request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GET_HWPCR, PcrMask, PcrBufAddr,
				(PcrBufAddr >> XOCP_ADDR_HIGH_SHIFT),
				PcrBufSize);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
        u32 Payload[PAYLOAD_ARG_CNT];

        if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
                goto END;
        }

        /** Fill IPI payload for XOCP_API_GET_HWPCRLOG command and send the request to Server */
	XOCP_PACK_PAYLOAD5(Payload, XOCP_API_GET_HWPCRLOG, HwPcrEventAddr,
				(HwPcrEventAddr >> XOCP_ADDR_HIGH_SHIFT), HwPcrLogInfoAddr,
				(HwPcrLogInfoAddr >> XOCP_ADDR_HIGH_SHIFT), NumOfLogEntries);

        Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 ExtendParamsAddr = (u64)(UINTPTR)ExtendParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	if (ExtendParams->DataSize > XOCP_EXTENDED_HASH_SIZE_IN_BYTES) {
		if (((u32)(ExtendParams->DataAddr >> 32U)) != 0x00U) {
			Status = (int)XOCP_PCR_ERR_DATA_IN_INVALID_MEM;
			goto END;
		}
	}
	/** Fill IPI payload for XOCP_API_EXTEND_SWPCR command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_EXTEND_SWPCR, ExtendParamsAddr,
				(ExtendParamsAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 PcrBufAddr = (u64)(UINTPTR)PcrBuf;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GET_SWPCR command and send the request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GET_SWPCR, PcrMask, PcrBufAddr,
				(PcrBufAddr >> XOCP_ADDR_HIGH_SHIFT),
				PcrBufSize);

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload,	PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 LogBufAddr = (u64)(UINTPTR)LogParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GET_SWPCRLOG command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_GET_SWPCRLOG, LogBufAddr,
				(LogBufAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 DataBufAddr = (u64)(UINTPTR)DataParams;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GET_SWPCRDATA command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_GET_SWPCRDATA, DataBufAddr,
				(DataBufAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
 *          which shall be used to fill one of the member of DME structure
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
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GENDMERESP command and send the request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GENDMERESP, NonceAddr,
				(NonceAddr >> XOCP_ADDR_HIGH_SHIFT),
				DmeStructResAddr,
				(DmeStructResAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_GETX509CERT command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_GETX509CERT, GetX509CertAddr,
				(GetX509CertAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief   This function sends IPI request to attest the data with DevAk.
 *
 * @param   InstancePtr - Pointer to the client instance
 * @param   AttestWithDevAk - Address of XOcp_AttestWithDevAk structure.
 *
 * @return
 *          - XST_SUCCESS - Attestation with DevAk is successful
 *          - XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ClientAttestWithDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttestWithDevAk)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_ATTESTWITHDEVAK command and send the request to Server */
	XOCP_PACK_PAYLOAD2(Payload, XOCP_API_ATTESTWITHDEVAK, AttestWithDevAk,
				(AttestWithDevAk >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to calculate the hash of the
 * 		Key Wrap buffer and attest with Key Wrap DevAk private key.
 *
 * @param	InstancePtr - Pointer to the client instance
 * @param	AttnPloadAddr - Address of the buffer which should be attested
 * @param	AttnPloadSize - Size of buffer in bytes
 * @param	PubKeyOffset - Offset in provided buffer where public key needs to be stored
 * @param	SignatureAddr - Address of the signature after attestation
 *
 * @return
 *		- XST_SUCCESS - Successfully hashed and attested the data with Key Wrap DevAk private key
 *		- XST_FAILURE - Upon any failure
 *
 ******************************************************************************/
int XOcp_ClientAttestWithKeyWrapDevAk(XOcp_ClientInstance *InstancePtr,
				u64 AttnPloadAddr, u32 AttnPloadSize, u32 PubKeyOffset, u64 SignatureAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		goto END;
	}

	/** Fill IPI payload for XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK command and send the request to Server */
	XOCP_PACK_PAYLOAD6(Payload, XOCP_API_ATTEST_WITH_KEYWRAP_DEVAK,
			AttnPloadAddr, (AttnPloadAddr >> XOCP_ADDR_HIGH_SHIFT),
			AttnPloadSize, PubKeyOffset,
			SignatureAddr, (SignatureAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

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
int XOcp_GenSharedSecretWithDevAk(XOcp_ClientInstance *InstancePtr, const u8* PubKey, u8 *SharedSecret)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];
	u64 PubKeyAddr = (u64)(UINTPTR)PubKey;
	u64 SharedSecretAddr = (u64)(UINTPTR)SharedSecret;

	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** Fill IPI Payload for XOCP_API_GEN_SHARED_SECRET and send request to Server */
	XOCP_PACK_PAYLOAD4(Payload, XOCP_API_GEN_SHARED_SECRET, PubKeyAddr,
				(PubKeyAddr >> XOCP_ADDR_HIGH_SHIFT),
				SharedSecretAddr,
				(SharedSecretAddr >> XOCP_ADDR_HIGH_SHIFT));

	Status = XOcp_ProcessMailbox(InstancePtr->MailboxPtr, Payload, PAYLOAD_ARG_CNT);

END:
	return Status;
}
/** @} */
