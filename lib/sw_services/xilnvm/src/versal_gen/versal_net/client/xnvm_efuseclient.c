/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
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
* 3.0  har  07/19/22  Initial release
* 3.1  skg  10/28/22  Added In body comments for APIs
* 3.2  har  02/21/23  Added support for writing Misc Ctrl bits and ROM Rsvd bits
*      am   03/09/23  Replaced xnvm payload lengths with xmailbox payload lengths
* 	   vek  05/31/23  Added support for Programming PUF secure control bits
*      kpt  09/02/23  Avoid returning XST_SUCCESS incase of fault injection
*      yog  09/13/23  Fixed review comments
* 3.4  kal  09/26/24  Updated AES, PPK and IV functions to pass EnvMonDis flag
*                     to server through IPI commands
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xil_util.h"
#include "xnvm_efuseclient.h"
#include "xnvm_efuseclient_hw.h"
#include "xnvm_mailbox.h"
#include "xnvm_validate.h"
#include "xnvm_defs.h"

/************************** Constant Definitions *****************************/
#define XNVM_ADDR_HIGH_SHIFT					(32U)
#define XNVM_MAX_PAYLOAD_LEN					(7U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK		(0x00ffffffU)

/************************** Function Prototypes ******************************/
static void XNvm_EfuseCreateWriteKeyCmd(XNvm_AesKeyWriteCdo* AesKeyWrCdo, XNvm_AesKeyType KeyType, u32 AddrLow, u32 AddrHigh, u32 EnvMonDis);
static void XNvm_EfuseCreateWritePpkCmd(XNvm_PpkWriteCdo* PpkWrCdo, XNvm_PpkType PpkType, u32 AddrLow, u32 AddrHigh, u32 EnvMonDis);
static void XNvm_EfuseCreateWriteIvCmd(XNvm_IvWriteCdo* IvWrCdo, XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh, u32 EnvMonDis);
static void XNvm_EfuseCreateReadEfuseCacheCmd(XNvm_RdCacheCdo* RdCacheCdo, u16 StartOffset, u8 RegCount, u32 AddrLow, u32 AddrHigh);
static void XNvm_EfuseCreateWritePufCmd(XNvm_PufWriteCdo* PufWrCdo, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseValidateNdWriteAesKey(const XNvm_ClientInstance *InstancePtr, XNvm_AesKeyWriteCdo *KeyWrCdo,
		XNvm_AesKeyType KeyType, u64 Addr, u32 EnvMonDis);
static int XNvm_EfuseValidatNdWritePpkHash(const XNvm_ClientInstance *InstancePtr, XNvm_PpkWriteCdo *PpkWrCdo,
		XNvm_PpkType PpkType, u64 Addr, u32 EnvMonDis);
static int XNvm_EfuseValidateNdWriteIv(const XNvm_ClientInstance *InstancePtr, XNvm_IvWriteCdo *IvWrCdo,
		XNvm_IvType IvType, u64 Addr, u32 EnvMonDis);

/************************** Variable Definitions *****************************/
/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program eFuses with
 * 		user provided data
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	DataAddr	Address of the data structure where the eFUSE
 * 				data to be programmed is stored
 *
 * @return
 * 		- XST_SUCCESS  If the eFUSE programming is successful
 * 		- ErrorCode  If there is a failure
 *
 *@section Implementation
 ******************************************************************************/
int XNvm_EfuseWrite(XNvm_ClientInstance *InstancePtr, const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_EfuseWriteDataAddr *EfuseData = (XNvm_EfuseWriteDataAddr *)DataAddr;
	XNvm_EfuseAesKeys *AesKeys = (XNvm_EfuseAesKeys *)EfuseData->AesKeyAddr;
	XNvm_EfusePpkHash *EfusePpk = (XNvm_EfusePpkHash *)EfuseData->PpkHashAddr;
	XNvm_EfuseIvs *Ivs = (XNvm_EfuseIvs *)EfuseData->IvAddr;
	XNvm_AesKeyWriteCdo *KeyWrCdo = (XNvm_AesKeyWriteCdo *)Payload;
	XNvm_PpkWriteCdo *PpkWrCdo = (XNvm_PpkWriteCdo *)Payload;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 *  @{ Validate AES,UsrKey,PPK0,PPK1,PPK2,IVs write request.
	 *  If validation is success, Send XNvm_EfuseWrite CDO over IPI to the PLM.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *  If the timeout exceeds then return the error else return status of the IPI response.
	 */
	if (AesKeys->PrgmAesKey == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming AES Key \r\n");
		Status = XNvm_EfuseValidateNdWriteAesKey(InstancePtr, KeyWrCdo, XNVM_EFUSE_AES_KEY,
					(u64)(UINTPTR)(AesKeys->AesKey), (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "AES Key write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed AES Key \r\n");
		}
	}

	if (AesKeys->PrgmUserKey0 == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming User Key 0 \r\n");
		Status = XNvm_EfuseValidateNdWriteAesKey(InstancePtr, KeyWrCdo, XNVM_EFUSE_USER_KEY_0,
					(u64)(UINTPTR)(AesKeys->UserKey0), (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "USER Key0 write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed User key 0 \r\n");
		}
	}

	if (AesKeys->PrgmUserKey1 == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming User Key 1 \r\n");
		Status = XNvm_EfuseValidateNdWriteAesKey(InstancePtr, KeyWrCdo, XNVM_EFUSE_USER_KEY_1,
					(u64)(UINTPTR)(AesKeys->UserKey1), (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "User Key1 write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed User key 1 \r\n");
		}
	}

	/**
	 *  @{ Validates PPK hash and sends an IPI request to the PLM by using the
	 *     XNVM_API_ID_EFUSE_WRITE_PPK_HASH CDO command.
	 *     Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *     If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	if (EfusePpk->PrgmPpk0Hash == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming PPK Hash 0 \r\n");
		Status = XNvm_EfuseValidatNdWritePpkHash(InstancePtr, PpkWrCdo,
				XNVM_EFUSE_PPK0, (u64)(UINTPTR)EfusePpk->Ppk0Hash, (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "PPK0 hash write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PPK0 Hash \r\n");
		}
	}

	if (EfusePpk->PrgmPpk1Hash == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming PPK Hash 1 \r\n");
		Status = XNvm_EfuseValidatNdWritePpkHash(InstancePtr, PpkWrCdo,
				XNVM_EFUSE_PPK1, (u64)(UINTPTR)EfusePpk->Ppk1Hash, (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "PPK1 hash write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PPK1 Hash \r\n");
		}
	}

	if (EfusePpk->PrgmPpk2Hash == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming PPK Hash 2 \r\n");
		Status = XNvm_EfuseValidatNdWritePpkHash(InstancePtr, PpkWrCdo,
				XNVM_EFUSE_PPK2, (u64)(UINTPTR)EfusePpk->Ppk2Hash, (u32)EfuseData->EnvMonDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "PPK2 hash write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PPK2 Hash \r\n");
		}
	}

	/**
	 *  @{ Validates IV and sends an IPI request to the PLM by using the
	 *     XNVM_API_ID_EFUSE_WRITE_IV CDO command.
	 *     Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *     If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XST_FAILURE;
	Status = XNvm_EfuseWriteIVs(InstancePtr, (u64)(UINTPTR)Ivs, (u32)EfuseData->EnvMonDisFlag);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program IV as requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	IvAddr		Address of the IV to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *                      when set to true it will not check for voltage
 *                      and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteIVs(XNvm_ClientInstance *InstancePtr, const u64 IvAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_EfuseIvs *Ivs = NULL;
	u32 Size;
	u32 TotalSize = sizeof(XNvm_EfuseIvs);
	XNvm_IvWriteCdo *IvWrCdo = (XNvm_IvWriteCdo *)Payload;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	/**
	 *  @{ Get shared memory allocated by the user using XMailbox_GetSharedMem API,
	 *  to store data structure in the user memory.
	 *  Perform validation on the size of the shared memory,
	 *  if size is less than the total size return XST_FAILURE.
	 */
	Size = XMailbox_GetSharedMem(InstancePtr->MailboxPtr, (u64**)(UINTPTR)&Ivs);
	if ((Ivs == NULL) || (Size < TotalSize)) {
		goto END;
	}

	Status = Xil_SMemSet(Ivs, TotalSize, 0U, TotalSize);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 *  Fill the EfuseData structure with the IV address, Environmental disable flag.
	 */
	Ivs = (XNvm_EfuseIvs *)IvAddr;

	Xil_DCacheFlushRange((UINTPTR)Ivs, TotalSize);

	/**
	 *  @{ Send XNvm_EfuseWriteIv CDO over IPI to the PLM.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 */
	if (Ivs->PrgmMetaHeaderIv == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming Metaheader IV \r\n");
		Status = XNvm_EfuseValidateNdWriteIv(InstancePtr, IvWrCdo,
				XNVM_EFUSE_META_HEADER_IV_RANGE, (u64)(UINTPTR)(Ivs->MetaHeaderIv),
				EnvDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Metaheader IV write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Metaheader IV \r\n");
		}
	}

	if (Ivs->PrgmBlkObfusIv == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming Black IV \r\n");
		Status = XNvm_EfuseValidateNdWriteIv(InstancePtr, IvWrCdo,
				XNVM_EFUSE_BLACK_IV, (u64)(UINTPTR)(Ivs->BlkObfusIv),
				EnvDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Black IV write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Black IV \r\n");
		}
	}

	if (Ivs->PrgmPlmIv == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming PLM IV \r\n");
		Status = XNvm_EfuseValidateNdWriteIv(InstancePtr, IvWrCdo,
		XNVM_EFUSE_PLM_IV_RANGE, (u64)(UINTPTR)(Ivs->PlmIv),
		EnvDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "PLM IV write failed;"
				"Error Code = %x\r\n", Status);
			goto END;
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PLM IV \r\n");
		}
	}

	if (Ivs->PrgmDataPartitionIv == TRUE) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Started programming Data Parition IV \r\n");
		Status = XNvm_EfuseValidateNdWriteIv(InstancePtr, IvWrCdo,
		XNVM_EFUSE_DATA_PARTITION_IV_RANGE, (u64)(UINTPTR)(Ivs->DataPartitionIv),
		EnvDisFlag);
		if (Status != XST_SUCCESS) {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Data Partition IV write failed;"
				"Error Code = %x\r\n", Status);
		}
		else {
			XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Data Partition IV \r\n");
		}
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}


/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program DICE UDS
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	UdsAddr		Address of the UDS to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *                      when set to true it will not check for voltage
 *                      and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteDiceUds(XNvm_ClientInstance *InstancePtr, const u64 UdsAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_Uds *Uds = (XNvm_Uds *)UdsAddr;
	XNvm_UdsWriteCdo *UdsWriteCdo = (XNvm_UdsWriteCdo *)Payload;

	/**
	 *  Perform input parameter validation on InstancePtr. Return XST_INVALID_PARAM If input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)Uds, sizeof(XNvm_Uds));

	UdsWriteCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_UDS);
	UdsWriteCdo->Pload.EnvDisFlag = (u16)EnvDisFlag;
	UdsWriteCdo->Pload.Reserved = (u16)0x0U;
	UdsWriteCdo->Pload.AddrLow = (u32)UdsAddr;
	UdsWriteCdo->Pload.AddrHigh = (u32)(UdsAddr >> XNVM_ADDR_HIGH_SHIFT);

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_UDS CDO command.
	 *     Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)UdsWriteCdo,
			sizeof(XNvm_UdsWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "UDS write failed; Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed UDS \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program encrypted DME private key
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	DmeKeyType	Type of DME Key which needs to be programmed
 * @param	DmeKeyAddr	Address of the encrypted DME private key to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *                      when set to true it will not check for voltage
 *                      and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_WriteDmePrivateKey(XNvm_ClientInstance *InstancePtr, u32 DmeKeyType, const u64 DmeKeyAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_DmeKey *DmeKey = (XNvm_DmeKey *)DmeKeyAddr;
	XNvm_DmeKeyWriteCdo *DmeKeyWriteCdo = (XNvm_DmeKeyWriteCdo *)Payload;

	/**
	 *  Perform input parameter validation on InstancePtr. Return XST_INVALID_PARAM If input parameters are invalid
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)DmeKey, sizeof(XNvm_DmeKey));

	DmeKeyWriteCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_DME_KEY);
	DmeKeyWriteCdo->Pload.EnvDisFlag = (u16)EnvDisFlag;
	DmeKeyWriteCdo->Pload.DmeKeyType = (u16)DmeKeyType;
	DmeKeyWriteCdo->Pload.AddrLow = (u32)DmeKeyAddr;
	DmeKeyWriteCdo->Pload.AddrHigh = (u32)(DmeKeyAddr >> XNVM_ADDR_HIGH_SHIFT);

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_DME_KEY CDO command.
	 *     Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)DmeKeyWriteCdo,
			sizeof(XNvm_DmeKeyWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "DME key write failed; Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed DME private key \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program DME mode requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	DmeMode		Value of DME mode to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteDmeMode(XNvm_ClientInstance *InstancePtr, u32 DmeMode, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_DmeModeWriteCdo *DmeModeWriteCdo = (XNvm_DmeModeWriteCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	DmeModeWriteCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_DME_MODE);
	DmeModeWriteCdo->Pload.EnvMonitorDis = EnvDisFlag;
	DmeModeWriteCdo->Pload.DmeMode = DmeMode;

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_DME_MODE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)DmeModeWriteCdo,
			sizeof(XNvm_DmeModeWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "DME mode write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed DME mode bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Secure Control Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	SecCtrlBits	Value of Secure Control  Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteSecCtrlBits(XNvm_ClientInstance *InstancePtr, u32 SecCtrlBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_SecCtrlBitsWriteCdo *SecCtrlBitsWrCdo = (XNvm_SecCtrlBitsWriteCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	SecCtrlBitsWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS);
	SecCtrlBitsWrCdo->Pload.EnvMonitorDis = EnvDisFlag;
	SecCtrlBitsWrCdo->Pload.SecCtrlBits = SecCtrlBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)SecCtrlBitsWrCdo,
			sizeof(XNvm_SecCtrlBitsWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Secure Control Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Security Control bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf Control Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	PufCtrlBits	Value of Puf Control  Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePufCtrlBits(XNvm_ClientInstance *InstancePtr, u32 PufCtrlBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_PufCtrlBitsWriteCdo *PufCtrlBitsWrCdo = (XNvm_PufCtrlBitsWriteCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	PufCtrlBitsWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS);
	PufCtrlBitsWrCdo->Pload.EnvMonitorDis = EnvDisFlag;
	PufCtrlBitsWrCdo->Pload.PufCtrlBits = PufCtrlBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)PufCtrlBitsWrCdo,
			sizeof(XNvm_PufCtrlBitsWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Puf Control Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PUF Control bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Misc Control Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	MiscCtrlBits	Value of Misc Control Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteMiscCtrlBits(XNvm_ClientInstance *InstancePtr, u32 MiscCtrlBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_MiscCtrlBitsWriteCdo *MiscCtrlBitsWrCdo = (XNvm_MiscCtrlBitsWriteCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	MiscCtrlBitsWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS);
	MiscCtrlBitsWrCdo->Pload.EnvMonitorDis = EnvDisFlag;
	MiscCtrlBitsWrCdo->Pload.MiscCtrlBits = MiscCtrlBits;

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)MiscCtrlBitsWrCdo,
			sizeof(XNvm_MiscCtrlBitsWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Misc Control Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Misc Control bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program BootMode disable efuse bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	BootModeDisBits	Value of Boot mode disable Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteBootModeDis(XNvm_ClientInstance *InstancePtr, u32 BootModeDisBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_BootModeDisCdo *BootModeDisCdo = (XNvm_BootModeDisCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	BootModeDisCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE);
	BootModeDisCdo->Pload.EnvDisFlag = EnvDisFlag;
	BootModeDisCdo->Pload.BootModeDisVal = BootModeDisBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE CDO command.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)BootModeDisCdo,
			sizeof(XNvm_BootModeDisCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Boot Mode disable write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Boot mode disable bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}


/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Sec Misc 1 Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	SecMisc1Bits	Value of Sec Misc 1 Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteSecMisc1Bits(XNvm_ClientInstance *InstancePtr, u32 SecMisc1Bits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_SecMisc1BitsCdo *SecMisc1BitsCdo = (XNvm_SecMisc1BitsCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	SecMisc1BitsCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS);
	SecMisc1BitsCdo->Pload.EnvDisFlag = EnvDisFlag;
	SecMisc1BitsCdo->Pload.Misc1CtrlBitsVal = SecMisc1Bits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS CDO command.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)SecMisc1BitsCdo,
			sizeof(XNvm_SecMisc1BitsCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Sec Misc 1 Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Security Misc 1 bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Boot Env Ctrl Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	BootEnvCtrlBits	Value of BootEnvCtrl Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteBootEnvCtrlBits(XNvm_ClientInstance *InstancePtr, u32 BootEnvCtrlBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_BootEnvCtrlBitsCdo *BootEnvCtrlBitsCdo = (XNvm_BootEnvCtrlBitsCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	BootEnvCtrlBitsCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS);
	BootEnvCtrlBitsCdo->Pload.EnvDisFlag = EnvDisFlag;
	BootEnvCtrlBitsCdo->Pload.BootEnvCtrlVal = BootEnvCtrlBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS CDO command.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)BootEnvCtrlBitsCdo,
			sizeof(XNvm_BootEnvCtrlBitsCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Boot Env Ctrl Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Boot environment control bits \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program ROM Rsvd Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	RomRsvdBits	Value of ROM Rsvd Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteRomRsvdBits(XNvm_ClientInstance *InstancePtr, u32 RomRsvdBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RomRsvdBitsWriteCdo *RomRsvdBitsWriteCdo = (XNvm_RomRsvdBitsWriteCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	RomRsvdBitsWriteCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_ROM_RSVD);
	RomRsvdBitsWriteCdo->Pload.EnvMonitorDis = EnvDisFlag;
	RomRsvdBitsWriteCdo->Pload.RomRsvdBits = RomRsvdBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_ROM_RSVD CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RomRsvdBitsWriteCdo,
			sizeof(XNvm_RomRsvdBitsWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Rom Reserved Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed ROM reserved bits \r\n");
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Glitch Cfg Bits
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	GlitchCfgBits	Value of Glitch Cfg Bits to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 *		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteGlitchConfigBits(XNvm_ClientInstance *InstancePtr, u32 GlitchCfgBits, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_GlitchConfigCdo *GlitchConfigCdo = (XNvm_GlitchConfigCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	GlitchConfigCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG);
	GlitchConfigCdo->Pload.EnvDisFlag = EnvDisFlag;
	GlitchConfigCdo->Pload.GlitchConfigVal = GlitchCfgBits;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)GlitchConfigCdo,
			sizeof(XNvm_GlitchConfigCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Glitch Config Bits write failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Glitch configuration bits \r\n");
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program PLM update bit
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePlmUpdate(XNvm_ClientInstance *InstancePtr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_DisPlmUpdateCdo *DisPlmUpdateCdo = (XNvm_DisPlmUpdateCdo *)Payload;

	/**
	 * Perform input parameter validation on InstancePtr.
	 * Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	DisPlmUpdateCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE);
	DisPlmUpdateCdo->Pload.EnvDisFlag = EnvDisFlag;

	/**
	 *  @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)DisPlmUpdateCdo,
			sizeof(XNvm_DisPlmUpdateCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Write disable PLM update failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed eFuse bit to disable PLM update \r\n");
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Dec Only eFuses.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteDecOnly(XNvm_ClientInstance *InstancePtr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_DecOnlyCdo *DecOnlyCdo = (XNvm_DecOnlyCdo *)Payload;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	DecOnlyCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_DEC_ONLY);
	DecOnlyCdo->Pload.EnvDisFlag = EnvDisFlag;

	/**
	 *  @{ Send XNvm_EfuseWriteSecCtrl CDO over IPI to the PLM.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)DecOnlyCdo,
			sizeof(XNvm_DecOnlyCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Write dec only efuses failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Decrypt only eFuse bits \r\n");
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program FIPS info eFuses.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	FipsMode	FIPS mode which needs to be programmed
 * @param	FipsVersion	FIPS version which needs to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteFipsInfo(XNvm_ClientInstance *InstancePtr, const u16 FipsMode, const u16 FipsVersion, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_FipsInfoCdo *FipsInfoCdo = (XNvm_FipsInfoCdo *)Payload;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	FipsInfoCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_FIPS_INFO);
	FipsInfoCdo->Pload.EnvDisFlag = EnvDisFlag;
	FipsInfoCdo->Pload.FipsMode = FipsMode;
	FipsInfoCdo->Pload.FipsVersion = FipsVersion;

	/**
	 *  @{ Send XNvm_EfuseWritePufCtrl CDO over IPI request to the PLM.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)FipsInfoCdo,
			sizeof(XNvm_FipsInfoCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Write FIPS info efuses failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed eFuse bits related to FIPS \r\n");
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Revocation Id eFuses.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	RevokeIdNum	Revocation ID number which needs to be programmed
 * @param	EnvDisFlag	Environmental monitoring flag set by the user.
 *				When set to true it will not check for voltage
 *				and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationId(XNvm_ClientInstance *InstancePtr, const u32 RevokeIdNum, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RevokeIdCdo *RevokeIdCdo = (XNvm_RevokeIdCdo *)Payload;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	RevokeIdCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID);
	RevokeIdCdo->Pload.EnvDisFlag = EnvDisFlag;
	RevokeIdCdo->Pload.RevokeIdNum = RevokeIdNum;

	/** @{ Send XNvm_EfuseWriteMiscCtrl CDO over IPI request to the PLM.
	 *  Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RevokeIdCdo,
			sizeof(XNvm_RevokeIdCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Write revocation Id efuses failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Revocation ID %x \r\n", RevokeIdNum);
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program off chip Revocation Id eFuses.
 *
 * @param	InstancePtr		Pointer to the client instance
 * @param	OffChipRevokeIdNum	Off Chip Revocation ID number which needs to be programmed
 * @param	EnvDisFlag		Environmental monitoring flag set by the user.
 *					When set to true it will not check for voltage
 *					and temparature limits.
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteOffChipRevocationId(XNvm_ClientInstance *InstancePtr, const u32 OffChipRevokeIdNum, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_OffChipIdCdo *OffChipIdCdo = (XNvm_OffChipIdCdo *)Payload;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	OffChipIdCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID);
	OffChipIdCdo->Pload.EnvDisFlag = EnvDisFlag;
	OffChipIdCdo->Pload.OffChipIdNum = OffChipRevokeIdNum;

	/**
	 *  @{ Send XNvm_EfuseWriteRomRsvd CDO over IPI request to the PLM.
	 *  Wait for IPI response from PLM with a default timeout of 300 seconds.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)OffChipIdCdo,
			sizeof(XNvm_OffChipIdCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Write off chip revocation Id efuses failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed Off chip revocation ID %x \r\n", OffChipRevokeIdNum);
	}

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf helper data
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	PufHdAddr	Address of the XNvm_EfusePufHdAddr structure
 * 				where the user provided helper data to be programmed
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(XNvm_ClientInstance *InstancePtr, const u64 PufHdAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_EfusePufHdAddr *EfusePuf = (XNvm_EfusePufHdAddr *)PufHdAddr;
	XNvm_PufWriteCdo *PufWrCdo = (XNvm_PufWriteCdo *)Payload;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheFlushRange((UINTPTR)EfusePuf, sizeof(XNvm_EfusePufHdAddr));

	HighAddr = (u32)((UINTPTR)EfusePuf >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)EfusePuf;
	XNvm_EfuseCreateWritePufCmd(PufWrCdo, LowAddr, HighAddr);

	/**
	 * @{ Send XNvm_EfuseWritePuf CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32*)PufWrCdo,
			sizeof(XNvm_PufWriteCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Writing PUF data failed;"
			"Error Code = %x\r\n", Status);
		goto END;
	}
	else {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "Successfully programmed PUF related data \r\n");
	}

	Status = XST_FAILURE;

	Payload[0U] =  Header(0U, (u32)XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS);
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, Payload, XMAILBOX_PAYLOAD_LEN_1U);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Puf data from the eFUSE cache.
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	PufHdAddr	Address of the XNvm_EfusePufHdAddr structure
 * 				where the user provided helper data to be programmed
 *
 * @return
 * 		- XST_SUCCESS  If the programming is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPuf(XNvm_ClientInstance *InstancePtr, u64 PufHdAddr)
{
	volatile int Status = XST_FAILURE;
	volatile int RetStatus = XST_GLITCH_ERROR;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	XNvm_EfusePufHdAddr *EfusePuf = (XNvm_EfusePufHdAddr *)PufHdAddr;
	u32 ReadPufHd[XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS] = {0U};
	u32 ReadChash = 0U;
	u32 ReadAux = 0U;
	u32 ReadRoSwap = 0U;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)ReadPufHd, (XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS * XNVM_WORD_LEN));
	Xil_DCacheInvalidateRange((UINTPTR)&ReadChash, XNVM_WORD_LEN);
	Xil_DCacheInvalidateRange((UINTPTR)&ReadAux, XNVM_WORD_LEN);
	Xil_DCacheInvalidateRange((UINTPTR)&ReadRoSwap, XNVM_WORD_LEN);

	/**
	 * Read Puf helper data.
	 */
	HighAddr = (u32)((UINTPTR)(ReadPufHd) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)ReadPufHd;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_PUF_SYN_DATA_OFFSET, XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS,
		LowAddr, HighAddr);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)ReadPufHd, (XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS * XNVM_WORD_LEN));

	/**
	 * Read Puf Chash.
	 */
	HighAddr = (u32)((UINTPTR)(&ReadChash) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadChash;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_PUF_CHASH_OFFSET, 1U, LowAddr,
		HighAddr);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadChash, XNVM_WORD_LEN);

	/**
	 * Read Puf Aux data.
	 */
	HighAddr = (u32)((UINTPTR)(&ReadAux) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadAux;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_PUF_ECC_CTRL_OFFSET, 1U, LowAddr,
		HighAddr);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadAux, XNVM_WORD_LEN);

	/**
	 * Read RO Swap.
	 */
	HighAddr = (u32)((UINTPTR)(&ReadRoSwap) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadRoSwap;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_PUF_RO_SWAP_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send XNvm_EfuseReadPuf CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadRoSwap, XNVM_WORD_LEN);

	Status = XST_FAILURE;
	Status = Xil_SMemCpy(EfusePuf->EfuseSynData, XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS, ReadPufHd,
		XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS, XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	EfusePuf->Chash = ReadChash;
	EfusePuf->Aux = ReadAux & XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_ECC_23_0_MASK;
	EfusePuf->RoSwap = ReadRoSwap;
	RetStatus = Status;

END:
	if ((RetStatus != XST_SUCCESS) && (Status != XST_SUCCESS)) {
		RetStatus = Status;
	}

	return RetStatus;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read IV eFuses
 * 		requested by the user
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	IvAddr		Address of the output buffer to store the
 * 				IV eFuse data
 * @param	IvType		Type of the IV to read out
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(XNvm_ClientInstance *InstancePtr, const u64 IvAddr,
	const XNvm_IvType IvType)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo = (XNvm_RdCacheCdo*)Payload;
	u16 StartOffset = 0U;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch(IvType) {
		case XNVM_EFUSE_META_HEADER_IV_RANGE:
			StartOffset = XNVM_EFUSE_CACHE_METAHEADER_IV_RANGE_0_OFFSET;
			break;
		case XNVM_EFUSE_BLACK_IV:
			StartOffset = XNVM_EFUSE_CACHE_BLACK_IV_0_OFFSET;
			break;
		case XNVM_EFUSE_PLM_IV_RANGE:
			StartOffset = XNVM_EFUSE_CACHE_PLM_IV_RANGE_0_OFFSET;
			break;
		case XNVM_EFUSE_DATA_PARTITION_IV_RANGE:
			StartOffset = XNVM_EFUSE_CACHE_DATA_PARTITION_IV_RANGE_0_OFFSET;
			break;
		default:
			Status =  (int)XST_INVALID_PARAM;
			break;
	}

	if (Status == XST_INVALID_PARAM) {
		goto END;
	}

	HighAddr = (u32)(IvAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)IvAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, StartOffset, XNVM_EFUSE_IV_LEN_IN_WORDS, LowAddr,
		HighAddr);

	/**
	 * @{ Send XNvm_EfuseReadIv CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceed return error else return the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Revocation ID eFuses
 * 		requested by the user
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	RevokeIdAddr	Address of the output buffer to store the
 * 				Revocation ID eFuse data
 * @param 	RevokeIdNum	Revocation ID to be read out
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadRevocationId(XNvm_ClientInstance *InstancePtr, const u64 RevokeIdAddr,
			const XNvm_RevocationId RevokeIdNum)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u16 StartOffset = XNVM_EFUSE_CACHE_REVOCATION_ID_0_OFFSET + ((u16)RevokeIdNum * XNVM_WORD_LEN);
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(RevokeIdAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)RevokeIdAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, StartOffset, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send XNvm_EfuseReadRevocationId CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read User eFuses
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	UserFuseAddr	Address of the output buffer to store the
 * 				User eFuse data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuses(XNvm_ClientInstance *InstancePtr, u64 UserFuseAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	XNvm_EfuseUserDataAddr *UserFuseData = (XNvm_EfuseUserDataAddr *)UserFuseAddr;
	u16 StartOffset = XNVM_EFUSE_CACHE_USER_FUSE_START_OFFSET + UserFuseData->StartUserFuseNum * XNVM_WORD_LEN;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Status = XNvm_IsDmeModeEn();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	HighAddr = (u32)((UINTPTR)(UserFuseData->UserFuseDataAddr) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)UserFuseData->UserFuseDataAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, StartOffset, UserFuseData->NumOfUserFuses, LowAddr,
		HighAddr);
	/**
	 * @{ Send XNvm_EfuseReadUserFuses CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read MiscCtrlBits
 * 		requested by the user
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	MiscCtrlBits	Address of the output buffer to store the
 * 				MiscCtrlBits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadMiscCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 MiscCtrlBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadReg;
	XNvm_EfuseMiscCtrlBits *MiscCtrlBitsData = (XNvm_EfuseMiscCtrlBits *)MiscCtrlBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_MISC_CTRL_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send EfuseReadMiscCtrlBits CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	MiscCtrlBitsData->GlitchDetHaltBootEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_HALT_BOOT_EN_1_0_SHIFT);
	MiscCtrlBitsData->GlitchDetRomMonitorEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_GD_ROM_MONITOR_EN_SHIFT);
	MiscCtrlBitsData->HaltBootError =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ERROR_1_0_SHIFT);
	MiscCtrlBitsData->HaltBootEnv =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_HALT_BOOT_ENV_1_0_SHIFT);
	MiscCtrlBitsData->CryptoKatEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_CRYPTO_KAT_EN_SHIFT);
	MiscCtrlBitsData->LbistEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_LBIST_EN_SHIFT);
	MiscCtrlBitsData->SafetyMissionEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_SAFETY_MISSION_EN_SHIFT);
	MiscCtrlBitsData->Ppk0Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK0_INVLD_1_0_SHIFT);
	MiscCtrlBitsData->Ppk1Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK1_INVLD_1_0_SHIFT);
	MiscCtrlBitsData->Ppk2Invalid =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_MASK) >>
		XNVM_EFUSE_CACHE_MISC_CTRL_PPK2_INVLD_1_0_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecCtrlBits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	SecCtrlBits	Address of the output buffer to store the
 * 				SecCtrlBits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 SecCtrlBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadReg;
	XNvm_EfuseSecCtrlBits *SecCtrlBitsData = (XNvm_EfuseSecCtrlBits *)SecCtrlBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send EfuseReadSecCtrlBits CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	SecCtrlBitsData->AesDis =
		(u8)(ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_DIS_MASK);
	SecCtrlBitsData->JtagErrOutDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROUT_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_ERROR_OUT_DIS_SHIFT);
	SecCtrlBitsData->JtagDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_JTAG_DIS_SHIFT);
	SecCtrlBitsData->HwTstBitsDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_HWTSTBITS_DIS_SHIFT);
	SecCtrlBitsData->Ppk0WrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK0_WR_LK_SHIFT);
	SecCtrlBitsData->Ppk1WrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK1_WR_LK_SHIFT);
	SecCtrlBitsData->Ppk2WrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PPK2_WR_LK_SHIFT);
	SecCtrlBitsData->AesCrcLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_CRC_LK_1_0_SHIFT);
	SecCtrlBitsData->AesWrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AES_WR_LK_SHIFT);
	SecCtrlBitsData->UserKey0CrcLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_CRC_LK_0_SHIFT);
	SecCtrlBitsData->UserKey0WrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_0_WR_LK_SHIFT);
	SecCtrlBitsData->UserKey1CrcLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_CRC_LK_0_SHIFT);
	SecCtrlBitsData->UserKey1WrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_USR_KEY_1_WR_LK_SHIFT);
	SecCtrlBitsData->SecDbgDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_DIS_1_0_SHIFT);
	SecCtrlBitsData->SecLockDbgDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_LOCK_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_AUTH_JTAG_LOCK_DIS_1_0_SHIFT);
	SecCtrlBitsData->PmcScEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PMC_SC_EN_2_0_SHIFT);
	SecCtrlBitsData->BootEnvWrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_BOOT_ENV_WR_LK_SHIFT);
	SecCtrlBitsData->RegInitDis =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_REG_INIT_DIS_1_0_SHIFT);
	SecCtrlBitsData->UdsWrLk =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_UDS_WR_LK_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecMisc1Bits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	SecMisc1Bits	Address of the output buffer to store the
 * 				SecMisc1Bits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecMisc1Bits(XNvm_ClientInstance *InstancePtr, const u64 SecMisc1Bits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadReg;
	XNvm_EfuseSecMisc1Bits *SecMisc1BitsData = (XNvm_EfuseSecMisc1Bits *)SecMisc1Bits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Perform input parameter validation on InstancePtr.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_SEC_MISC1_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send EfuseReadSecMisc1Bits CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	SecMisc1BitsData->LpdMbistEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_MBIST_EN_2_0_SHIFT);
	SecMisc1BitsData->PmcMbistEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_PMC_MBIST_EN_2_0_SHIFT);
	SecMisc1BitsData->LpdNocScEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_LPD_NOC_SC_EN_2_0_SHIFT);
	SecMisc1BitsData->SysmonVoltMonEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_VOLT_MON_EN_1_0_SHIFT);
	SecMisc1BitsData->SysmonTempMonEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_MASK) >>
		XNVM_EFUSE_CACHE_SEC_MISC_1_SYSMON_TEMP_MON_EN_1_0_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read BootEnvCtrlBits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	BootEnvCtrlBits	Address of the output buffer to store the
 * 				BootEnvCtrlBits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadBootEnvCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 BootEnvCtrlBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadReg;
	XNvm_EfuseBootEnvCtrlBits *BootEnvCtrlBitsData = (XNvm_EfuseBootEnvCtrlBits *)BootEnvCtrlBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send EfuseReadBootEnvCtrlBits CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	BootEnvCtrlBitsData->SysmonTempEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_EN_SHIFT);
	BootEnvCtrlBitsData->SysmonVoltEn =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_EN_SHIFT);
	BootEnvCtrlBitsData->SysmonTempHot =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_HOT_SHIFT);
	BootEnvCtrlBitsData->SysmonVoltPmc =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PMC_SHIFT);
	BootEnvCtrlBitsData->SysmonVoltPslp =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_PSLP_SHIFT);
	BootEnvCtrlBitsData->SysmonVoltSoc =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_VOLT_SOC_SHIFT);
	BootEnvCtrlBitsData->SysmonTempCold =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_MASK) >>
		XNVM_EFUSE_CACHE_BOOT_ENV_CTRL_SYSMON_TEMP_COLD_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read RomRsvdBits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	RomRsvdBits	Address of the output buffer to store the
 * 				RomRsvdBits eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadRomRsvdBits(XNvm_ClientInstance *InstancePtr, const u64 RomRsvdBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadReg;
	XNvm_EfuseRomRsvdBits *RomRsvdBitsData = (XNvm_EfuseRomRsvdBits *)RomRsvdBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Perform input parameter validation on InstancePtr.
	 *  Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_ROM_RSVD_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 *    Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *    If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadReg, XNVM_WORD_LEN);

	RomRsvdBitsData->PlmUpdate =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_ROM_RSVD_PLM_UPDATE_MASK) >>
		XNVM_EFUSE_CACHE_ROM_RSVD_PLM_UPDATE_SHIFT);

	RomRsvdBitsData->AuthKeysToHash =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_ROM_RSVD_AUTH_KEYS_TO_HASH_MASK) >>
		XNVM_EFUSE_CACHE_ROM_RSVD_AUTH_KEYS_TO_HASH_SHIFT);

	RomRsvdBitsData->IroSwap =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_ROM_RSVD_IRO_SWAP_MASK) >>
		XNVM_EFUSE_CACHE_ROM_RSVD_IRO_SWAP_SHIFT);

	RomRsvdBitsData->RomSwdtUsage =
		(u8)((ReadReg &
		XNVM_EFUSE_CACHE_ROM_RSVD_ROM_SWDT_USAGE_MASK) >>
		XNVM_EFUSE_CACHE_ROM_RSVD_ROM_SWDT_USAGE_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read FIPS info bits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	FipsInfoBits	Address of the output buffer to store the
 * 				RomRsvdBits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadFipsInfoBits(XNvm_ClientInstance *InstancePtr, const u64 FipsInfoBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadDmeFipsReg;
	u32 ReadIpDisable0Reg;
	XNvm_EfuseFipsInfoBits *FipsInfoBitsData = (XNvm_EfuseFipsInfoBits *)FipsInfoBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Perform input parameter validation on InstancePtr.
	 *  Return XST_INVALID_PARAM If input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadDmeFipsReg, XNVM_WORD_LEN);
	Xil_DCacheInvalidateRange((UINTPTR)&ReadIpDisable0Reg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadDmeFipsReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadDmeFipsReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_DME_FIPS_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 *    Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *    If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadDmeFipsReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadIpDisable0Reg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadIpDisable0Reg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_IP_DISABLE_0_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 *    Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 *    If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadIpDisable0Reg, XNVM_WORD_LEN);

	FipsInfoBitsData->FipsMode =
		(u8)((ReadDmeFipsReg &
		XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_MASK) >>
		XNVM_EFUSE_CACHE_DME_FIPS_FIPS_MODE_SHIFT);

	FipsInfoBitsData->FipsVersion =
		(u8)((ReadIpDisable0Reg &
		XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_MASK) >>
		XNVM_EFUSE_CACHE_IP_DISABLE_0_FIPS_VERSION_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PufSecCtrlBits
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PufSecCtrlBits	Address of the output buffer to store the
 * 				PufSecCtrlBits eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPufSecCtrlBits(XNvm_ClientInstance *InstancePtr, const u64 PufSecCtrlBits)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 ReadSecurityCtrlReg;
	u32 ReadEccCtrlReg;
	XNvm_EfusePufSecCtrlBits *PufSecCtrlBitsData = (XNvm_EfusePufSecCtrlBits *)PufSecCtrlBits;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadEccCtrlReg, XNVM_WORD_LEN);
	Xil_DCacheInvalidateRange((UINTPTR)&ReadSecurityCtrlReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadSecurityCtrlReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadSecurityCtrlReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_SECURITY_CONTROL_OFFSET, 1U, LowAddr,
		HighAddr);

	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadSecurityCtrlReg, XNVM_WORD_LEN);

	HighAddr = (u32)((UINTPTR)(&ReadEccCtrlReg) >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)(UINTPTR)&ReadEccCtrlReg;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_PUF_ECC_CTRL_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send EfuseReadPufSecCtrlBits CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);
	if (Status != XST_SUCCESS) {
		XSECURE_STATUS_CHK_GLITCH_DETECT(Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&ReadEccCtrlReg, XNVM_WORD_LEN);

	PufSecCtrlBitsData->PufRegenDis =
		(u8)((ReadEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT);
	PufSecCtrlBitsData->PufHdInvalid =
		(u8)((ReadEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_HD_INVLD_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_HD_INVLD_SHIFT);
	PufSecCtrlBitsData->PufTest2Dis =
		(u8)((ReadSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_TEST2_DIS_SHIFT);
	PufSecCtrlBitsData->PufDis =
		(u8)((ReadSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_DIS_SHIFT);
	PufSecCtrlBitsData->PufSynLk =
		(u8)((ReadSecurityCtrlReg &
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_MASK) >>
		XNVM_EFUSE_CACHE_SECURITY_CONTROL_PUF_SYN_LK_SHIFT);
	PufSecCtrlBitsData->PufRegisDis =
		(u8)((ReadEccCtrlReg &
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_REGEN_DIS_MASK) >>
		XNVM_EFUSE_CACHE_PUF_ECC_PUF_CTRL_PUF_REGEN_DIS_SHIFT);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read OffChip ID eFuses
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	OffChipIdAddr	Address of the output buffer to store the
 * 				OffChip ID eFuse data
 * @param	OffChipIdNum	OffChip ID number to be read out
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadOffchipRevokeId(XNvm_ClientInstance *InstancePtr, const u64 OffChipIdAddr,
	const XNvm_OffchipId OffChipIdNum)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u16 StartOffset = XNVM_EFUSE_CACHE_OFFCHIP_REVOKE_0_OFFSET + ((u16)OffChipIdNum * XNVM_WORD_LEN);
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(OffChipIdAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)OffChipIdAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, StartOffset, 1U, LowAddr, HighAddr);

	/**
	 * @{ Send EfuseReadOffchipRevokeId CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PpkHash
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	PpkHashAddr	Address of the output buffer to store the
 * 				PpkHashAddr eFuses data
 * @param 	PpkHashType	Type of the PpkHash to be read out
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(XNvm_ClientInstance *InstancePtr, const u64 PpkHashAddr, const XNvm_PpkType PpkHashType)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u16 StartOffset = 0U;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	switch(PpkHashType) {
		case XNVM_EFUSE_PPK0:
			StartOffset = XNVM_EFUSE_CACHE_PPK0_HASH_0_OFFSET;
			break;
		case XNVM_EFUSE_PPK1:
			StartOffset = XNVM_EFUSE_CACHE_PPK0_HASH_1_OFFSET;
			break;
		case XNVM_EFUSE_PPK2:
			StartOffset = XNVM_EFUSE_CACHE_PPK0_HASH_2_OFFSET;
			break;
		default:
			Status = (int)XST_INVALID_PARAM;
			break;
	}

	if (Status == XST_INVALID_PARAM) {
		goto END;
	}

	HighAddr = (u32)(PpkHashAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)PpkHashAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, StartOffset, XNVM_EFUSE_PPK_HASH_LEN_IN_WORDS,
		LowAddr, HighAddr);

	/**
	 * @{ Send EfuseReadPpkHash CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DecEfuseOnly eFuses
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DecOnlyAddr	Address of the output buffer to store the
 * 				DecEfuseOnly eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(XNvm_ClientInstance *InstancePtr, const u64 DecOnlyAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(DecOnlyAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)DecOnlyAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_DEC_ONLY_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DME Mode eFuses
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DmeModeAddr	Address of the output buffer to store the
 * 				DecEfuseOnly eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDmeMode(XNvm_ClientInstance *InstancePtr, const u64 DmeModeAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(DmeModeAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)DmeModeAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_DME_FIPS_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read BootModeDisable eFuses
 * 		requested by the user
 *
 * @param	InstancePtr	Pointer to the client instance
 * @param	BootModeDisAddr	Address of the output buffer to store the
 * 				BootModeDisable eFuses data
 *
 * @return
 *		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadBootModeDis(XNvm_ClientInstance *InstancePtr, const u64 BootModeDisAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 *  Validate input parameters.
	 *  Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(BootModeDisAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)BootModeDisAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_BOOT_MODE_DIS_OFFSET, 1U, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DNA eFuses
 * 		requested by the user
 *
 * @param	InstancePtr Pointer to the client instance
 * @param	DnaAddr		Address of the output buffer to store the
 * 				DNA eFuses data
 *
 * @return
 * 		- XST_SUCCESS  If the read is successful
 * 		- ErrorCode  If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(XNvm_ClientInstance *InstancePtr, const u64 DnaAddr)
{
	volatile int Status = XST_FAILURE;
	u32 Payload[XNVM_MAX_PAYLOAD_LEN];
	XNvm_RdCacheCdo* RdCacheCdo =  (XNvm_RdCacheCdo*)Payload;
	u32 HighAddr;
	u32 LowAddr;

	/**
	 * Validate input parameters.
	 * Return XST_INVALID_PARAM, if input parameters are invalid.
	 */
	if ((InstancePtr == NULL) || (InstancePtr->MailboxPtr == NULL)) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

	HighAddr = (u32)(DnaAddr >> XNVM_ADDR_HIGH_SHIFT);
	LowAddr = (u32)DnaAddr;
	XNvm_EfuseCreateReadEfuseCacheCmd(RdCacheCdo, XNVM_EFUSE_CACHE_DNA_OFFSET, XNVM_EFUSE_DNA_LEN_IN_WORDS, LowAddr,
		HighAddr);

	/**
	 * @{ Send an IPI request to the PLM by using the XNVM_API_ID_EFUSE_READ_CACHE CDO command.
	 * Wait for IPI response from PLM  with a default timeout of 300 seconds.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)RdCacheCdo,
			sizeof(XNvm_RdCacheCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function creates payload for Write PUF CDO.
 *
 * @param	PufWrCdo	Pointer to the Write PUF Key CDO
 * @param	AddrLow		Lower Address of the PUF data buffer
 * @param	AddrHigh	Higher Address of the PUF data buffer
 *
 ******************************************************************************/
static void XNvm_EfuseCreateWritePufCmd(XNvm_PufWriteCdo* PufWrCdo, u32 AddrLow, u32 AddrHigh)
{
	PufWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_PUF);
	PufWrCdo->Pload.AddrLow = AddrLow;
	PufWrCdo->Pload.AddrHigh = AddrHigh;
}

/*****************************************************************************/
/**
 * @brief	This function creates payload for Write AES Key CDO.
 *
 * @param	AesKeyWrCdo	Pointer to the Write AES Key CDO
 * @param	KeyType		Type of the Key
 * @param	AddrLow		Lower Address of the key buffer
 * @param	AddrHigh	Higher Address of the key buffer
 * @param       EnvMonDis       Environmental monitor disable flag
 *
 ******************************************************************************/
static void XNvm_EfuseCreateWriteKeyCmd(XNvm_AesKeyWriteCdo* AesKeyWrCdo, XNvm_AesKeyType KeyType, u32 AddrLow,
	u32 AddrHigh, u32 EnvMonDis)
{
	AesKeyWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_AES_KEY);
	AesKeyWrCdo->Pload.KeyType = KeyType;
	AesKeyWrCdo->Pload.AddrLow = AddrLow;
	AesKeyWrCdo->Pload.AddrHigh = AddrHigh;
	AesKeyWrCdo->Pload.EnvDisFlag = (u16)EnvMonDis;
}

/*****************************************************************************/
/**
 * @brief	This function creates payload for Write PPK Hash CDO.
 *
 * @param	PpkWrCdo	Pointer to the Write PPK Hash CDO
 * @param	PpkType		Type of PPK
 * @param	AddrLow		Lower Address of the PPK Hash buffer
 * @param	AddrHigh	Higher Address of the PPK Hash buffer
 * @param       EnvMonDis       Environmental monitor disable flag
 *
 ******************************************************************************/
static void XNvm_EfuseCreateWritePpkCmd(XNvm_PpkWriteCdo* PpkWrCdo, XNvm_PpkType PpkType, u32 AddrLow,
	u32 AddrHigh, u32 EnvMonDis)
{
	PpkWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_PPK_HASH);
	PpkWrCdo->Pload.PpkType = PpkType;
	PpkWrCdo->Pload.AddrLow = AddrLow;
	PpkWrCdo->Pload.AddrHigh = AddrHigh;
	PpkWrCdo->Pload.EnvDisFlag = (u16)EnvMonDis;
}

/*****************************************************************************/
/**
 * @brief	This function creates payload for Write IV CDO.
 *
 * @param	IvWrCdo		Pointer to the Write IV CDO
 * @param	IvType		Type of IV
 * @param	AddrLow		Lower Address of the IV buffer
 * @param	AddrHigh	Higher Address of the IV buffer
 * @param       EnvMonDis       Environmental monitor disable flag
 *
 ******************************************************************************/
static void XNvm_EfuseCreateWriteIvCmd(XNvm_IvWriteCdo* IvWrCdo, XNvm_IvType IvType, u32 AddrLow,
	u32 AddrHigh, u32 EnvMonDis)
{
	IvWrCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_WRITE_IV);
	IvWrCdo->Pload.IvType = (u16)IvType;
	IvWrCdo->Pload.AddrLow = AddrLow;
	IvWrCdo->Pload.AddrHigh = AddrHigh;
	IvWrCdo->Pload.EnvDisFlag = (u16)EnvMonDis;
}

/*****************************************************************************/
/**
 * @brief	This function creates payload for Read Efuse Cache CDO.
 *
 * @param	RdCacheCdo	Pointer to the Read eFUSE Cache CDO
 * @param	StartOffset	Start offset of cache register
 * @param	RegCount	Number of registers to be read
 * @param	AddrLow		Lower Address of the output buffer
 * @param	AddrHigh	Higher Address of the output buffer
 *
 ******************************************************************************/
static void XNvm_EfuseCreateReadEfuseCacheCmd(XNvm_RdCacheCdo* RdCacheCdo, u16 StartOffset, u8 RegCount, u32 AddrLow, u32 AddrHigh)
{
	RdCacheCdo->CdoHdr = Header(0U, (u32)XNVM_API_ID_EFUSE_READ_CACHE);
	RdCacheCdo->Pload.StartOffset = StartOffset;
	RdCacheCdo->Pload.RegCount = RegCount;
	RdCacheCdo->Pload.AddrLow = AddrLow;
	RdCacheCdo->Pload.AddrHigh = AddrHigh;
}

/******************************************************************************/
/**
 * This function Validates the Aes Key and sends IPI request to program
 * Aes Key/User Key0/User Key1 as requested by the user
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	KeyWrCdo	Pointer to the Write AES Key CDO
 * @param	KeyType		Type of the Key
 * @param	Addr		Aes Key address
 * @param	EnvMonDis	Environmental monitor disable flag
 *
 * @return
 * 		- XST_SUCCESS  If the eFUSE programming is successful
 * 		- XST_FAILURE  If there is a failure
 *
 *@section Implementation
 ******************************************************************************/
static int XNvm_EfuseValidateNdWriteAesKey(const XNvm_ClientInstance *InstancePtr,
		XNvm_AesKeyWriteCdo *KeyWrCdo, XNvm_AesKeyType KeyType, u64 Addr, u32 EnvMonDis)
{
	int Status = XST_FAILURE;

	/**
	 * Validate Aes Keys requested for programming.
	 */
	Status = XNvm_EfuseValidateAesKeyWriteReq(KeyType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Create Payload for write Aes Key/User0 Key/User1 Key.
	 */
	XNvm_EfuseCreateWriteKeyCmd(KeyWrCdo, KeyType, (u32)(Addr),
		(u32)((Addr) >> XNVM_ADDR_HIGH_SHIFT), EnvMonDis);

	/**
	 * Send XNvm_EfuseWriteAesKey CDO over IPI request to the PLM.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then return error else return the status of the IPI response.
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)KeyWrCdo,
			sizeof(XNvm_AesKeyWriteCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates the PPK Hash and sends IPI request to program
 * PPK0/PPK1/PPK2 HASH
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	KeyWrCdo	Pointer to the Write AES Key CDO
 * @param	KeyType		Type of the Key
 * @param	Addr		Aes Key address
 * @param       EnvMonDis       Environmental monitor disable flag
 *
 * @return
 * 		- XST_SUCCESS  If the eFUSE programming is successful
 * 		- XST_FAILURE  If there is a failure
 *
 *@section Implementation
 ******************************************************************************/
static int XNvm_EfuseValidatNdWritePpkHash(const XNvm_ClientInstance *InstancePtr,
		XNvm_PpkWriteCdo *PpkWrCdo, XNvm_PpkType PpkType, u64 Addr, u32 EnvMonDis)
{
	int Status = XST_FAILURE;

	/**
	 * Validate PPK Hash's requested for programming
	 */
	Status = XNvm_EfuseValidatePpkHashWriteReq(PpkType);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Create Payload for write PPK0/PPK1/PPK2 Hash
	 */
	XNvm_EfuseCreateWritePpkCmd(PpkWrCdo, PpkType, (u32)(Addr),
			(u32)((Addr) >> XNVM_ADDR_HIGH_SHIFT), EnvMonDis);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XNvm_EfuseWritePpkHash API.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)PpkWrCdo,
			sizeof(XNvm_PpkWriteCdo) / XNVM_WORD_LEN);

END:
	return Status;
}

/******************************************************************************/
/**
 * This function Validates the IV and sends IPI request to program
 * IV's as requested by the user
 *
 * @param	InstancePtr 	Pointer to the client instance
 * @param	KeyWrCdo	Pointer to the Write AES Key CDO
 * @param	KeyType		Type of the Key
 * @param	Addr		Aes Key address
 * @param       EnvMonDis       Environmental monitor disable flag
 *
 * @return
 * 		- XST_SUCCESS  If the eFUSE programming is successful
 * 		- XST_FAILURE  If there is a failure
 *
 *@section Implementation
 ******************************************************************************/
static int XNvm_EfuseValidateNdWriteIv(const XNvm_ClientInstance *InstancePtr,
		XNvm_IvWriteCdo *IvWrCdo, XNvm_IvType IvType, u64 Addr, u32 EnvMonDis)
{
	int Status = XST_FAILURE;

	/**
	 * Validate IV's requested for programming
	 */
	Status = XNvm_EfuseValidateIvWriteReq(IvType, (XNvm_Iv*)(Addr));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * Create Payload for write IV
	 */
	XNvm_EfuseCreateWriteIvCmd(IvWrCdo, IvType, (u32)(Addr),
			(u32)((Addr) >> XNVM_ADDR_HIGH_SHIFT), EnvMonDis);

	/**
	 * Send an IPI request to the PLM by using the CDO command to call XNvm_EfuseWriteIv API.
	 * Wait for IPI response from PLM with a timeout.
	 * If the timeout exceeds then error is returned otherwise it returns the status of the IPI response
	 */
	Status = XNvm_ProcessMailbox(InstancePtr->MailboxPtr, (u32 *)IvWrCdo,
			sizeof(XNvm_IvWriteCdo) / XNVM_WORD_LEN);
END:
	return Status;
}
