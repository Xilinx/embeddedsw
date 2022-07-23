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
static INLINE int XNvm_EfuseMemCopy(u64 SourceAddr, u64 DestAddr, u32 Len);

/***************** Macros (Inline Functions) Definitions *********************/
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
	case XNVM_API(XNVM_API_ID_EFUSE_RELOAD_N_PRGM_PROT_BITS):
		Status =  XNvm_EfuseCacheLoadNPrgmProtBits();
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
		XPlmi_OutByte64(OutputBuffer, RegData);

		OutputBuffer++;
		Offset = Offset + XNVM_WORD_LEN;
	}

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
#endif
