/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_hwpcr.c
* @addtogroup xocp_hwpcr_client_apis XilOcp HwPcr Client APIs
* @{
*
* This file contains the implementation of the client interface functions for Hardware PCR API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 1.7   rpu  02/18/26 Initial release
*       rpu  03/11/26 Validate input parameters
* </pre>
*
* @note
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/
#include "xocp_hwpcr.h"

/************************************* Constant Definitions ***************************************/

/*************************************** Type Definitions *****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/************************************** Function Prototypes ***************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
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
 **************************************************************************************************/
int XOcp_ExtendHwPcr(XOcp_ClientInstance *InstancePtr, XOcp_HwPcr PcrNum,
	u64 ExtHashAddr, u32 Size)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Validate input parameters */
	if ((InstancePtr == NULL) ||(PcrNum <= XOCP_PCR_1) || (PcrNum > XOCP_PCR_7) ||
		(ExtHashAddr == 0U) || (Size != XOCP_PCR_SIZE_BYTES)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
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

/**************************************************************************************************/
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
 **************************************************************************************************/
int XOcp_GetHwPcr(XOcp_ClientInstance *InstancePtr, u32 PcrMask, u64 PcrBufAddr,
	u32 PcrBufSize)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Validate input parameters */
	if ((InstancePtr == NULL) || (PcrMask == 0U) || (PcrBufAddr == 0U) || (PcrBufSize == 0U)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
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

/**************************************************************************************************/
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
 **************************************************************************************************/
int XOcp_GetHwPcrLog(XOcp_ClientInstance *InstancePtr, u64 HwPcrEventAddr, u64 HwPcrLogInfoAddr,
		u32 NumOfLogEntries)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/** Validate input parameters */
	if ((InstancePtr == NULL) || (HwPcrEventAddr == 0U) || (HwPcrLogInfoAddr == 0U) ||
		(NumOfLogEntries == 0U) || (NumOfLogEntries > XOCP_MAX_NUM_OF_HWPCR_EVENTS)) {
		Status = (int)XST_INVALID_PARAM;
		goto END;
	}
	if (InstancePtr->MailboxPtr == NULL) {
		Status = (int)XST_INVALID_PARAM;
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
/** @} */