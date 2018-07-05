/******************************************************************************
*
* Copyright (C) 2015 - 18 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xfsbl_authentication.c
*
* Contains the function definitions for RSA Signature verification.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   ssc  01/20/16 Initial release
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines.
*       vns  02/17/17 Added PPK hash and SPK ID verification when eFUSE
*                     RSA authentication is enabled
* 3.0   vns  09/08/17 Added PPK revoke check.
* 4.0   vns  01/23/18 Added KECCAK SHA3 padding selection for SPK signature
*                     verification and PPK hash caclulation, however partition
*                     will be authenticated with NIST SHA3 padding
*       vns  03/07/18 Added API to do boot header authentication, removed
*                     PPK verification for every partition instead saving
*                     PPK key at the time of boot header authentication,
*                     Modified XFsbl_PpkSpkIdVer to XFsbl_PpkVer which
*                     takes care of PPK revocation checks as well and
*                     Modified XFsbl_ReadPpkHashSpkID to XFsbl_ReadPpkHash
*                     as SPK ID reading and verification moved to XFsbl_SpkVer.
* 5.0   ka   04/10/18 Added support for user-efuse revocation
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"
#include "xfsbl_bs.h"

#ifdef XFSBL_SECURE

#include "xfsbl_authentication.h"
#include "xfsbl_csu_dma.h"

u32 XFsbl_SpkVer(u64 AcOffset, u32 HashLen);
u32 XFsbl_PpkVer(u64 AcOffset, u32 HashLen);
void XFsbl_ReadPpkHash(u32 *PpkHash, u8 PpkSelect);
/*****************************************************************************/

static XSecure_Rsa SecureRsa;

#if defined(XFSBL_BS)
extern u8 ReadBuffer[READ_BUFFER_SIZE];
#endif

u8 EfusePpkKey[XFSBL_PPK_SIZE]__attribute__ ((aligned (32))) = {0U};

/*****************************************************************************/
/**
 * Configure the RSA and SHA for the SPK
 * Signature verification.
 * If SPK Signature verification fails
 * then return unique error code saying
 * XFSBL_STAGE3_SPK_SIGN_VERIF_ERROR.
 *
 * @param
 *
 * @return
 *
 * @note	This function uses verified PPK key but not from current
 * 		authentication certificate. Also checks SPK revocation if it
 *		is not boot header authentication.
 ******************************************************************************/
u32 XFsbl_SpkVer(u64 AcOffset, u32 HashLen)
{
	u8 SpkHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4)))={0};
	u8* PpkModular;
	u8* PpkModularEx;
	u8* PpkExpPtr;
	u32 PpkExp;
	u8 * AcPtr = (u8*) (PTRSIZE) AcOffset;
	u8 SpkIdFuseSel = ((*(u32 *)(AcPtr) & XFSBL_AH_ATTR_SPK_ID_FUSE_SEL_MASK)) >>
		                                        XFSBL_AH_ATTR_SPK_ID_FUSE_SEL_SHIFT;
	u32 Status;
	void * ShaCtx = (void * )NULL;
	u8 XFsbl_RsaSha3Array[512] = {0};
	u8 *PpkKey = EfusePpkKey;
	u32 EfuseRsa = XFsbl_In32(EFUSE_SEC_CTRL);
	u32 EfuseSpkId;
	u32 *SpkId = (u32 *)(AcPtr + XFSBL_SPKID_AC_ALIGN);
	u32 UserFuseAddr;
	u32 UserFuseVal;

#ifdef XFSBL_SHA2
	sha2_context ShaCtxObj;
	ShaCtx = &ShaCtxObj;
#endif

	/* Re-initialize CSU DMA. This is a workaround and need to be removed */
	Status = XFsbl_CsuDmaInit();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	(void)XFsbl_ShaStart(ShaCtx, HashLen);
	if (SpkIdFuseSel == XFSBL_SPKID_EFUSE) {
		Status = XFsbl_Sha3PadSelect(XSECURE_CSU_KECCAK_SHA3);
	}
	else if (SpkIdFuseSel != XFSBL_USER_EFUSE) {
		Status = XFSBL_ERROR_INVALID_EFUSE_SELECT;
		XFsbl_Printf(DEBUG_INFO,"Invalid SpkIdFuseSel: %u\n\r : ",SpkIdFuseSel);
		XFsbl_Printf(DEBUG_GENERAL,
					"XFsbl_SpkVer: "
				        "XFSBL_ERROR_INVALID_EFUSE_SELECT\r\n");
		goto END;
	}

	if (Status != XST_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL,
					"XFsbl_SpkVer: Error in SHA3 padding selection\r\n");
		goto END;
	}

	/* Hash the PPK + SPK choice */
	XFsbl_ShaUpdate(ShaCtx, AcPtr, 8, HashLen);

	/* Calculate SPK + Auth header Hash */
	XFsbl_ShaUpdate(ShaCtx, (u8 *)(AcPtr + XFSBL_AUTH_CERT_SPK_OFFSET),
					XFSBL_SPK_SIZE, HashLen);

	XFsbl_ShaFinish(ShaCtx, (u8 *)SpkHash, HashLen);

	/* Set PPK pointer */
	PpkModular = (u8 *)PpkKey;
	PpkKey += XFSBL_PPK_MOD_SIZE;
	PpkModularEx = PpkKey;
	PpkKey += XFSBL_PPK_MOD_EXT_SIZE;
	PpkExp = *((u32 *)PpkKey);
	PpkExpPtr = PpkKey;

	if((PpkModular != NULL) && (PpkModularEx != NULL)) {
	XFsbl_Printf(DEBUG_DETAILED,
		"XFsbl_SpkVer: Ppk Mod %0x, Ppk Mod Ex %0x, Ppk Exp %0x\r\n",
		PpkModular, PpkModularEx, PpkExp);
		XFsbl_PrintArray(DEBUG_DETAILED, PpkModular, XFSBL_PPK_MOD_SIZE, "Ppk Modular");
		XFsbl_PrintArray(DEBUG_DETAILED, PpkModularEx, XFSBL_PPK_MOD_EXT_SIZE, "Ppk ModularEx");
		XFsbl_Printf(DEBUG_DETAILED, "Ppk Exp = %x\n\r", PpkExp);
	}

	/* Set SPK Signature pointer */
	if(PpkExpPtr!=NULL) {
		Status = (u32)XSecure_RsaInitialize(&SecureRsa, PpkModular,
			PpkModularEx, PpkExpPtr);
	}
	else
	{
		Status = (u32)(XFSBL_FAILURE);
	}

	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_RSA_INITIALIZE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_RSA_INITIALIZE\r\n");
		goto END;
	}
	/* Decrypt SPK Signature */
	if(XFSBL_SUCCESS != XSecure_RsaDecrypt(&SecureRsa,
		AcPtr + XFSBL_AUTH_CERT_SPK_SIG_OFFSET, XFsbl_RsaSha3Array))
	{
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_SpkVer: XFSBL_ERROR_SPK_RSA_DECRYPT\r\n");
		Status = XFSBL_ERROR_SPK_RSA_DECRYPT;
		goto END;
	}

	/* Authenticate SPK Signature */
	if(XFSBL_SUCCESS != XSecure_RsaSignVerification(XFsbl_RsaSha3Array,
							SpkHash, HashLen))
	{
		XFsbl_PrintArray(DEBUG_INFO, SpkHash,
				HashLen, "Calculated SPK Hash");
		XFsbl_PrintArray(DEBUG_INFO, XFsbl_RsaSha3Array,
				512, "RSA decrypted Hash");
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_SpkVer: XFSBL_ERROR_SPK_SIGNATURE\r\n");
		Status = XFSBL_ERROR_SPK_SIGNATURE;
		goto END;
	}

	/* SPK revocation check */
	if ((EfuseRsa & EFUSE_SEC_CTRL_RSA_EN_MASK) != 0x00) {
		EfuseSpkId = Xil_In32(EFUSE_SPKID);

		/* If SPKID Efuse is selected , Verifies SPKID with Efuse SPKID*/
		if (SpkIdFuseSel == XFSBL_SPKID_EFUSE) {
			if (EfuseSpkId != *SpkId) {
				Status = XFSBL_ERROR_SPKID_VERIFICATION;
				XFsbl_Printf(DEBUG_INFO,
						"Image's SPK ID : %x\n\r", SpkId);
				XFsbl_Printf(DEBUG_INFO,
						"eFUSE SPK ID: %x\n\r", EfuseSpkId);
				XFsbl_Printf(DEBUG_GENERAL,
						"XFsbl_SpkVer: "
						"XFSBL_ERROR_SPKID_VERIFICATION\r\n");
				goto END;
			}
		}
		/*
		 * If User EFUSE is selected, checks the corresponding User-Efuse bit
		 * programmed or not. If Programmed (indicates that key is revocated)
		 * throws an error
		 */
		else if (SpkIdFuseSel == XFSBL_USER_EFUSE) {
			if ((*SpkId >= XFSBL_USER_EFUSE_MIN_VALUE) &&
				(*SpkId <= XFSBL_USER_EFUSE_MAX_VALUE)) {
				UserFuseAddr = XFSBL_USER_EFUSE_ADDR +
								(((*SpkId - 1) / XFSBL_WORD_SHIFT) *
											XFSBL_WORD_LEN_IN_BYTES);
				UserFuseVal = Xil_In32(UserFuseAddr);
				if ((UserFuseVal & (0x1U << ((*SpkId - 1) %
									XFSBL_WORD_SHIFT))) != 0x0U) {
					Status = XFSBL_ERROR_USER_EFUSE_ISREVOKED;
					XFsbl_Printf(DEBUG_GENERAL,
							"XFsbl_SpkVer: "
							"XFSBL_ERROR_USER_EFUSE_ISREVOKED\r\n");
					goto END;
				}
			}
			else {
				Status = XFSBL_ERROR_OUT_OF_RANGE_USER_EFUSE;
				XFsbl_Printf(DEBUG_GENERAL,
								"XFsbl_SpkVer: "
								"XFSBL_ERROR_OUT_OF_RANGE_USER_EFUSE\r\n");
				goto END;
			}
		}
	}

END:
	return Status;
}

/*****************************************************************************/
/**
 * Configure the RSA and SHA for the
 * Boot Header Signature verification.
 * If Boot Header Signature verification
 * fails then return unique error code saying
 * XFSBL_STAGE3_BOOT_HDR_SIGN_VERIF_ERROR.
 *
 * @param
 *
 * @return
 *
 ******************************************************************************/
static u32 XFsbl_PartitionSignVer(const XFsblPs *FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset,
				u32 PartitionNum)
{

	u8 PartitionHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4))) = {0};
	u8 * SpkModular;
	u8* SpkModularEx;
	u32 SpkExp;
	u8 * AcPtr = (u8*)(PTRSIZE) AcOffset;
	u32 Status;
	u32 HashDataLen;
	void * ShaCtx = (void * )NULL;
	u8 XFsbl_RsaSha3Array[512] = {0};
	u32 HashLen;
	s32 SStatus;

#ifdef XFSBL_SHA2
	sha2_context ShaCtxObj;
	ShaCtx = &ShaCtxObj;
#endif

	XFsbl_Printf(DEBUG_INFO, "Doing Partition Sign verification\r\n");

	/* Get the Sha type to be used from boot header attributes */
	if ((FsblInstancePtr->BootHdrAttributes &
			XIH_BH_IMAGE_ATTRB_SHA2_MASK) ==
			XIH_BH_IMAGE_ATTRB_SHA2_MASK) {
		HashLen = XFSBL_HASH_TYPE_SHA2;
	}
	else
	{
		HashLen = XFSBL_HASH_TYPE_SHA3;
	}

	/**
	 * total partition length to be hashed except the AC
	 */
	HashDataLen = PartitionLen - XFSBL_AUTH_CERT_MIN_SIZE;

	/* Start the SHA engine */
	(void)XFsbl_ShaStart(ShaCtx, HashLen);

	/* Calculate Partition Hash */
#ifndef XFSBL_PS_DDR
	const XFsblPs_PartitionHeader * PartitionHeader;
	u32 DestinationDevice = 0U;
	PartitionHeader =
		&FsblInstancePtr->ImageHeader.PartitionHeader[PartitionNum];
	DestinationDevice = XFsbl_GetDestinationDevice(PartitionHeader);

	if (DestinationDevice == XIH_PH_ATTRB_DEST_DEVICE_PL)
	{
#ifdef XFSBL_BS
		if(XFSBL_SUCCESS != XFsbl_ShaUpdate_DdrLess(FsblInstancePtr,
		 ShaCtx, PartitionOffset, HashDataLen, HashLen, PartitionHash))
		{
			XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_PartitionVer: XFSBL_ERROR_PART_RSA_DECRYPT\r\n");
			Status = XFSBL_ERROR_PART_RSA_DECRYPT;
			goto END;
		}

#endif
	}
	else
	{
		XFsbl_Printf(DEBUG_INFO, "XFsbl_PartitionVer: SHA calc. "
					"for non bs DDR less partition \r\n");
		/* SHA calculation for non-bitstream, DDR less partitions */
		XFsbl_ShaUpdate(ShaCtx, (u8 *)(PTRSIZE)PartitionOffset,
							HashDataLen, HashLen);
	}
#else
	/* SHA calculation in DDRful systems */
	XFsbl_ShaUpdate(ShaCtx, (u8 *)(PTRSIZE)PartitionOffset, HashDataLen, HashLen);

#endif

	/* Calculate hash for (AC - signature size) */
	XFsbl_ShaUpdate(ShaCtx, (u8 *)(PTRSIZE)AcOffset,
			(XFSBL_AUTH_CERT_MIN_SIZE - XFSBL_FSBL_SIG_SIZE), HashLen);

	XFsbl_ShaFinish(ShaCtx, (u8 *)PartitionHash, HashLen);

	/* Set SPK pointer */
	AcPtr += (XFSBL_RSA_AC_ALIGN + XFSBL_PPK_SIZE);
	SpkModular = AcPtr;
	AcPtr += XFSBL_SPK_MOD_SIZE;
	SpkModularEx = AcPtr;
	AcPtr += XFSBL_SPK_MOD_EXT_SIZE;
	SpkExp = *((u32 *)AcPtr);
	AcPtr += XFSBL_RSA_AC_ALIGN;

	/* Increment by  SPK Signature pointer */
	AcPtr += XFSBL_SPK_SIG_SIZE;
	/* Increment by  BHDR Signature pointer */
	AcPtr += XFSBL_BHDR_SIG_SIZE;
	if((SpkModular != NULL) && (SpkModularEx != NULL)) {
	XFsbl_Printf(DEBUG_DETAILED,
		"XFsbl_PartVer: Spk Mod %0x, Spk Mod Ex %0x, Spk Exp %0x\r\n",
		SpkModular, SpkModularEx, SpkExp);
		XFsbl_PrintArray(DEBUG_DETAILED, SpkModular, XFSBL_SPK_MOD_SIZE, "Spk Modular");
		XFsbl_PrintArray(DEBUG_DETAILED, SpkModularEx, XFSBL_SPK_MOD_EXT_SIZE, "Spk ModularEx");
		XFsbl_Printf(DEBUG_DETAILED, "Spk Exp %x\n\r", SpkExp);
	}
	XFsbl_Printf(DEBUG_INFO,
			"Partition Verification done \r\n");

	SStatus = XSecure_RsaInitialize(&SecureRsa, SpkModular,
				SpkModularEx, (u8 *)&SpkExp);
	if (SStatus != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_RSA_INITIALIZE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_RSA_INITIALIZE\r\n");
		goto END;
	}
	/* Decrypt Partition Signature. */
	if(XFSBL_SUCCESS !=
		XSecure_RsaDecrypt(&SecureRsa, AcPtr, XFsbl_RsaSha3Array))
	{
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_SpkVer: XFSBL_ERROR_PART_RSA_DECRYPT\r\n");
		Status = XFSBL_ERROR_PART_RSA_DECRYPT;
		goto END;
	}

	/* Authenticate Partition Signature */
	if(XFSBL_SUCCESS != XSecure_RsaSignVerification(XFsbl_RsaSha3Array,
				PartitionHash, HashLen))
	{
		XFsbl_PrintArray(DEBUG_INFO, PartitionHash,
				HashLen, "Calculated Partition Hash");
		XFsbl_PrintArray(DEBUG_INFO, XFsbl_RsaSha3Array,
				512, "RSA decrypted Hash");
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_PartVer: XFSBL_ERROR_PART_SIGNATURE\r\n");
		Status = XFSBL_ERROR_PART_SIGNATURE;
		goto END;
	}
	Status = XFSBL_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
 *
 * @param       None
 *
 * @return      None
 *
 ******************************************************************************/
u32 XFsbl_Authentication(const XFsblPs * FsblInstancePtr, u64 PartitionOffset,
				u32 PartitionLen, u64 AcOffset,
				u32 PartitionNum)
{
        u32 Status;
        u32 HashLen;

	/* Get the Sha type to be used from boot header attributes */
	if ((FsblInstancePtr->BootHdrAttributes &
			XIH_BH_IMAGE_ATTRB_SHA2_MASK) ==
			XIH_BH_IMAGE_ATTRB_SHA2_MASK) {
		HashLen = XFSBL_HASH_TYPE_SHA2;
	}
	else
	{
		HashLen = XFSBL_HASH_TYPE_SHA3;
	}

	XFsbl_Printf(DEBUG_INFO,
		"Auth: Partition Offset %0x, PartitionLen %0x,"
		" AcOffset %0x, HashLen %0x\r\n",
		(PTRSIZE )PartitionOffset, PartitionLen,
		(PTRSIZE )AcOffset, HashLen);
        /* Do SPK Signature verification using PPK */
        Status = XFsbl_SpkVer(AcOffset, HashLen);

        if(XFSBL_SUCCESS != Status)
        {
                goto END;
        }

        /* Do Partition Signature verification using SPK */
        Status = XFsbl_PartitionSignVer(FsblInstancePtr, PartitionOffset,
					PartitionLen, AcOffset, PartitionNum);

        if(XFSBL_SUCCESS != Status)
        {
                goto END;
        }

END:
        return Status;
}
#ifndef XFSBL_PS_DDR
#ifdef XFSBL_BS
/*****************************************************************************/
/**
 *
 * @param      FsblInstancePtr - FSBL Instance Pointer
 * @param      Ctx - SHA Ctx Pointer
 * @param      PartitionOffset - Start Offset
 * @param      PatitionLen - Data Len for SHA calculation
 * @param      HashLen - SHA3/SHA2
 * @param      ParitionHash - Pointer to store hash
 *
 * @return     XFSBL_SUCCESS - In case of Success
 *             XFSBL_FAILURE - In case of Failure
 *
 ******************************************************************************/
u32 XFsbl_ShaUpdate_DdrLess(const XFsblPs *FsblInstancePtr, void *Ctx,
		u64 PartitionOffset, u32 PartitionLen,
		u32 HashLen, u8 *PartitionHash)
{
	u32 Status = XFSBL_SUCCESS;
	/**
	 * bitstream partion in DDR less system, Chunk by chunk copy
	 * into OCM and update SHA module
	 */
	u32 NumChunks = 0U;
	u32 RemainingBytes = 0U;
	u32 Index = 0U;
	u32 StartAddrByte = PartitionOffset;

	NumChunks = PartitionLen / READ_BUFFER_SIZE;
	RemainingBytes = (PartitionLen % READ_BUFFER_SIZE);

			/* Start the SHA engine */
		(void)XFsbl_ShaStart(Ctx, HashLen);

		XFsbl_Printf(DEBUG_INFO,
			"XFsbl_PartitionVer: NumChunks :%0d, RemainingBytes : %0d \r\n",
			NumChunks, RemainingBytes);

		for(Index = 0; Index < NumChunks; Index++)
		{
			if(XFSBL_SUCCESS !=FsblInstancePtr->DeviceOps.DeviceCopy(
					StartAddrByte, (PTRSIZE)ReadBuffer,
					READ_BUFFER_SIZE))
			{
				XFsbl_Printf(DEBUG_GENERAL,
					"XFsblPartitionVer: Device "
					"to OCM copy of partition failed \r\n");
				Status = XFSBL_FAILURE;
				goto END;
			}

			XFsbl_ShaUpdate(Ctx, (u8 *)ReadBuffer,
						READ_BUFFER_SIZE, HashLen);

			StartAddrByte += READ_BUFFER_SIZE;
		}

		/* Send the residual bytes if Size is not buffer size multiple */
		if(RemainingBytes != 0)
		{
			if(XFSBL_SUCCESS!=FsblInstancePtr->DeviceOps.DeviceCopy(
						StartAddrByte, (PTRSIZE)ReadBuffer,
						RemainingBytes))
			{
				XFsbl_Printf(DEBUG_GENERAL,
					"XFsblPartitionVer: Device "
					"to OCM copy of partition failed (last chunk)\r\n");
				Status = XFSBL_FAILURE;
				goto END;
			}

			XFsbl_ShaUpdate(Ctx, (u8 *)ReadBuffer,
						RemainingBytes, HashLen);
		}
END:
		return Status;

}
#endif
#endif

/*****************************************************************************/
/*
* This function is used to read PPK0/PPK1 hash from efuse based on PPK
* selection.
*
* @param	PpkHash is a pointer to an array which holds the readback
*		PPK hash in.
* @param	PpkSelect is a u8 variable which has to be provided by user
*		based on this input reading is happens from efuse PPK0 or PPK1
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XFsbl_ReadPpkHash(u32 *PpkHash, u8 PpkSelect)
{
	s32 RegNum;
	u32 *DataRead = PpkHash;

	if (PpkSelect == 0) {
		for (RegNum = 0; RegNum < 12; RegNum++) {
			*DataRead = Xil_In32(EFUSE_PPK0 + (RegNum * 4));
			DataRead++;
		}
	}
	else {
		for (RegNum = 0; RegNum < 12; RegNum++) {
			*DataRead = Xil_In32(EFUSE_PPK1 + (RegNum * 4));
			DataRead++;
		}
	}

}

/*****************************************************************************/
/*
* This function is used to verify PPK hash with
* the values stored on eFUSE.
*
* @param	AcOffset is the Authentication certificate offset which has
*		AC.
* @param	HashLen holds the type of authentication enabled.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
u32 XFsbl_PpkVer(u64 AcOffset, u32 HashLen)
{
	u8 PpkHash[XFSBL_HASH_TYPE_SHA3]
			   __attribute__ ((aligned (4)));
	void * ShaCtx = (void * )NULL;
	u8 * AcPtr = (u8*) (PTRSIZE) AcOffset;
	u32 Status = XFSBL_SUCCESS;
	u8 EfusePpkHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4)))={0U};

	(void)memset(PpkHash,0U,sizeof(PpkHash));

	if ((*(u32 *)(AcPtr)
			& XIH_AC_ATTRB_PPK_SELECT_MASK) == 0x00U) {
		/* PPK revoke check */
		if ((Xil_In32(EFUSE_SEC_CTRL) &
			EFUSE_SEC_CTRL_PPK0_RVK_MASK) != 0x00) {
			Status = XSFBL_ERROR_PPK_SELECT_ISREVOKED;
			goto END;
		}
		/* PPK 0 */
		XFsbl_ReadPpkHash((u32 *)EfusePpkHash, 0U);
	}
	else {
		/* PPK revoke check */
		if ((Xil_In32(EFUSE_SEC_CTRL) &
			EFUSE_SEC_CTRL_PPK1_RVK_MASK) != 0x00) {
			Status = XSFBL_ERROR_PPK_SELECT_ISREVOKED;
			goto END;
		}
		/* PPK 1 */
			XFsbl_ReadPpkHash((u32 *)EfusePpkHash, 1U);
	}


	/* Hash calculation on PPK */
	(void)XFsbl_ShaStart(ShaCtx, HashLen);
	Status = XFsbl_Sha3PadSelect(XSECURE_CSU_KECCAK_SHA3);
	if (Status != XST_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL,
		"XFsbl_PartVer: Error in SHA3 padding selection\r\n");
		goto END;
	}
	/* Hash the PPK  choice */
	XFsbl_ShaUpdate(ShaCtx, AcPtr + XFSBL_RSA_AC_ALIGN,
					XFSBL_PPK_SIZE, HashLen);
	XFsbl_ShaFinish(ShaCtx, (u8 *)PpkHash, HashLen);

	/* Compare hashs */
	Status = XFsbl_CompareHashs(PpkHash, EfusePpkHash, HashLen);
	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_PPK_VERIFICATION;
		XFsbl_PrintArray(DEBUG_INFO, PpkHash,
				HashLen, "Image PPK Hash");
		XFsbl_PrintArray(DEBUG_INFO, EfusePpkHash,
				HashLen, "eFUSE PPK Hash");
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_PartVer: XFSBL_ERROR_PPK_VERIFICATION\r\n");
		goto END;

	}

END:

	return Status;

}

/******************************************************************************
*
* This function compares the hashs
*
* @param	Hash1 stores the hash to be compared.
* @param	Hash2 stores the hash to be compared.
*
* @return
* 		Error code on failure
* 		XFSBL_SUCESS on success
*
* @note		None.
*
******************************************************************************/
u32 XFsbl_CompareHashs(u8 *Hash1, u8 *Hash2, u32 HashLen)
{
	u8 Index;
	u32 *HashOne = (u32 *)Hash1;
	u32 *HashTwo = (u32 *)Hash2;

	for (Index = 0; Index < HashLen/4; Index++) {
		if (HashOne[Index] != HashTwo[Index]) {
			return XFSBL_FAILURE;
		}
	}

	return XFSBL_SUCCESS;
}

/*****************************************************************************/
/**
 * This function performs authentication of boot header.
 *
 * @param	FsblInstancePtr is an pointer to FSBL instance
 * @param	Data is pointer to boot header buffer.
 * @param	AcOffset is the address of authentication certificate
 * @param	IsEfuseRsa variable tells whether RSA bit of eFUSE is enabled
 * 		or not
 *
 * @return	XFSBL_SCUCCESS on success
 *
 * @note	This API also copies PPK key to internal buffer for other
 *		partitions.
 *
 ******************************************************************************/
u32 XFsbl_BhAuthentication(const XFsblPs * FsblInstancePtr, u8 *Data,
					u64 AcOffset, u8 IsEfuseRsa)
{
	u32 Status = XST_SUCCESS;
	u32 HashLen;
	void * ShaCtx = (void * )NULL;
	u32 SizeofBH;
	u8 BhHash[XFSBL_HASH_TYPE_SHA3] __attribute__ ((aligned (4)))={0};
	u8 * SpkModular;
	u8* SpkModularEx;
	u32 SpkExp;
	u8 XFsbl_RsaSha3Array[512] = {0};
	u8 * AcPtr = (u8*) (PTRSIZE) AcOffset;

#ifdef XFSBL_SHA2
	sha2_context ShaCtxObj;
	ShaCtx = &ShaCtxObj;
#endif
	/* Get the Sha type to be used from boot header attributes */
	if ((FsblInstancePtr->BootHdrAttributes &
				XIH_BH_IMAGE_ATTRB_SHA2_MASK) ==
				XIH_BH_IMAGE_ATTRB_SHA2_MASK) {
			HashLen = XFSBL_HASH_TYPE_SHA2;
	}
	else{
		HashLen = XFSBL_HASH_TYPE_SHA3;
	}

	/* Size of Boot header */
	if ((FsblInstancePtr->BootHdrAttributes &
		XIH_BH_IMAGE_ATTRB_PUF_BH_MASK) != 0x00) {
		SizeofBH = XIH_BH_MAX_SIZE;
	}
	else {
		SizeofBH = XIH_BH_MIN_SIZE;
	}

	/* Initialize CSU DMA driver */
	Status = XFsbl_CsuDmaInit();
	if (XFSBL_SUCCESS != Status) {
		goto END;
	}

	/* If EFUSE RSA enabled verify PPK */
	if (IsEfuseRsa != 0x00U) {
		Status = XFsbl_PpkVer(AcOffset, HashLen);
		if (Status != XFSBL_SUCCESS) {
			XFsbl_Printf(DEBUG_GENERAL,
				"XFsbl_BhAuthentication:"
				" Error in PPK verification\r\n");
			goto END;
		}
	}

	/* Copy PPK to global variable for future use */
	XFsbl_MemCpy(EfusePpkKey, AcPtr + XFSBL_AUTH_CERT_PPK_OFFSET,
						XFSBL_PPK_SIZE);

	/* SPK verify */
	Status = XFsbl_SpkVer(AcOffset, HashLen);
	if (Status != XFSBL_SUCCESS) {
		goto END;
	}

	/* Authentication of boot header */

	/* Initialize Sha and RSA instances */
	(void)XFsbl_ShaStart(ShaCtx, HashLen);
	Status = XFsbl_Sha3PadSelect(XSECURE_CSU_KECCAK_SHA3);
	if (Status != XST_SUCCESS) {
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_BhAuthentication:"
			" Error in SHA3 padding selection\r\n");
		goto END;
	}

	/* Calculate Hash on Data to be authenticated */
	XFsbl_ShaUpdate(ShaCtx, Data, SizeofBH, HashLen);
	XFsbl_ShaFinish(ShaCtx, BhHash, HashLen);

	/* Set SPK pointer */
	AcPtr += (XFSBL_RSA_AC_ALIGN + XFSBL_PPK_SIZE);
	SpkModular = AcPtr;
	AcPtr += XFSBL_SPK_MOD_SIZE;
	SpkModularEx = AcPtr;
	AcPtr += XFSBL_SPK_MOD_EXT_SIZE;
	SpkExp = *((u32 *)AcPtr);
	AcPtr += XFSBL_RSA_AC_ALIGN;

	/* Increment by  SPK Signature pointer */
	AcPtr += XFSBL_SPK_SIG_SIZE;

	if((SpkModular != NULL) && (SpkModularEx != NULL)) {
		XFsbl_Printf(DEBUG_DETAILED,
			"XFsbl_BhAuthentication: "
			"Spk Mod %0x, Spk Mod Ex %0x, Spk Exp %0x\r\n",
		SpkModular, SpkModularEx, SpkExp);
		XFsbl_PrintArray(DEBUG_DETAILED, SpkModular,
			XFSBL_SPK_MOD_SIZE, "Spk Modular");
		XFsbl_PrintArray(DEBUG_DETAILED, SpkModularEx,
			XFSBL_SPK_MOD_EXT_SIZE, "Spk ModularEx");
		XFsbl_Printf(DEBUG_DETAILED, "Spk Exp = %x\n\r", SpkExp);
	}

	/* Set SPK Signature pointer */
	Status = XSecure_RsaInitialize(&SecureRsa, SpkModular,
					SpkModularEx, (u8 *)&SpkExp);

	if (Status != XFSBL_SUCCESS) {
		Status = XFSBL_ERROR_RSA_INITIALIZE;
		XFsbl_Printf(DEBUG_GENERAL, "XFSBL_ERROR_RSA_INITIALIZE\r\n");
		goto END;
	}
	/* Decrypt SPK Signature */
	if(XFSBL_SUCCESS !=
		XSecure_RsaDecrypt(&SecureRsa, AcPtr, XFsbl_RsaSha3Array))
	{
		XFsbl_Printf(DEBUG_GENERAL,"XFsbl_BhAuthentication:"
				" XFSBL_ERROR_BH_RSA_DECRYPT\r\n");
			Status = XFSBL_ERROR_BH_RSA_DECRYPT;
		goto END;
	}

	/* Authenticate SPK Signature */
	if(XFSBL_SUCCESS != XSecure_RsaSignVerification(XFsbl_RsaSha3Array,
			BhHash, HashLen))
	{
		XFsbl_PrintArray(DEBUG_INFO, BhHash,
				HashLen, "Calculated Boot header Hash");
		XFsbl_PrintArray(DEBUG_INFO, XFsbl_RsaSha3Array,
				512, "RSA decrypted Hash");
		XFsbl_Printf(DEBUG_GENERAL,
			"XFsbl_BhAuthentication: XFSBL_ERROR_BH_SIGNATURE\r\n");
		Status = XFSBL_ERROR_BH_SIGNATURE;
	}

END:
	return Status;
}

#endif /* end of XFSBL_SECURE */
