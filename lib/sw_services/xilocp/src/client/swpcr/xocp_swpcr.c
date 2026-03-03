/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xocp_swpcr.c
* @addtogroup xocp_swpcr_client_apis XilOcp SwPcr Client APIs
* @{
*
* This file contains the implementation of the client interface functions for Software PCR API's.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ------------------------------------------------------------------------------
* 1.7   rpu  02/18/26 Initial release
*
* </pre>
*
* @note
*
***************************************************************************************************/

/*************************************** Include Files ********************************************/
#include "xocp_swpcr.h"

/************************************* Constant Definitions ***************************************/

/*************************************** Type Definitions *****************************************/

/****************************** Macros (Inline Functions) Definitions *****************************/

/************************************** Function Prototypes ***************************************/

/************************************** Variable Definitions **************************************/

/************************************** Function Definitions **************************************/

/**************************************************************************************************/
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
 **************************************************************************************************/
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

/**************************************************************************************************/
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
 **************************************************************************************************/
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

/**************************************************************************************************/
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
 **************************************************************************************************/
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

/**************************************************************************************************/
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
 **************************************************************************************************/
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
/** @} */