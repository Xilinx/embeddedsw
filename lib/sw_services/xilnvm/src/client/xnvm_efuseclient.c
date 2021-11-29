/******************************************************************************
* Copyright (c) 2021 Xilinx, Inc.  All rights reserved.
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
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xil_cache.h"
#include "xnvm_defs.h"
#include "xnvm_efuseclient.h"
#include "xnvm_ipi.h"

/************************** Constant Definitions *****************************/
#define XNVM_WORD_LEN		(4U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program eFuses with
 * 		user provided data
 *
 * @param	DataAddr	Address of the data structure where the eFUSE
 * 				data to be programmed is stored
 *
 * @return	- XST_SUCCESS - If the eFUSE programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWrite(const u64 DataAddr)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE,
			(u32)DataAddr, (u32)(DataAddr >> 32U));
	if (Status != XST_SUCCESS) {
		XNvm_Printf(XNVM_DEBUG_GENERAL, "eFUSE programming Failed \r\n");
	}

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function sends IPI request to program IV eFuses with
 * 		user provided data
 *
 * @param	IvAddr		Address of the data structure where the eFUSE
 * 				data to be programmed is stored
 * @param	EnvDisFlag	Flag that tells weather to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteIVs(const u64 IvAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr EfuseData __attribute__ ((aligned (64U))) = {0U};
	u64 DataAddr;

	EfuseData.EnvMonDisFlag = EnvDisFlag;
	EfuseData.IvAddr = (UINTPTR)IvAddr;
	DataAddr = (u64)(UINTPTR)&EfuseData;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(EfuseData));

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE,
				(u32)DataAddr, (u32)(DataAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program PPK_INVLD eFuse
 * 		requested by the user
 *
 * @param	PpkRevoke	Type of PPK_INVLD to be revoked
 * @param	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return	- XST_SUCCESS - If the programming is successful
 *		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
int XNvm_EfuseRevokePpk(const XNvm_PpkType PpkRevoke, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr EfuseData __attribute__ ((aligned (64U))) = {0U};
	XNvm_EfuseMiscCtrlBits MiscCtrlBits __attribute__ ((aligned (64U))) = {0U};
	u64 DataAddr;

	if (PpkRevoke == XNVM_EFUSE_PPK0) {
		MiscCtrlBits.Ppk0Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK1) {
		MiscCtrlBits.Ppk1Invalid = TRUE;
	}
	else if (PpkRevoke == XNVM_EFUSE_PPK2) {
		MiscCtrlBits.Ppk2Invalid = TRUE;
	}
	else {
		goto END;
	}
	XNvm_DCacheFlushRange((UINTPTR)&MiscCtrlBits, sizeof(MiscCtrlBits));

	EfuseData.EnvMonDisFlag = EnvDisFlag;
	EfuseData.MiscCtrlAddr = (UINTPTR)&MiscCtrlBits;
	DataAddr = (u64)(UINTPTR)&EfuseData;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(EfuseData));

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE,
			(u32)DataAddr, (u32)(DataAddr >> 32U));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program revoke id eFuses
 * 		requested by the user
 *
 * @param	RevokeId	RevokeId number to be revoked
 * @param 	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteRevocationId(const u32 RevokeId, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr EfuseData __attribute__ ((aligned (64U))) = {0U};
	u32 RevokeIdRow;
	u32 RevokeIdBit;
	XNvm_EfuseRevokeIds WriteRevokeId = {0U};
	u64 DataAddr;

	RevokeIdRow = RevokeId >> (XNVM_WORD_LEN + 1U);
	RevokeIdBit = RevokeId & (XNVM_WORD_LEN - 1U);

	if (RevokeIdRow > (XNVM_NUM_OF_REVOKE_ID_FUSES - 1U)) {
		goto END;
	}

	WriteRevokeId.RevokeId[RevokeIdRow] = ((u32)1U << RevokeIdBit);
	WriteRevokeId.PrgmRevokeId = TRUE;

	XNvm_DCacheFlushRange((UINTPTR)&WriteRevokeId, sizeof(WriteRevokeId));

	EfuseData.RevokeIdAddr = (UINTPTR)&WriteRevokeId;
	EfuseData.EnvMonDisFlag = EnvDisFlag;
	DataAddr = (u64)(UINTPTR)&EfuseData;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(EfuseData));
	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE,
                                (u32)DataAddr, (u32)(DataAddr >> 32U));

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program User eFuses
 * 		requested by the user
 *
 * @param	UserFuseAddr	Address of the XNvm_EfuseUserData structure
 * 				where the user provided data to be programmed
 * @param	EnvDisFlag	Flag that tells whether to enable/disable
 *				Environmental monitoring in XilNvm
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWriteUserFuses(const u64 UserFuseAddr, const u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfuseDataAddr EfuseData __attribute__ ((aligned (64U))) = {0U};
	u64 DataAddr;

	EfuseData.EnvMonDisFlag = EnvDisFlag;
	EfuseData.UserFuseAddr = (UINTPTR)UserFuseAddr;
	DataAddr = (u64)(UINTPTR)&EfuseData;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(EfuseData));

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE,
			(u32)DataAddr, (u32)(DataAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read IV eFuses
 * 		requested by the user
 *
 * @param	IvAddr		Address of the output buffer to store the
 * 				IV eFuse data
 * @param	IvType		Type of the IV to read out
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadIv(const u64 IvAddr, const XNvm_IvType IvType)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload3((u32)XNVM_EFUSE_READ_IV,
			(u32)IvType, (u32)IvAddr, (u32)(IvAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Revocation ID eFuses
 * 		requested by the user
 *
 * @param	RevokeIdAddr	Address of the output buffer to store the
 * 				Revocation ID eFuse data
 * @param 	RevokeIdNum	Revocation ID to be read out
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadRevocationId(const u64 RevokeIdAddr,
			const XNvm_RevocationId RevokeIdNum)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload3((u32)XNVM_EFUSE_READ_REVOCATION_ID,
			(u32)RevokeIdNum, (u32)RevokeIdAddr,
			(u32)(RevokeIdAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read User eFuses
 * 		requested by the user
 *
 * @param	UserFuseAddr	Address of the output buffer to store the
 * 				User eFuse data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadUserFuses(const u64 UserFuseAddr)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_READ_USER_FUSES,
			(u32)UserFuseAddr, (u32)(UserFuseAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read MiscCtrlBits
 * 		requested by the user
 *
 * @param	MiscCtrlBits	Address of the output buffer to store the
 * 				MiscCtrlBits eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadMiscCtrlBits(const u64 MiscCtrlBits)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_MISC_CTRL_BITS,
			(u32)MiscCtrlBits, (u32)(MiscCtrlBits >> 32U));
	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecCtrlBits
 * 		requested by the user
 *
 * @param	SecCtrlBits	Address of the output buffer to store the
 * 				SecCtrlBits eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecCtrlBits(const u64 SecCtrlBits)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_SEC_CTRL_BITS,
			(u32)SecCtrlBits, (u32)(SecCtrlBits >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read SecMisc1Bits
 * 		requested by the user
 *
 * @param	SecMisc1Bits	Address of the output buffer to store the
 * 				SecMisc1Bits eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadSecMisc1Bits(const u64 SecMisc1Bits)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_SEC_MISC1_BITS,
			(u32)SecMisc1Bits, (u32)(SecMisc1Bits >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read BootEnvCtrlBits
 * 		requested by the user
 *
 * @param	BootEnvCtrlBits	Address of the output buffer to store the
 * 				BootEnvCtrlBits eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadBootEnvCtrlBits(const u64 BootEnvCtrlBits)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_BOOT_ENV_CTRL_BITS,
			(u32)BootEnvCtrlBits, (u32)(BootEnvCtrlBits >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PufSecCtrlBits
 * 		requested by the user
 *
 * @param	PufSecCtrlBits	Address of the output buffer to store the
 * 				PufSecCtrlBits eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPufSecCtrlBits(const u64 PufSecCtrlBits)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_PUF_SEC_CTRL_BITS,
			(u32)PufSecCtrlBits, (u32)(PufSecCtrlBits >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read OffChip ID eFuses
 * 		requested by the user
 *
 * @param	OffChipIdAddr	Address of the output buffer to store the
 * 				OffChip ID eFuse data
 * @param	OffChipIdNum	OffChip ID number to be read out
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadOffchipRevokeId(const u64 OffChidIdAddr,
	const XNvm_OffchipId OffChipIdNum)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload3(
			(u32)XNVM_EFUSE_READ_OFFCHIP_REVOCATION_ID,
			(u32)OffChipIdNum, (u32)OffChidIdAddr,
			(u32)(OffChidIdAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read PpkHash
 * 		requested by the user
 *
 * @param	PpkHashAddr	Address of the output buffer to store the
 * 				PpkHashAddr eFuses data
 * @param 	PpkHashType	Type of the PpkHash to be read out
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPpkHash(const u64 PpkHashAddr, const XNvm_PpkType PpkHashType)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload3((u32)XNVM_EFUSE_READ_PPK_HASH,
			(u32)PpkHashType, (u32)PpkHashAddr,
			(u32)(PpkHashAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DecEfuseOnly eFuses
 * 		requested by the user
 *
 * @param	DecOnlyAddr	Address of the output buffer to store the
 * 				DecEfuseOnly eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDecOnly(const u64 DecOnlyAddr)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_DEC_EFUSE_ONLY,
			(u32)DecOnlyAddr, (u32)(DecOnlyAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read DNA eFuses
 * 		requested by the user
 *
 * @param	DnaAddr		Address of the output buffer to store the
 * 				DNA eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadDna(const u64 DnaAddr)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_READ_DNA,
			(u32)DnaAddr, (u32)(DnaAddr >> 32U));

	return Status;
}

#ifdef XNVM_ACCESS_PUF_USER_DATA

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf as User eFuses
 * 		requested by the user
 *
 * @param	PufUserFuseAddr	Address of the XNvm_EfusePufFuseAddr structure
 * 				where the user provided data to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePufAsUserFuses(const u64 PufUserFuseAddr)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_PUF_USER_FUSE_WRITE,
			(u32)PufUserFuseAddr, (u32)(PufUserFuseAddr >> 32U));

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Puf User eFuses
 * 		requested by the user
 *
 * @param	PufUserFuseAddr	Address of the output buffer to store the
 * 				Puf User eFuses data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPufAsUserFuses(const u64 PufUserFuseAddr)
{
	int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2(
			(u32)XNVM_EFUSE_READ_PUF_USER_FUSE,
			(u32)PufUserFuseAddr, (u32)(PufUserFuseAddr >> 32U));

	return Status;
}
#else

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to program Puf helper data
 * 		requested by the user
 *
 * @param	PufHdAddr	Address of the XNvm_EfusePufHdAddr structure
 * 				where the user provided helper data to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseWritePuf(const u64 PufHdAddr) {
	volatile int Status = XST_FAILURE;
	u64 DataAddr;

	DataAddr = PufHdAddr;

	XNvm_DCacheFlushRange((UINTPTR)DataAddr, sizeof(XNvm_EfusePufHdAddr));

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_WRITE_PUF,
			(u32)DataAddr, (u32)(DataAddr >> 32U));

	return Status;

}

/*****************************************************************************/
/**
 * @brief	This function sends IPI request to read Puf helper data
 * 		requested by the user
 *
 * @param	PufHdAddr	Address of the output buffer to store the
 * 				Puf helper data
 *
 * @return	- XST_SUCCESS - If the read is successful
 * 		- XST_FAILURE - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseReadPuf(const u64 PufHdAddr) {
	volatile int Status = XST_FAILURE;

	Status = XNvm_ProcessIpiWithPayload2((u32)XNVM_EFUSE_READ_PUF,
			(u32)PufHdAddr, (u32)(PufHdAddr >> 32U));

	return Status;
}
#endif
