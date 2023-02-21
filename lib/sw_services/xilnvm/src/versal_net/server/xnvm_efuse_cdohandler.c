/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file net/server/xnvm_efuse_cdohandler.c
*
* This file contains the Versal_Net XilNvm EFUSE CDO Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 3.0  kal   07/12/2022 Initial release
* 3.1  skg   10/25/2022 Added in body comments for APIs
*
* </pre>
*
* @note
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xplmi_config.h"

#ifdef PLM_NVM
#include "xnvm_efuse.h"
#include "xnvm_efuse_cdohandler.h"
#include "xplmi_dma.h"
#include "xnvm_defs.h"
#include "xnvm_utils.h"
#include "xplmi_hw.h"

/************************** Constant Definitions *****************************/
#define XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK 	(0x0000FFFFU)
#define XNVM_EFUSE_CACHE_BASEADDR		(0xF1250000U)
#define XNVM_EFUSE_CACHE_END_OFFSET		(0x00000BFCU)

/************************** Function Prototypes *****************************/
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 EnvDisFlag, XNvm_AesKeyType KeyType, XNvm_AesKey *Key);
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 EnvDisFlag, XNvm_PpkType PpkType, XNvm_PpkHash *Hash);
static int XNvm_EfuseWriteIvFromCdoPload(u32 EnvDisFlag, XNvm_IvType IvType, XNvm_Iv *Iv);
static int XNvm_EfuseWriteAesKeys(u32 EnvDisFlag, XNvm_AesKeyType KeyType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseWritePpk(u32 EnvDisFlag, XNvm_PpkType PpkType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseWriteIvs(u32 EnvDisFlag, XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseRead(u16 RegCount, u16 StartOffset, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseCacheLoadNPrgmProtBits(void);
static int XNvm_EfuseWriteGlitchConfiguration(u32 EnvDisFlag, u32 GlitchConfig);
static int XNvm_EfuseWriteDecOnlyFuse(u32 EnvDisFlag);
static int XNvm_EfuseWriteRevocationId(u32 EnvDisFlag, u32 RevokeIdNum);
static int XNvm_EfuseWriteOffchipRevocationId(u32 EnvDisFlag, u32 OffChipId);
static int XNvm_EfuseWriteMiscCtrl(u32 EnvDisFlag, u32 MiscCtrlBits);
static int XNvm_EfuseWriteSecCtrl(u32 EnvDisFlag, u32 SecCtrlBits);
static int XNvm_EfuseWriteMisc1Ctrl(u32 EnvDisFlag, u32 Misc1CtrlBits);
static int XNvm_EfuseWriteBootEnvCtrl(u32 EnvDisFlag, u32 BootEnvCtrlBits);
static int XNvm_EfuseWriteFipsInfoFuses(u32 EnvDisFlag, u32 FipsMode, u32 FipsVersion);
static int XNvm_EfuseWriteDiceUds(u32 EnvDisFlag, XNvm_Uds *Uds);
static int XNvm_EfuseWriteDmeKeyFromPload(u32 EnvDisFlag, XNvm_DmeKeyType KeyType, XNvm_DmeKey *Key);
static int XNvm_EfuseWriteDmeRevokeBits(u32 EnvDisFlag, u32 DmeRevokeNum);
static int XNvm_EfuseWritePlmUpdate(u32 EnvDisFlag);
static int XNvm_EfuseWriteBootModeDis(u32 EnvDisFlag, u32 BootModeDisMask);
static int XNvm_EfuseWritePufDataFromPload(XNvm_PufInfoDirectPload *PufData);
static int XNvm_EfuseWritePufData(u32 AddrLow, u32 AddrHigh);
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len);
static int XNvm_EfuseWriteCrcVal(u32 EnvDisFlag, u32 Crc);
static int XNvm_EfuseWriteDmeModeVal(u32 EnvDisFlag, u32 EfuseDmeMode);

/*************************** Function Definitions *****************************/

/*****************************************************************************/
/**
 * @brief	This function copies word aligned or non word aligned data
 * 		from source address to destination address.
 *
 * @param 	SourceAddr 	From where the buffer data is read
 * @param 	DestAddr 	To which the buffer data is copied
 * @param 	Len 		Length of data to be copied in bytes
 *
 * @return	- XST_SUCCESS - If the copy is successful
 * 		- XST_FAILURE - If there is a failure
 *
 *****************************************************************************/
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len)
{
	int Status = XST_FAILURE;

	Status = XPlmi_MemCpy64(DestAddr, SourceAddr, Len);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function calls respective handler based on the API_ID
 *
 * @param 	Cmd is pointer to the command structure
 *
 * @return	- XST_SUCCESS - If the handler execution is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
int XNvm_EfuseCdoHandler(XPlmi_Cmd *Cmd)
{
	volatile int Status = XST_FAILURE;
	XNvm_AesKeyWriteDirectPload *KeyWrDirectPload = NULL;
	XNvm_PpkWriteDirectPload *HashWrDirectPload = NULL;
	XNvm_IvWriteDirectPload *IvWrDirectPload = NULL;
	XNvm_AesKeyWritePload *KeyWrPload = NULL;
	XNvm_PpkWritePload *PpkWrPload = NULL;
	XNvm_IvWritePload *IvWrPload = NULL;
	XNvm_RdCachePload *RdCachePload = NULL;
	XNvm_PufWritePload *WrPufPload = NULL;
	XNvm_GlitchConfig *GlitchConfig = NULL;
	XNvm_DecOnly *DecOnly = NULL;
	XNvm_RevodeId *RevokeId = NULL;
	XNvm_OffChipId *OffChipId = NULL;
	XNvm_MiscCtrlBits *MiscCtrlData = NULL;
	XNvm_SecCtrlBits *SecCtrlData = NULL;
	XNvm_Misc1CtrlBits *Misc1CtrlData = NULL;
	XNvm_BootEnvCtrlBits *BootEnvCtrlData = NULL;
	XNvm_FipsInfo *FipsInfo = NULL;
	XNvm_UdsDirectPload *DiceUds = NULL;
	XNvm_DmeKeyDirectPload *DmeKey = NULL;
	XNvm_DmeRevokeDirectPload *DmeRevoke = NULL;
	XNvm_DisPlmUpdate *DisPlmUpdate = NULL;
	XNvm_BootModeDis *BootModeDisMask = NULL;
	XNvm_Crc *Crc = NULL;
	XNvm_DmeMode *DmeMode = NULL;
	XNvm_PufInfoDirectPload *PufData = NULL;

    /**
	 *  Validate input parameters. Return XST_INVALID_PARAM if input parameters are invalid
	 */
	if (Cmd == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}

    /**
	 *  Calls the respective handler based on API ID. Return error code upon failure
	 */
	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD):
		KeyWrDirectPload = (XNvm_AesKeyWriteDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWriteAesKeyFromCdoPload((u32)KeyWrDirectPload->EnvDisFlag,
			(XNvm_AesKeyType)KeyWrDirectPload->KeyType, &KeyWrDirectPload->EfuseKey);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD):
		HashWrDirectPload = (XNvm_PpkWriteDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWritePpkHashFromCdoPload((u32)HashWrDirectPload->EnvDisFlag,
			(XNvm_PpkType)HashWrDirectPload->PpkType, &HashWrDirectPload->EfuseHash);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD):
		IvWrDirectPload = (XNvm_IvWriteDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWriteIvFromCdoPload((u32)IvWrDirectPload->EnvDisFlag,
			(XNvm_IvType)IvWrDirectPload->IvType, &IvWrDirectPload->EfuseIv);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY):
		KeyWrPload = (XNvm_AesKeyWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWriteAesKeys((u32)KeyWrPload->EnvDisFlag,
				(XNvm_AesKeyType)KeyWrPload->KeyType, KeyWrPload->AddrLow,
				KeyWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH):
		PpkWrPload = (XNvm_PpkWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWritePpk((u32)PpkWrPload->EnvDisFlag,
				(XNvm_PpkType)PpkWrPload->PpkType, PpkWrPload->AddrLow,
				PpkWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV):
		IvWrPload = (XNvm_IvWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWriteIvs((u32)IvWrPload->EnvDisFlag,
				(XNvm_IvType)IvWrPload->IvType, IvWrPload->AddrLow,
				IvWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_CACHE):
		RdCachePload = (XNvm_RdCachePload *)Cmd->Payload;
		Status = XNvm_EfuseRead(RdCachePload->RegCount, RdCachePload->StartOffset, RdCachePload->AddrLow,
			RdCachePload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF):
		WrPufPload = (XNvm_PufWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWritePufData(WrPufPload->AddrLow, WrPufPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS):
		Status =  XNvm_EfuseCacheLoadNPrgmProtBits();
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG):
		GlitchConfig = (XNvm_GlitchConfig *)Cmd->Payload;
		Status = XNvm_EfuseWriteGlitchConfiguration((u32)GlitchConfig->EnvDisFlag, GlitchConfig->GlitchConfigVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DEC_ONLY):
		DecOnly = (XNvm_DecOnly *)Cmd->Payload;
		Status = XNvm_EfuseWriteDecOnlyFuse((u32)DecOnly->EnvDisFlag);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID):
		RevokeId = (XNvm_RevodeId *)Cmd->Payload;
		Status = XNvm_EfuseWriteRevocationId((u32)RevokeId->EnvDisFlag, RevokeId->RevokeIdNum);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID):
		OffChipId = (XNvm_OffChipId *)Cmd->Payload;
		Status = XNvm_EfuseWriteOffchipRevocationId((u32)OffChipId->EnvDisFlag, OffChipId->OffChipIdNum);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS):
		MiscCtrlData = (XNvm_MiscCtrlBits *)Cmd->Payload;
		Status = XNvm_EfuseWriteMiscCtrl((u32)MiscCtrlData->EnvDisFlag, MiscCtrlData->MiscCtrlBitsVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS):
		SecCtrlData = (XNvm_SecCtrlBits *)Cmd->Payload;
		Status = XNvm_EfuseWriteSecCtrl((u32)SecCtrlData->EnvDisFlag, SecCtrlData->SecCtrlBitsVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS):
		Misc1CtrlData = (XNvm_Misc1CtrlBits *)Cmd->Payload;
		Status = XNvm_EfuseWriteMisc1Ctrl((u32)Misc1CtrlData->EnvDisFlag, Misc1CtrlData->Misc1CtrlBitsVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS):
		BootEnvCtrlData = (XNvm_BootEnvCtrlBits *)Cmd->Payload;
		Status = XNvm_EfuseWriteBootEnvCtrl((u32)BootEnvCtrlData->EnvDisFlag, BootEnvCtrlData->BootEnvCtrlVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_FIPS_INFO):
		FipsInfo = (XNvm_FipsInfo *)Cmd->Payload;
		Status = XNvm_EfuseWriteFipsInfoFuses((u32)FipsInfo->EnvDisFlag, FipsInfo->FipsMode, FipsInfo->FipsVersion);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_UDS_FROM_PLOAD):
		DiceUds = (XNvm_UdsDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWriteDiceUds((u32)DiceUds->EnvDisFlag, &DiceUds->EfuseUds);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD):
		DmeKey = (XNvm_DmeKeyDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWriteDmeKeyFromPload((u32)DmeKey->EnvDisFlag, (XNvm_DmeKeyType)DmeKey->KeyType, &DmeKey->EfuseDmeKey);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_REVOKE):
		DmeRevoke = (XNvm_DmeRevokeDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWriteDmeRevokeBits(DmeRevoke->EnvDisFlag, DmeRevoke->DmeRevokeNum);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE):
		DisPlmUpdate = (XNvm_DisPlmUpdate *)Cmd->Payload;
		Status = XNvm_EfuseWritePlmUpdate((u32)DisPlmUpdate->EnvDisFlag);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE):
		BootModeDisMask = (XNvm_BootModeDis *)Cmd->Payload;
		Status = XNvm_EfuseWriteBootModeDis((u32)BootModeDisMask->EnvDisFlag, (u32)BootModeDisMask->BootModeDisVal);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_CRC):
		Crc = (XNvm_Crc *)Cmd->Payload;
		Status = XNvm_EfuseWriteCrcVal((u32)Crc->EnvDisFlag, Crc->EfuseCrc);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_MODE):
		DmeMode = (XNvm_DmeMode *)Cmd->Payload;
		Status = XNvm_EfuseWriteDmeModeVal((u32)DmeMode->EnvDisFlag, DmeMode->EfuseDmeMode);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_FROM_PLOAD):
		PufData = (XNvm_PufInfoDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWritePufDataFromPload(PufData);
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse Aes key from the CDO
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	KeyType	- Type of the AesKey
 * @param	Key - Pointer to the Aes key
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 EnvDisFlag, XNvm_AesKeyType KeyType, XNvm_AesKey *Key)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteAesKey(EnvDisFlag, KeyType, Key);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse PpkHash from the CDO
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	PpkType	- Type of the PpkHash
 * @param	Hash - Pointer to the PpkHash
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 EnvDisFlag, XNvm_PpkType PpkType, XNvm_PpkHash *Hash)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWritePpkHash(EnvDisFlag, PpkType, Hash);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse IV from the CDO
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	IvType	- Type of the Iv
 * @param	Iv - Pointer to the Iv
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteIvFromCdoPload(u32 EnvDisFlag, XNvm_IvType IvType, XNvm_Iv *Iv)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteIv(EnvDisFlag, IvType, Iv);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs keys received via IPI into eFUSEs.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param	KeyType		Type of the Key
 * @param	AddrLow		Lower Address of the key buffer
 * @param	AddrHigh	Higher Address of the key buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteAesKeys(u32 EnvDisFlag, XNvm_AesKeyType KeyType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_AesKey AesKeys __attribute__ ((aligned (32U))) = {0U};
	u64 AesKeyAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(AesKeyAddr, (u64)(UINTPTR)&AesKeys, sizeof(AesKeys));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWriteAesKey(EnvDisFlag, KeyType, &AesKeys);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Glitch Configuration eFuses with the
 * 		data sent through CDO
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	GlitchConfig - Glitch configuration to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteGlitchConfiguration(u32 EnvDisFlag, u32 GlitchConfig)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteGlitchConfigBits(EnvDisFlag, GlitchConfig);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DecOnly eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDecOnlyFuse(u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDecOnly(EnvDisFlag);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Revocation id eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	RevokeIdNum - Revoke Id eFuse number to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteRevocationId(u32 EnvDisFlag, u32 RevokeIdNum)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteRevocationID(EnvDisFlag, RevokeIdNum);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Offchip revocation id eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	OffChipId - Offchip eFuse number to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteOffchipRevocationId(u32 EnvDisFlag, u32 OffChipId)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteOffChipRevokeID(EnvDisFlag, OffChipId);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs PPK Hash received via IPI into eFUSEs.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param	PpkType		Type of PPK
 * @param	AddrLow		Lower Address of the PPK Hash buffer
 * @param	AddrHigh	Higher Address of the PPK Hash buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePpk(u32 EnvDisFlag, XNvm_PpkType PpkType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_PpkHash PpkHash __attribute__ ((aligned (32U))) = {0U};
	u64 PpkHashAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(PpkHashAddr, (u64)(UINTPTR)&PpkHash, sizeof(PpkHash));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePpkHash(EnvDisFlag, PpkType, &PpkHash);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs MiscCtrl eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	MiscCtrlBits - MiscCtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteMiscCtrl(u32 EnvDisFlag, u32 MiscCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteMiscCtrlBits(EnvDisFlag, MiscCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs SecCtrlBits eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	SecCtrlBits - SecCtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteSecCtrl(u32 EnvDisFlag, u32 SecCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteSecCtrlBits(EnvDisFlag, SecCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs IV received via IPI into eFUSEs.
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param	IvType		Type of IV
 * @param	AddrLow		Lower Address of the IV buffer
 * @param	AddrHigh	Higher Address of the IV buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteIvs(u32 EnvDisFlag, XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_Iv Iv __attribute__ ((aligned (32U))) = {0U};
	u64 IvAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(IvAddr, (u64)(UINTPTR)&Iv, sizeof(Iv));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWriteIv(EnvDisFlag, IvType, &Iv);

END:
	return Status;
}

/*****************************************************************************/
/**
 ** @brief       This function programs Misc1Ctrl eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	Misc1CtrlBits - Misc1CtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteMisc1Ctrl(u32 EnvDisFlag, u32 Misc1CtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteMisc1Bits(EnvDisFlag, Misc1CtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BootEnvCtrl eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	BootEnvCtrl - BootEnvCtrl input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteBootEnvCtrl(u32 EnvDisFlag, u32 BootEnvCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteBootEnvCtrlBits(EnvDisFlag, BootEnvCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Fips mode and Fips version eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	FipsInfo - FipsMode and FipsVersion input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteFipsInfoFuses(u32 EnvDisFlag, u32 FipsMode, u32 FipsVersion)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteFipsInfo(EnvDisFlag, FipsMode, FipsVersion);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DICE UDS eFuses
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param	Uds - Pointer to the Uds
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDiceUds(u32 EnvDisFlag, XNvm_Uds *Uds)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteUds(EnvDisFlag, Uds);

	return Status;
}

/*****************************************************************************/
/**
 * @brief	This function reads eFUSE cache registers from the required offset
 *
 * @param	StartOffset	Start offset of cache register
 * @param	RegCount	Number of registers to be read
 * @param	AddrLow		Lower Address of the output buffer
 * @param	AddrHigh	Higher Address of the output buffer
 *
 * @return	- XST_SUCCESS - If reading eFUSEs is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
 static int XNvm_EfuseRead(u16 RegCount, u16 StartOffset, u32 AddrLow, u32 AddrHigh)
 {
	volatile int Status = XST_FAILURE;
	u64 OutputBuffer = ((u64)AddrHigh << 32U) | (u64)AddrLow;
	u32 RegData;
	u32 EndOffset = StartOffset + RegCount * XNVM_WORD_LEN;
	u32 Offset = StartOffset;

	if (StartOffset > XNVM_EFUSE_CACHE_END_OFFSET || EndOffset > XNVM_EFUSE_CACHE_END_OFFSET){
		Status = XST_INVALID_PARAM;
		goto END;
	}

	while(Offset < EndOffset){
		RegData = XNvm_EfuseReadReg(XNVM_EFUSE_CACHE_BASEADDR, Offset);

		XPlmi_Out64(OutputBuffer, RegData);

		OutputBuffer = OutputBuffer + XNVM_WORD_LEN;
		Offset = Offset + XNVM_WORD_LEN;
	}

	Status = XST_SUCCESS;

END:
	return Status;
 }

/*****************************************************************************/
/**
 * @brief	This function reloads cache and programs the protection bits
 *
 * @return	- XST_SUCCESS - In case of success
 *  		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
 static int XNvm_EfuseCacheLoadNPrgmProtBits(void)
 {
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseCacheLoadNPrgmProtectionBits();

	return Status;
 }

/*****************************************************************************/
/**
 * @brief       This function programs DmeKey eFuses
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param	KeyType - Type of DME user key
 * @param 	Key - Pointer to the DME key
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDmeKeyFromPload(u32 EnvDisFlag, XNvm_DmeKeyType KeyType, XNvm_DmeKey *Key)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDmeUserKey(EnvDisFlag, KeyType,(XNvm_DmeKey *)Key);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DmeRevoke eFuses
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	DmeRevokeNum - Dme revoke number to programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDmeRevokeBits(u32 EnvDisFlag, u32 DmeRevokeNum)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDmeRevoke(EnvDisFlag, DmeRevokeNum);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs PLM_UPDATE eFuses
 *
 * @param 	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePlmUpdate(u32 EnvDisFlag)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDisableInplacePlmUpdate(EnvDisFlag);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BootModeDisable eFuses
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	BootModeDisMask - BootMode disable mask to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteBootModeDis(u32 EnvDisFlag, u32 BootModeDisMask)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteBootModeDisable(EnvDisFlag, BootModeDisMask);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs CRC eFuses
 *
 * @param 	EnvDisFlag 	Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * 		Crc		Crc to be programmed into eFuses
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteCrcVal(u32 EnvDisFlag, u32 Crc)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteCrc(EnvDisFlag, Crc);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DmeMode eFuses
 *
 * @param 	EnvDisFlag 	Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * 		DmeMode		DmeMode to be programmed into eFuses
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDmeModeVal(u32 EnvDisFlag, u32 EfuseDmeMode)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDmeMode(EnvDisFlag, EfuseDmeMode);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Puf helper data, Chash, Aux, Puf Ctrl
 * 		bits. RoSwap eFuses
 *
 * @param	PufData - Pointer to the XNvm_PufInfoDirectPload structure
 *
 * @return	- XST_SUCCESS - If the write is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufDataFromPload(XNvm_PufInfoDirectPload *PufData)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePufHdAddr EfusePufData = {0U};
	u32 Index = 0U;
	EfusePufData.PrgmPufHelperData = 1U;
	EfusePufData.EnvMonitorDis = (u32)PufData->EnvDisFlag;
	EfusePufData.PufSecCtrlBits = PufData->PufCtrlBits;
	EfusePufData.Chash = PufData->Chash;
	EfusePufData.Aux = PufData->Aux;
	EfusePufData.RoSwap = PufData->RoSwap;

	for (Index = 0U; Index < XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS; Index++) {
		EfusePufData.EfuseSynData[Index] = PufData->SynData[Index];
	}

	Status = XNvm_EfuseWritePuf(&EfusePufData);

	return Status;
}

/*****************************************************************************/
/**

 * @brief       This function programs PUF helper data and PUF control bits received
 * 		via IPI into eFUSEs.
 *
 * @param	AddrLow		Lower Address of the IV buffer
 * @param	AddrHigh	Higher Address of the IV buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 ******************************************************************************/
static int XNvm_EfuseWritePufData(u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePufHdAddr PufData __attribute__ ((aligned (32U))) = {0U};
	u64 PufDataAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(PufDataAddr, (u64)(UINTPTR)&PufData, sizeof(PufData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePuf(&PufData);

END:
	return Status;
}
#endif
