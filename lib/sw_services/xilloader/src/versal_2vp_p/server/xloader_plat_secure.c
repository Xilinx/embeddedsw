/***************************************************************************************************
* Copyright (c) 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file versal_2vp/server/xloader_plat_secure.c
*
* This file contains the versal_2vp specific secure code related to PDI image
* loading.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 2.3   sd  10/13/25 Initial release
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/*************************************** Include Files ********************************************/
#include "xplmi_config.h"
#include "xloader_secure.h"
#include "xilpdi.h"
#include "xplmi.h"
#include "xplmi_status.h"
#include "xloader_plat_secure.h"

/************************************ Constant Definitions ****************************************/

/************************************** Type Definitions ******************************************/

/*************************** Macros (Inline Functions) Definitions ********************************/

/************************************ Function Prototypes *****************************************/

#ifndef PLM_SECURE_EXCLUDE
#ifndef PLM_RSA_EXCLUDE
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert);
#endif

/************************************ Variable Definitions ****************************************/

/**************************************************************************************************/
/**
 * @brief	This function updates KEK red key availability status from boot header.
 *
 * @return
 *		- Decryption Key source status.
 *
 **************************************************************************************************/
u32 XLoader_GetKekSrc(void)
{
	volatile u32 DecKeySrc = 0x0U;
	const XilPdi_BootHdr *BootHdrPtr = (XilPdi_BootHdr *)(UINTPTR)XIH_BH_PRAM_ADDR;

	XPlmi_Printf(DEBUG_INFO, "Identifying KEK's corresponding RED key availability status\n\r");
	switch(BootHdrPtr->EncStatus) {
	case XLOADER_BH_BLK_KEY:
		DecKeySrc = XLOADER_BHDR_RED_KEY;
		break;
	case XLOADER_BBRAM_BLK_KEY:
		DecKeySrc = XLOADER_BBRAM_RED_KEY;
		break;
	case XLOADER_EFUSE_BLK_KEY:
		DecKeySrc = XLOADER_EFUSE_RED_KEY;
		break;
	default:
		/* No KEK is available for PLM */
		break;
	}
	XPlmi_Printf(DEBUG_DETAILED, "KEK red key available after for PLM %x\n\r", DecKeySrc);

	return DecKeySrc;
}

/**************************************************************************************************/
/**
 * @brief	This function provides Obfuscated Aes Key source.
 *
 * @param	PdiKeySrc is the Key source given in Pdi.
 * @param	DecKeyMask is the current DecKeyMask.
 * @param	KeySrcPtr is the pointer to the calculated KeySrc.
 *
 * @return
 *		- XST_SUCCESS on success and error code on failure.
 *
 **************************************************************************************************/
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 DecKeyMask, void *KeySrcPtr)
{
	(void)PdiKeySrc;
	(void)KeySrcPtr;
	(void)DecKeyMask;

	/* Obfuscated Key is not supported in Versal_2vp */
	return XLoader_UpdateMinorErr(XLOADER_SEC_AES_KEK_DEC, 0);
}

#ifndef PLM_RSA_EXCLUDE
/**************************************************************************************************/
/**
 * @brief	This function runs the KAT for RSA
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 **************************************************************************************************/
int XLoader_RsaKat(void) {
	int Status = XST_FAILURE;

	Status = XSecure_RsaPublicEncryptKat();

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function converts a non-negative integer to an octet string of a specified
 * 		length.
 *
 * @param	Integer is the variable in which input should be provided.
 * @param	Size holds the required size.
 * @param	Convert is a pointer in which output will be updated.
 *
 **************************************************************************************************/
static inline void XLoader_I2Osp(u32 Integer, u32 Size, u8 *Convert)
{
	if (Integer < XLOADER_I2OSP_INT_LIMIT) {
		Convert[Size - 1U] = (u8)Integer;
	}
}

/**************************************************************************************************/
/**
 * @brief	Mask generation function with SHA3.
 *
 * @param	Sha3InstancePtr is pointer to the XSecure_Sha3 instance.
 * @param	Out is pointer in which output of this function will be stored.
 * @param	OutLen specifies the required length.
 * @param	Input is pointer which holds the input data for	which mask should be calculated
 * 		which should be 48 bytes length.
 *
 * @return
 * 		- XST_SUCCESS on success.
 * 		- XLOADER_SEC_BUF_CLEAR_ERR if failed to clear buffer.
 *
 **************************************************************************************************/
int XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr, u8 * Out, u32 OutLen, u8 *Input)
{
	int Status = XST_FAILURE;
	int ClearStatus = XST_FAILURE;
	u32 Counter = 0U;
	u32 HashLen = XLOADER_SHA3_LEN;
	XSecure_Sha3Hash HashStore;
	u8 Convert[XIH_PRTN_WORD_LEN] = {0U};
	u32 Size = XLOADER_SHA3_LEN;
	u8 *OutTmp;

	if ((Sha3InstancePtr == NULL) || (Out == NULL) || (Input == NULL)) {
		goto END;
	}

	if (OutLen == 0U) {
		goto END;
	}

	OutTmp = Out;
	while (Counter <= (OutLen / HashLen)) {
		XLoader_I2Osp(Counter, XIH_PRTN_WORD_LEN, Convert);

		Status = XSecure_ShaStart(Sha3InstancePtr, XSECURE_SHA3_384);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)Input, HashLen);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaLastUpdate(Sha3InstancePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaUpdate(Sha3InstancePtr, (UINTPTR)Convert,
					XIH_PRTN_WORD_LEN);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		Status = XSecure_ShaFinish(Sha3InstancePtr, (u64)(UINTPTR)&HashStore, sizeof(HashStore));
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (Counter == (OutLen / HashLen)) {
			/*
			 * Only 463 bytes are required but the chunklen is 48 bytes.
			 * The extra bytes are discarded by the modulus operation below.
			 */
			 Size = (OutLen % HashLen);
		}
		Status = Xil_SMemCpy(OutTmp, Size, HashStore.Hash, Size, Size);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		OutTmp = &OutTmp[XLOADER_SHA3_LEN];
		Counter = Counter + 1U;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(Convert, sizeof(Convert), 0U, sizeof(Convert));
	ClearStatus |= XPlmi_MemSetBytes(&HashStore, XLOADER_SHA3_LEN, 0U, XLOADER_SHA3_LEN);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}
#endif

/**************************************************************************************************/
/**
 * @brief	This function verifies whether the additional PPK is valid.
 *
 * @param	PpkHash is pointer to the PPK hash.
 *
 * @return
 * 		- XST_SUCCESS on success and error code on failure
 *
 **************************************************************************************************/
int XLoader_IsAdditionalPpkValid(const u8 *PpkHash)
{
	int Status = XST_FAILURE;
	(void)PpkHash;

	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function checks for the additional PPK select and returns the PPK invalid mask
 * 		and PPK efuse cache start offset if PPK is valid.
 *
 * @param	PpkSelect   PPK selection of eFUSE.
 * @param	InvalidMask Pointer to the PPK invalid mask.
 * @param 	PpkOffset   Pointer to the efuse cache PPK start offset.
 *
 * @return
 *	 	- XST_SUCCESS on success and error code on failure.
 *
 **************************************************************************************************/
int XLoader_AdditionalPpkSelect(XLoader_PpkSel PpkSelect, u32 *InvalidMask, u32 *PpkOffset)
{
	int Status = XST_FAILURE;

	(void)PpkSelect;
	(void)InvalidMask;
	(void)PpkOffset;

	return Status;
}

#endif/* END OF PLM_SECURE_EXCLUDE */
/**************************************************************************************************/
/**
* @brief	This function checks Secure State for Authentication.
*
* @param	AHWRoT	Buffer to store Secure state for authentication.
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_ERR_GLITCH_DETECTED if glitch is detected.
*
***************************************************************************************************/
int XLoader_CheckSecureStateAuth(volatile u32* AHWRoT)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile u32 IsSignedImg = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;
	volatile u32 IsSignedImgTmp = XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE;

	IsSignedImg = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK) >> XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT);
	IsSignedImgTmp = ((XPlmi_In32(XIH_BH_PRAM_ADDR + XIH_BH_IMG_ATTRB_OFFSET) &
		XIH_BH_IMG_ATTRB_SIGNED_IMG_MASK) >> XIH_BH_IMG_ATTRB_SIGNED_IMG_SHIFT);

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_CheckNonZeroPpk);
	if ((Status == XST_SUCCESS) || (StatusTmp == XST_SUCCESS)) {
		/** - If PPK hash is programmed in eFUSEs, then Secure State of boot is A-HWRoT. */
		*AHWRoT = XPLMI_RTCFG_SECURESTATE_AHWROT;
		XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Asymmetric HWRoT\r\n");
	} else {
		if ((IsSignedImg == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE) ||
			(IsSignedImgTmp == XIH_BH_IMG_ATTRB_SIGNED_IMG_VALUE)) {
			/**
			 * - If PPK hash is not programmed in eFUSEs and PLM is authenticated then
			 *   Secure State of boot is emulated A-HWRoT.
			 */
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_EMUL_AHWROT;
			XPlmi_Printf(DEBUG_PRINT_ALWAYS, "State of Boot(Authentication):"
			" Emulated Asymmetric HWRoT\r\n");
		} else {
			*AHWRoT = XPLMI_RTCFG_SECURESTATE_NONSECURE;
		}
		Status = XST_SUCCESS;
	}

	return Status;
}
