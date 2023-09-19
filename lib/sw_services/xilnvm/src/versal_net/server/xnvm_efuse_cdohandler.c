/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
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
* 3.2  har   02/21/2023 Added support for writing ROM Rsvd bits
*      kum   05/03/2023 Added support to handle cdo chunk boundary before efuse writing
*      vek   05/31/2023 Added support for Programming PUF secure control bits
*      kpt   07/26/2023 Clear AES keys
*      yog   09/13/2023 Removed XNvm_EfuseMemCopy() API
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
static int XNvm_EfuseWritePufDataFromPload(XNvm_PufHDInfoDirectPload *PufData);
static int XNvm_EfuseWritePufCtrlBitsFromPload(XNvm_PufCtrlDirectPload *PufSecCtrlBits);
static int XNvm_EfuseWritePufData(u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseWriteCrcVal(u32 EnvDisFlag, u32 Crc);
static int XNvm_EfuseWriteDmeModeVal(u32 EnvDisFlag, u32 EfuseDmeMode);
static int XNvm_EfuseWriteRomRsvd(u32 EnvDisFlag, u32 RomRsvdBits);

/*************************** Function Definitions *****************************/

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
	int ClrStatus = XST_FAILURE;
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
	XNvm_RomRsvdBitsWritePload *RomRsvd = NULL;
	XNvm_PufHDInfoDirectPload *PufData = NULL;
	XNvm_PufCtrlDirectPload *PufSecData = NULL;
	static XNvm_CdoChunk CdoChunkData;
	u32 PloadLen = 0;
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
		PloadLen = Cmd->PayloadLen * XNVM_WORD_LEN;
		if (Cmd->ProcessedLen == 0U){
			Status = Xil_SMemSet(&CdoChunkData.Keys.AesKey[0U], XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN, 0U, XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			CdoChunkData.MemClear = FALSE;
		}
		Status = Xil_SMemCpy((u32 *)&CdoChunkData.Keys.AesKey[Cmd->ProcessedLen],
				(XNVM_AES_KEY_CDO_PAYLOAD_LEN_IN_WORDS - Cmd->ProcessedLen) * XNVM_WORD_LEN, (u32 *)&Cmd->Payload[0U],
				PloadLen, PloadLen);
		if (Status != XST_SUCCESS) {
			CdoChunkData.MemClear = TRUE;
			goto END;
		}
		if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
			KeyWrDirectPload = (XNvm_AesKeyWriteDirectPload *)&CdoChunkData.Keys.AesKey;
			Status = XNvm_EfuseWriteAesKeyFromCdoPload((u32)KeyWrDirectPload->EnvDisFlag,
					(XNvm_AesKeyType)KeyWrDirectPload->KeyType, &KeyWrDirectPload->EfuseKey);
			CdoChunkData.MemClear = TRUE;
		}
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD):
		PloadLen = Cmd->PayloadLen * XNVM_WORD_LEN;
		if (Cmd->ProcessedLen == 0U){
			Status = Xil_SMemSet(&CdoChunkData.Keys.PpkHash[0U], XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN, 0U, XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			CdoChunkData.MemClear = FALSE;
		}
		Status = Xil_SMemCpy((u32 *)&CdoChunkData.Keys.PpkHash[Cmd->ProcessedLen],
				(XNVM_PPK_HASH_CDO_PAYLOAD_LEN_IN_WORDS - Cmd->ProcessedLen) * XNVM_WORD_LEN, (u32 *)&Cmd->Payload[0U],
				PloadLen, PloadLen);
		if (Status != XST_SUCCESS) {
			CdoChunkData.MemClear = TRUE;
			goto END;
		}
		if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
			HashWrDirectPload = (XNvm_PpkWriteDirectPload *)&CdoChunkData.Keys.PpkHash;
			Status = XNvm_EfuseWritePpkHashFromCdoPload((u32)HashWrDirectPload->EnvDisFlag,
							(XNvm_PpkType)HashWrDirectPload->PpkType, &HashWrDirectPload->EfuseHash);
			CdoChunkData.MemClear = TRUE;
		}
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
		PloadLen = Cmd->PayloadLen * XNVM_WORD_LEN;
		if (Cmd->ProcessedLen == 0U){
			Status = Xil_SMemSet(&CdoChunkData.Keys.UdsKey[0U], XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN, 0U, XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			CdoChunkData.MemClear = FALSE;
		}
		Status = Xil_SMemCpy((u32 *)&CdoChunkData.Keys.UdsKey[Cmd->ProcessedLen],
				(XNVM_UDS_CDO_PAYLOAD_LEN_IN_WORDS - Cmd->ProcessedLen) * XNVM_WORD_LEN, (u32 *)&Cmd->Payload[0U],
				PloadLen, PloadLen);
		if (Status != XST_SUCCESS) {
			CdoChunkData.MemClear = TRUE;
			goto END;
		}
		if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
			DiceUds = (XNvm_UdsDirectPload *)&CdoChunkData.Keys.UdsKey;
			Status = XNvm_EfuseWriteDiceUds((u32)DiceUds->EnvDisFlag, &DiceUds->EfuseUds);
			CdoChunkData.MemClear = TRUE;
		}
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD):
		PloadLen = Cmd->PayloadLen * XNVM_WORD_LEN;
		if (Cmd->ProcessedLen == 0U){
			Status = Xil_SMemSet(&CdoChunkData.Keys.DmeKey[0U], XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN, 0U, XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			CdoChunkData.MemClear = FALSE;
		}
		Status = Xil_SMemCpy((u32 *)&CdoChunkData.Keys.DmeKey[Cmd->ProcessedLen],
				(XNVM_DME_KEY_CDO_PAYLOAD_LEN_IN_WORDS - Cmd->ProcessedLen) * XNVM_WORD_LEN, (u32 *)&Cmd->Payload[0U],
				PloadLen, PloadLen);
		if (Status != XST_SUCCESS) {
			CdoChunkData.MemClear = TRUE;
			goto END;
		}
		if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
			DmeKey = (XNvm_DmeKeyDirectPload *)&CdoChunkData.Keys.DmeKey;
			Status = XNvm_EfuseWriteDmeKeyFromPload((u32)DmeKey->EnvDisFlag, (XNvm_DmeKeyType)DmeKey->KeyType, &DmeKey->EfuseDmeKey);
			CdoChunkData.MemClear = TRUE;
		}
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
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_HD_FROM_PLOAD):
		PloadLen = Cmd->PayloadLen * XNVM_WORD_LEN;
		if (Cmd->ProcessedLen == 0U){
			Status = Xil_SMemSet(&CdoChunkData.Keys.PufCfg[0U], XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN, 0U, XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS * XNVM_WORD_LEN);
			if (Status != XST_SUCCESS) {
				goto END;
			}
			CdoChunkData.MemClear = FALSE;
		}
		Status = Xil_SMemCpy((u32 *)&CdoChunkData.Keys.PufCfg[Cmd->ProcessedLen],
				(XNVM_PUF_CFG_CDO_PAYLOAD_LEN_IN_WORDS - Cmd->ProcessedLen) * XNVM_WORD_LEN, (u32 *)&Cmd->Payload[0U],
				PloadLen, PloadLen);
		if (Status != XST_SUCCESS) {
			CdoChunkData.MemClear = TRUE;
			goto END;
		}
		if ((Cmd->ProcessedLen + Cmd->PayloadLen) == Cmd->Len) {
			PufData = (XNvm_PufHDInfoDirectPload *)&CdoChunkData.Keys.PufCfg;
			Status = XNvm_EfuseWritePufDataFromPload(PufData);
			CdoChunkData.MemClear = TRUE;
		}
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_ROM_RSVD):
		RomRsvd = (XNvm_RomRsvdBitsWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWriteRomRsvd(RomRsvd->EnvMonitorDis, RomRsvd->RomRsvdBits);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_CTRL_BITS):
		PufSecData = (XNvm_PufCtrlDirectPload *)Cmd->Payload;
		Status = XNvm_EfuseWritePufCtrlBitsFromPload(PufSecData);
		break;
	default:
		XNvm_Printf(XNVM_DEBUG_GENERAL, "CMD: INVALID PARAM\r\n");
		Status = XST_INVALID_PARAM;
		break;
	}

END:
	if(CdoChunkData.MemClear == TRUE){
		/* Zeroize the keys */
		ClrStatus = Xil_SMemSet(&CdoChunkData, sizeof(XNvm_CdoChunk), 0U, sizeof(XNvm_CdoChunk));
		if ((Status != XST_SUCCESS) || (ClrStatus != XST_SUCCESS)) {
			Status |= ClrStatus;
		}
	}
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
	volatile int ClearStatus = XST_FAILURE;
	volatile int ClearStatusTmp = XST_FAILURE;
	XNvm_AesKey AesKeys __attribute__ ((aligned (32U))) = {0U};
	u64 AesKeyAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&AesKeys, AesKeyAddr, sizeof(AesKeys));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWriteAesKey(EnvDisFlag, KeyType, &AesKeys);

END:
	ClearStatus = XNvm_ZeroizeAndVerify((u8*)&AesKeys, sizeof(AesKeys));
	ClearStatusTmp = XNvm_ZeroizeAndVerify((u8*)&AesKeys, sizeof(AesKeys));
	if (Status == XST_SUCCESS) {
		Status |= (ClearStatus | ClearStatusTmp);
	}

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

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&PpkHash, PpkHashAddr, sizeof(PpkHash));
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

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&Iv, IvAddr, sizeof(Iv));
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
static int XNvm_EfuseWritePufDataFromPload(XNvm_PufHDInfoDirectPload *PufData)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePufHdAddr EfusePufData = {0U};
	u32 Index = 0U;
	EfusePufData.PrgmPufHelperData = 1U;
	EfusePufData.EnvMonitorDis = (u32)PufData->EnvDisFlag;
	EfusePufData.Chash = PufData->Chash;
	EfusePufData.Aux = PufData->Aux;
	EfusePufData.RoSwap = PufData->RoSwap;

	for (Index = 0U; Index < XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS; Index++) {
		EfusePufData.EfuseSynData[Index] = PufData->SynData[Index];
	}

	Status = XNvm_EfuseWritePuf(&EfusePufData);

	return Status;
}

static int XNvm_EfuseWritePufCtrlBitsFromPload(XNvm_PufCtrlDirectPload *PufSecCtrlBits)
{
	volatile int Status = XST_FAILURE;

	/* Programming Puf SecCtrl bits */
    Status = XNvm_EfuseWritePufSecCtrl(PufSecCtrlBits->EnvDisFlag,PufSecCtrlBits->PufCtrlBits);

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

	Status = XPlmi_MemCpy64((u64)(UINTPTR)&PufData, PufDataAddr, sizeof(PufData));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePuf(&PufData);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs ROM Rsvd eFuses
 *
 * @param	EnvDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	RomRsvdBits - ROM Rsvd bits to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteRomRsvd(u32 EnvDisFlag, u32 RomRsvdBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteRomRsvdBits(EnvDisFlag, RomRsvdBits);

	return Status;
}

#endif
