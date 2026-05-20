/******************************************************************************
* Copyright (c) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuseclient.c
*
* This file contains the implementation of the client interface functions for
* eFUSE programming.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kal  07/29/21 Initial release
*       kpt  08/27/21 Added client API's to support puf helper data efuse
*                     programming
* 1.1   kpt  11/29/21 Replaced Xil_DCacheFlushRange with
*                     XNvm_DCacheFlushRange
*       kpt  01/13/22 Allocated CDO structure's in shared memory set by the
*                     user
*       am   02/28/22 Fixed MISRA C violations
*       kpt  03/16/22 Removed IPI related code and added mailbox support
* 1.2   kal  08/22/22 Corrected revoke id column mask in
*                     XNvm_EfuseWriteRevocationId function
* 3.1   skg  10/04/22 Added SlrIndex as part of payload based on user input
*       skg  10/25/22 Added in body comments for APIs
* 3.2   am   03/09/23 Replaced xnvm payload lengths with xmailbox payload lengths
*       vss  09/19/23 Fixed MISRA-C 8.3 violation
* 3.3   ng   11/22/2023 Fixed doxygen grouping
* 3.7   mb   12/31/2025 Added client API to check AES key CRC
*       mb   01/06/2026 Added client API to check USER key CRC
*       tbk  03/20/26 Added SMC support for client applications
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xnvm_efuseclient.h"
#include "xil_util.h"
#include "xnvm_generic.h"

/************************** Constant Definitions *****************************/
#define XNVM_REVOKE_ID_COL_MASK		(0x1FU) /**< Column Mask*/
#define XNVM_REVOKE_ID_ROW_SHIFT	(5U) /**< Row shift value*/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program eFuses with
 * 		user provided data.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DataAddr	Address of the data structure where the eFUSE
 * 				data to be programmed is stored
 *
 * @return
 *		- XST_SUCCESS  If the eFUSE programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWrite(const XNvm_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "eFUSE programming Failed \r\n");
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program IV eFuses with
 * 		user provided data.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	IvAddr		Address of the data structure where the eFUSE
 * 				data to be programmed is stored
 * @param	EnvDisFlag	Flag that tells weather to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteIVs(const XNvm_ClientInstance *InstancePtr, const u64 IvAddr,
								const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr *EfuseData = NULL;
	u64 DataAddr;
	u32 Size;
	u32 TotalSize = sizeof(XNvm_EfuseDataAddr);
	u32 Payload[PAYLOAD_ARG_CNT];

        /**
	 *  - Validate input parameters.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

        /**
	 *  - Link Shared memory for IPI usage.
	 *  If shared memory is not assigned return XST_FAILURE.
	 */
	Size = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EfuseData);

	if ((EfuseData == NULL) || (Size < TotalSize)) {
		goto END;
	}

        /**
	 *  - Zeroize the shared memory.
	 */
	Status = Xil_SMemSet(EfuseData, TotalSize, 0U, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData->EnvMonDisFlag = EnvDisFlag;
	EfuseData->IvAddr = IvAddr;
	DataAddr = (u64)(UINTPTR)EfuseData;

	XNvm_DCacheFlushRange(EfuseData, TotalSize);

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program PPK_INVLD eFuse
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PpkRevoke	Type of PPK_INVLD to be revoked
 * @param	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 *****************************************************************************/
int XNvm_EfuseRevokePpk(const XNvm_ClientInstance *InstancePtr, const XNvm_PpkType PpkRevoke,
								const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr *EfuseData = NULL;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBits = NULL;
	u64 DataAddr;
	u32 Size;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 TotalSize = sizeof(XNvm_EfuseDataAddr) + sizeof(XNvm_EfuseMiscCtrlBits);

        /**
	 *  - Validate input parameters.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 *  - Link shared memory for IPI usage.
	 *  If shared memory is not assigned return XST_FAILURE.
	 */
	Size = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EfuseData);

	if ((EfuseData == NULL) || (Size < TotalSize)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	MiscCtrlBits = (XNvm_EfuseMiscCtrlBits*)(UINTPTR)((u8*)EfuseData + sizeof(XNvm_EfuseDataAddr));

	/**
	 *  - Zeroize the shared memory.
	 */
	Status = Xil_SMemSet(EfuseData, sizeof(XNvm_EfuseDataAddr), 0U, sizeof(XNvm_EfuseDataAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(MiscCtrlBits, sizeof(XNvm_EfuseMiscCtrlBits), 0U, sizeof(XNvm_EfuseMiscCtrlBits));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (PpkRevoke == XNVM_EFUSE_PPK0) {
		MiscCtrlBits->Ppk0Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK1) {
		MiscCtrlBits->Ppk1Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK2) {
		MiscCtrlBits->Ppk2Invalid = TRUE;
	}
#ifdef XNVM_EN_ADD_PPKS
	else if (PpkRevoke == XNVM_EFUSE_PPK3) {
		MiscCtrlBits->Ppk3Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK4) {
		MiscCtrlBits->Ppk4Invalid = TRUE;
	}
#endif
	else {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	EfuseData->EnvMonDisFlag = EnvDisFlag;
	EfuseData->MiscCtrlAddr = (UINTPTR)MiscCtrlBits;
	DataAddr = (u64)(UINTPTR)EfuseData;

	XNvm_DCacheFlushRange(EfuseData, TotalSize);

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program revoke id eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	RevokeId	RevokeId number to be revoked
 * @param 	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationId(const XNvm_ClientInstance *InstancePtr, const u32 RevokeId,
											const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr *EfuseData = NULL;
	XNvm_EfuseRevokeIds *WriteRevokeId = NULL;
	u32 RevokeIdRow;
	u32 RevokeIdBit;
	u64 DataAddr;
	u32 Size;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 TotalSize = sizeof(XNvm_EfuseDataAddr) + sizeof(XNvm_EfuseRevokeIds);

        /**
	 *  - Validate input parameters.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 *  - Link shared memory for IPI usage.
	 *  If shared memory is not assigned return XST_FAILURE.
	 */
	Size = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EfuseData);

	if ((EfuseData == NULL) || (Size < TotalSize)) {
		goto END;
	}

	WriteRevokeId = (XNvm_EfuseRevokeIds*)(UINTPTR)((u8*)EfuseData + sizeof(XNvm_EfuseDataAddr));

	/**
	 *  - Zeroize the shared memory.
	 */
	Status = Xil_SMemSet(EfuseData, sizeof(XNvm_EfuseDataAddr), 0U, sizeof(XNvm_EfuseDataAddr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = Xil_SMemSet(WriteRevokeId, sizeof(XNvm_EfuseRevokeIds), 0U, sizeof(XNvm_EfuseRevokeIds));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	RevokeIdRow = RevokeId >> XNVM_REVOKE_ID_ROW_SHIFT;
	RevokeIdBit = RevokeId & XNVM_REVOKE_ID_COL_MASK;

	if (RevokeIdRow > (XNVM_NUM_OF_REVOKE_ID_FUSES - 1U)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	WriteRevokeId->RevokeId[RevokeIdRow] = ((u32)1U << RevokeIdBit);
	WriteRevokeId->PrgmRevokeId = TRUE;

	EfuseData->RevokeIdAddr = (UINTPTR)WriteRevokeId;
	EfuseData->EnvMonDisFlag = EnvDisFlag;
	DataAddr = (u64)(UINTPTR)EfuseData;

	XNvm_DCacheFlushRange(EfuseData, TotalSize);

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program User eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	UserFuseAddr	Address of the XNvm_EfuseUserData structure
 * 				where the user provided data to be programmed
 * @param	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteUserFuses(const XNvm_ClientInstance *InstancePtr, const u64 UserFuseAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr *EfuseData = NULL;
	u64 DataAddr;
	u32 Size;
	u32 Payload[PAYLOAD_ARG_CNT];
	u32 TotalSize = sizeof(XNvm_EfuseDataAddr);

        /**
	 *  - Validate input parameters.
	 *  Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 *  - Link shared memory for IPI usage.
	 *  If shared memory is not assigned return XST_FAILURE.
	 */
	Size = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&EfuseData);

	if ((EfuseData == NULL) || (Size < TotalSize)) {
		goto END;
	}

	/**
	 *  - Zeroize the shared memory.
	 */
	Status = Xil_SMemSet(EfuseData, TotalSize, 0U, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfuseData->EnvMonDisFlag = EnvDisFlag;
	EfuseData->UserFuseAddr = UserFuseAddr;
	DataAddr = (u64)(UINTPTR)EfuseData;

	XNvm_DCacheFlushRange(EfuseData, TotalSize);

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read IV eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	IvAddr		Address of the output buffer to store the
 * 				IV eFuse data
 * @param	IvType		Type of the IV to read out
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(const XNvm_ClientInstance *InstancePtr, const u64 IvAddr, const XNvm_IvType IvType)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD3(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_IV),
				IvType,
				IvAddr,
				(IvAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Revocation ID eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	RevokeIdAddr	Address of the output buffer to store the
 * 				Revocation ID eFuse data
 * @param 	RevokeIdNum	Revocation ID to be read out
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadRevocationId(const XNvm_ClientInstance *InstancePtr, const u64 RevokeIdAddr,
			const XNvm_RevocationId RevokeIdNum)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD3(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_REVOCATION_ID),
				RevokeIdNum,
				RevokeIdAddr,
				(RevokeIdAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read User eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	UserFuseAddr	Address of the output buffer to store the
 * 				User eFuse data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuses(const XNvm_ClientInstance *InstancePtr, const u64 UserFuseAddr)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_USER_FUSES),
				UserFuseAddr,
				(UserFuseAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read MiscCtrlBits
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	MiscCtrlBits	Address of the output buffer to store the
 * 				MiscCtrlBits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadMiscCtrlBits(const XNvm_ClientInstance *InstancePtr, const u64 MiscCtrlBits)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_MISC_CTRL_BITS),
				MiscCtrlBits,
				(MiscCtrlBits >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecCtrlBits
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	SecCtrlBits	Address of the output buffer to store the
 * 				SecCtrlBits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(const XNvm_ClientInstance *InstancePtr, const u64 SecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_SEC_CTRL_BITS),
				SecCtrlBits,
				(SecCtrlBits >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecMisc1Bits
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	SecMisc1Bits	Address of the output buffer to store the
 * 				SecMisc1Bits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecMisc1Bits(const XNvm_ClientInstance *InstancePtr, const u64 SecMisc1Bits)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_SEC_MISC1_BITS),
				SecMisc1Bits,
				(SecMisc1Bits >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
		return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read BootEnvCtrlBits
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	BootEnvCtrlBits	Address of the output buffer to store the
 * 				BootEnvCtrlBits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadBootEnvCtrlBits(const XNvm_ClientInstance *InstancePtr, const u64 BootEnvCtrlBits)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_BOOT_ENV_CTRL_BITS),
				BootEnvCtrlBits,
				(BootEnvCtrlBits >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PufSecCtrlBits
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufSecCtrlBits	Address of the output buffer to store the
 * 				PufSecCtrlBits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPufSecCtrlBits(const XNvm_ClientInstance *InstancePtr, const u64 PufSecCtrlBits)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill IPI Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_PUF_SEC_CTRL_BITS),
				PufSecCtrlBits,
				(PufSecCtrlBits >> XNVM_ADDR_HIGH_SHIFT));

	 /** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read OffChip ID eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	OffChidIdAddr	Address of the output buffer to store the
 * 				OffChip ID eFuse data
 * @param	OffChipIdNum	OffChip ID number to be read out
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadOffchipRevokeId(const XNvm_ClientInstance *InstancePtr, const u64 OffChidIdAddr,
	const XNvm_OffchipId OffChipIdNum)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD3(Payload,  (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_OFFCHIP_REVOCATION_ID),
				OffChipIdNum,
				OffChidIdAddr,
				(OffChidIdAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PpkHash
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PpkHashAddr	Address of the output buffer to store the
 * 				PpkHashAddr eFuses data
 * @param 	PpkHashType	Type of the PpkHash to be read out
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(const XNvm_ClientInstance *InstancePtr, const u64 PpkHashAddr, const XNvm_PpkType PpkHashType)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD3(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_PPK_HASH),
				PpkHashType,
				PpkHashAddr,
				(PpkHashAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DecEfuseOnly eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DecOnlyAddr	Address of the output buffer to store the
 * 				DecEfuseOnly eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(const XNvm_ClientInstance *InstancePtr, const u64 DecOnlyAddr)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_DEC_EFUSE_ONLY),
				DecOnlyAddr,
				(DecOnlyAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DNA eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DnaAddr		Address of the output buffer to store the
 * 				DNA eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(const XNvm_ClientInstance *InstancePtr, const u64 DnaAddr)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_DNA),
				DnaAddr,
				(DnaAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

#ifdef XNVM_ACCESS_PUF_USER_DATA

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf as User eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufUserFuseAddr	Address of the XNvm_EfusePufFuseAddr structure
 * 				where the user provided data to be programmed
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePufAsUserFuses(XNvm_ClientInstance *InstancePtr, const u64 PufUserFuseAddr)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_PUF_USER_FUSE_WRITE),
				PufUserFuseAddr,
				(PufUserFuseAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Puf User eFuses
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufUserFuseAddr	Address of the output buffer to store the
 * 				Puf User eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPufAsUserFuses(XNvm_ClientInstance *InstancePtr, const u64 PufUserFuseAddr)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_PUF_USER_FUSE),
				PufUserFuseAddr,
				(PufUserFuseAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}
#else

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf helper data
 * 		requested by the user.
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufHdAddr	Address of the XNvm_EfusePufHdAddr structure
 * 				where the user provided helper data to be programmed
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(const XNvm_ClientInstance *InstancePtr, const u64 PufHdAddr) {
	volatile int Status = XST_FAILURE;
	u64 DataAddr;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	DataAddr = PufHdAddr;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(XNvm_EfusePufHdAddr));

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_WRITE_PUF),
				DataAddr,
				(DataAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Puf helper data
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufHdAddr	Address of the output buffer to store the
 * 				Puf helper data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XST_FAILURE  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPuf(const XNvm_ClientInstance *InstancePtr, const u64 PufHdAddr) {
	volatile int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_READ_PUF),
				PufHdAddr,
				(PufHdAddr >> XNVM_ADDR_HIGH_SHIFT));

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}
#endif

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to check given CRC is matched with calculated CRC.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	AesKeyCrc	CRC value of the AES key which needs to be checked
 * @param	AesKeyType	Type of AES key
 * 				Supported key types:
 * 				- XNVM_EFUSE_AES_KEY
 * 				- XNVM_EFUSE_USER_KEY0
 * 				- XNVM_EFUSE_USER_KEY1
 *
 * @return
 *		- XST_SUCCESS  If the CRC matches
 *		- XST_INVALID_PARAM  If there is a input validation failure
 *		- XNVM_EFUSE_ERR_CRC_VERIFICATION  If there is a CRC verification failure
 *
 ******************************************************************************/
int XNvm_EfuseAesKeyCrcCheck(const XNvm_ClientInstance *InstancePtr, const u32 AesKeyCrc, const XNvm_AesKeyType AesKeyType)
{
	int Status = XST_FAILURE;
	u32 Payload[PAYLOAD_ARG_CNT];

	/**
	 * - Validate input parameters.
	 * Return XST_INVALID_PARAM if input parameters are invalid.
	 */
	if (InstancePtr == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/** - Fill Payload */
	XNVM_PACK_PAYLOAD2(Payload, (u32)(((InstancePtr->SlrIndex) << XNVM_SLR_INDEX_SHIFT)
				| (u32)XNVM_API_ID_EFUSE_CHECK_AES_KEY_CRC),
				AesKeyCrc,
				AesKeyType);

	/** - Send request using generic API */
	Status = XNvm_SendRequest(InstancePtr, Payload, (u32)PAYLOAD_ARG_CNT, NULL, 0U);
END:
	return Status;
}
