/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xnvm_efuse_cdohandler.c
* @addtogroup xnvm_apis XilNvm Versal_Net eFuse APIs
* @{
* @cond xnvm_internal
* This file contains the Versal_Net XilNvm EFUSE CDO Handler definition.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal   12/07/2022 Initial release
*
* </pre>
*
* @note
* @endcond
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
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 *Pload);
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 *Pload);
static int XNvm_EfuseWriteIvFromCdoPload(u32 *Pload);
static int XNvm_EfuseWriteAesKeys(XNvm_AesKeyType KeyType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseWritePpk(XNvm_PpkType PpkType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseWriteIvs(XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseRead(u16 RegCount, u16 StartOffset, u32 AddrLow, u32 AddrHigh);
static int XNvm_EfuseCacheLoadNPrgmProtBits(void);
static int XNvm_EfuseWriteGlitchConfiguration(u32 EnvMonDisFlag, u32 GlitchConfig);
static int XNvm_EfuseWriteDecOnlyFuse(u32 EnvMonDisFlag);
static int XNvm_EfuseWriteRevocationId(u32 EnvMonDisFlag, u32 RevokeIdNum);
static int XNvm_EfuseWriteOffchipRevocationId(u32 EnvMonDisFlag, u32 OffChipId);
static int XNvm_EfuseWriteMiscCtrl(u32 EnvMonDisFlag, u32 MiscCtrlBits);
static int XNvm_EfuseWriteSecCtrl(u32 EnvMonDisFlag, u32 SecCtrlBits);
static int XNvm_EfuseWriteMisc1Ctrl(u32 EnvMonDisFlag, u32 Misc1CtrlBits);
static int XNvm_EfuseWriteBootEnvCtrl(u32 EnvMonDisFlag, u32 BootEnvCtrlBits);
static int XNvm_EfuseWriteFipsInfoFuses(u32 EnvMonDisFlag, u32 FipsInfo);
static int XNvm_EfuseWriteDiceUds(u32 *Pload);
static int XNvm_EfuseWriteDmeKeyFromPload(u32 *Pload);
static int XNvm_EfuseWriteDmeRevokeBits(u32 Pload);
static int XNvm_EfuseWritePlmUpdate(u32 EnvMonDisFlag);
static int XNvm_EfuseWriteBootModeDis(u32 Pload);
static int XNvm_EfuseWritePufDataFromCdoPload(u32 *Pload);
static int XNvm_EfuseWritePufData(u32 AddrLow, u32 AddrHigh);
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len);

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
	u32 *Pload = Cmd->Payload;

	if (Pload == NULL) {
		Status = XST_INVALID_PARAM;
		goto END;
	}
	switch (Cmd->CmdId & XNVM_API_ID_MASK) {
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY_FROM_PLOAD):
		Status = XNvm_EfuseWriteAesKeyFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH_FROM_PLOAD):
		Status = XNvm_EfuseWritePpkHashFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV_FROM_PLOAD):
		Status = XNvm_EfuseWriteIvFromCdoPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_AES_KEY):
		XNvm_AesKeyWritePload *KeyWrPload = (XNvm_AesKeyWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWriteAesKeys(KeyWrPload->KeyType, KeyWrPload->AddrLow, KeyWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PPK_HASH):
		XNvm_PpkWritePload *PpkWrPload = (XNvm_PpkWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWritePpk(PpkWrPload->PpkType, PpkWrPload->AddrLow, PpkWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_IV):
		XNvm_IvWritePload *IvWrPload = (XNvm_IvWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWriteIvs(IvWrPload->IvType, IvWrPload->AddrLow, IvWrPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_READ_CACHE):
		XNvm_RdCachePload *RdCachePload = (XNvm_RdCachePload *)Cmd->Payload;
		Status = XNvm_EfuseRead(RdCachePload->RegCount, RdCachePload->StartOffset, RdCachePload->AddrLow,
			RdCachePload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF):
		XNvm_PufWritePload *WrPufPload = (XNvm_PufWritePload *)Cmd->Payload;
		Status = XNvm_EfuseWritePufData(WrPufPload->AddrLow, WrPufPload->AddrHigh);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS):
		Status =  XNvm_EfuseCacheLoadNPrgmProtBits();
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_GLITCH_CONFIG):
		Status = XNvm_EfuseWriteGlitchConfiguration(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DEC_ONLY):
		Status = XNvm_EfuseWriteDecOnlyFuse(Pload[0U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_REVOCATION_ID):
		Status = XNvm_EfuseWriteRevocationId(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_OFFCHIP_REVOKE_ID):
		Status = XNvm_EfuseWriteOffchipRevocationId(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC_CTRL_BITS):
		Status = XNvm_EfuseWriteMiscCtrl(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_SEC_CTRL_BITS):
		Status = XNvm_EfuseWriteSecCtrl(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_MISC1_CTRL_BITS):
		Status = XNvm_EfuseWriteMisc1Ctrl(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_ENV_CTRL_BITS):
		Status = XNvm_EfuseWriteBootEnvCtrl(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_FIPS_INFO):
		Status = XNvm_EfuseWriteFipsInfoFuses(Pload[0U], Pload[1U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_UDS_FROM_PLOAD):
		Status = XNvm_EfuseWriteDiceUds(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_KEY_FROM_PLOAD):
		Status = XNvm_EfuseWriteDmeKeyFromPload(Pload);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_DME_REVOKE):
		Status = XNvm_EfuseWriteDmeRevokeBits(Pload[0U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PLM_UPDATE):
		Status = XNvm_EfuseWritePlmUpdate(Pload[0U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_BOOT_MODE_DISABLE):
		Status = XNvm_EfuseWriteBootModeDis(Pload[0U]);
		break;
	case XNVM_API(XNVM_API_ID_EFUSE_WRITE_PUF_FROM_PLOAD):
		Status = XNvm_EfuseWritePufDataFromCdoPload(Pload);
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
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteAesKeyFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_AesKeyType KeyType = (XNvm_AesKeyType)(Pload[0U] &
					XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK);

	Status = XNvm_EfuseWriteAesKey(KeyType, (XNvm_AesKey *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse PpkHash from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePpkHashFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_PpkType HashType = (XNvm_PpkType)(Pload[0U] &
					XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK);

	Status = XNvm_EfuseWritePpkHash(HashType, (XNvm_PpkHash *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Efuse IV from the CDO
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteIvFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_IvType IvType = (XNvm_IvType)Pload[0U];

	Status = XNvm_EfuseWriteIv(IvType, (XNvm_Iv *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs keys received via IPI into eFUSEs.
 *
 * @param	KeyType		Type of the Key
 * @param	AddrLow		Lower Address of the key buffer
 * @param	AddrHigh	Higher Address of the key buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteAesKeys(XNvm_AesKeyType KeyType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_AesKey AesKeys __attribute__ ((aligned (32U))) = {0U};
	u64 AesKeyAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(AesKeyAddr, (u64)(UINTPTR)&AesKeys, sizeof(AesKeys));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWriteAesKey(KeyType, &AesKeys);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Glitch Configuration eFuses with the
 * 		data sent through CDO
 *
 * @param	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	GlitchConfig - Glitch configuration to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteGlitchConfiguration(u32 EnvMonDisFlag, u32 GlitchConfig)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteGlitchConfigBits(EnvMonDisFlag, GlitchConfig);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DecOnly eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDecOnlyFuse(u32 EnvMonDisFlag)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDecOnly(EnvMonDisFlag);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Revocation id eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	RevokeIdNum - Revoke Id eFuse number to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteRevocationId(u32 EnvMonDisFlag, u32 RevokeIdNum)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteRevocationID(EnvMonDisFlag, RevokeIdNum);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Offchip revocation id eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	OffChipId - Offchip eFuse number to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteOffchipRevocationId(u32 EnvMonDisFlag, u32 OffChipId)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteOffChipRevokeID(EnvMonDisFlag, OffChipId);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs PPK Hash received via IPI into eFUSEs.
 *
 * @param	PpkType		Type of PPK
 * @param	AddrLow		Lower Address of the PPK Hash buffer
 * @param	AddrHigh	Higher Address of the PPK Hash buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePpk(XNvm_PpkType PpkType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_PpkHash PpkHash __attribute__ ((aligned (32U))) = {0U};
	u64 PpkHashAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(PpkHashAddr, (u64)(UINTPTR)&PpkHash, sizeof(PpkHash));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWritePpkHash(PpkType, &PpkHash);

END:
	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs MiscCtrl eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	MiscCtrlBits - MiscCtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteMiscCtrl(u32 EnvMonDisFlag, u32 MiscCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteMiscCtrlBits(EnvMonDisFlag, MiscCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs SecCtrlBits eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	SecCtrlBits - SecCtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteSecCtrl(u32 EnvMonDisFlag, u32 SecCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteSecCtrlBits(EnvMonDisFlag, SecCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs IV received via IPI into eFUSEs.
 *
 * @param	IvType		Type of IV
 * @param	AddrLow		Lower Address of the IV buffer
 * @param	AddrHigh	Higher Address of the IV buffer
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteIvs(XNvm_IvType IvType, u32 AddrLow, u32 AddrHigh)
{
	volatile int Status = XST_FAILURE;
	XNvm_Iv Iv __attribute__ ((aligned (32U))) = {0U};
	u64 IvAddr = ((u64)AddrHigh << 32U) | (u64)AddrLow;

	Status = XNvm_EfuseMemCopy(IvAddr, (u64)(UINTPTR)&Iv, sizeof(Iv));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XNvm_EfuseWriteIv(IvType, &Iv);

END:
	return Status;
}

/*****************************************************************************/
/**
 ** @brief       This function programs Misc1Ctrl eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	Misc1CtrlBits - Misc1CtrlBits input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteMisc1Ctrl(u32 EnvMonDisFlag, u32 Misc1CtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteMisc1Bits(EnvMonDisFlag, Misc1CtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BootEnvCtrl eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	BootEnvCtrl - BootEnvCtrl input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteBootEnvCtrl(u32 EnvMonDisFlag, u32 BootEnvCtrlBits)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteBootEnvCtrlBits(EnvMonDisFlag, BootEnvCtrlBits);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Fips mode and Fips version eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 * @param 	FipsInfo - FipsMode and FipsVersion input to be programmed
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteFipsInfoFuses(u32 EnvMonDisFlag, u32 FipsInfo)
{
	volatile int Status = XST_FAILURE;
	u32 FipsMode = FipsInfo & XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK;
	u32 FipsVersion = (FipsInfo & (~XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK)) >> 16U;

	Status = XNvm_EfuseWriteFipsInfo(EnvMonDisFlag, FipsMode, FipsVersion);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DICE UDS eFuses
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDiceUds(u32 *Pload)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteUds(Pload[0U], (XNvm_Uds *)&Pload[1U]);

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
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDmeKeyFromPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDmeUserKey((XNvm_DmeKeyType)Pload[0U], (XNvm_DmeKey *)&Pload[1U]);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs DmeRevoke eFuses
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteDmeRevokeBits(u32 Pload)
{
	volatile int Status = XST_FAILURE;
	u32 DmeRevokeNum = Pload & XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK;
	u32 EnvMonDisFlag = Pload & ~XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK;
	Status = XNvm_EfuseWriteDmeRevoke(EnvMonDisFlag, DmeRevokeNum);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs PLM_UPDATE eFuses
 *
 * @param 	EnvMonDisFlag - Environmental monitoring flag set by the user,
 * 				when set to true it will not check for voltage
 * 				and temperature limits.
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePlmUpdate(u32 EnvMonDisFlag)
{
	volatile int Status = XST_FAILURE;

	Status = XNvm_EfuseWriteDisableInplacePlmUpdate(EnvMonDisFlag);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs BootModeDisable eFuses
 *
 * @param 	Pload is pointer to the CDO payload
 *
 * @return	- XST_SUCCESS - If the programming is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWriteBootModeDis(u32 Pload)
{
	volatile int Status = XST_FAILURE;
	u32 BootModeDisMask = Pload & XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK;
	u32 EnvMonDisFlag = Pload & (~XNVM_EFUSE_LOWER_DOUBLE_BYTE_MASK);

	Status = XNvm_EfuseWriteBootModeDisable(EnvMonDisFlag, BootModeDisMask);

	return Status;
}

/*****************************************************************************/
/**
 * @brief       This function programs Puf helper data, Chash, Aux, Puf Ctrl
 * 		bits. RoSwap eFuses
 *
 * @return	- XST_SUCCESS - If the write is successful
 * 		- ErrorCode - If there is a failure
 *
 ******************************************************************************/
static int XNvm_EfuseWritePufDataFromCdoPload(u32 *Pload)
{
	volatile int Status = XST_FAILURE;
	XNvm_EfusePufHdAddr PufData = {0U};
	u32 Index = 0U;
	PufData.PufSecCtrlBits = Pload[1U];
	PufData.PrgmPufHelperData = 1U;
	PufData.EnvMonitorDis = (u8)Pload[0U];
	PufData.Chash = Pload[2U];
	PufData.Aux = Pload[3U];
	PufData.RoSwap = Pload[4U];

	for (Index = 0U; Index < XNVM_PUF_FORMATTED_SYN_DATA_LEN_IN_WORDS; Index++) {
		PufData.EfuseSynData[Index] = Pload[Index + 5U];
	}

	Status = XNvm_EfuseWritePuf(&PufData);

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
