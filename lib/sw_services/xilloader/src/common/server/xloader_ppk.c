/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xloader_ppk.c
*
* This file contains the code related to PPK verification.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0  kal  06/25/2026 Initial release
*
* </pre>
*
******************************************************************************/

/**
 * @addtogroup xloader_server_apis XilLoader Server APIs
 * @{
 */

/***************************** Include Files *********************************/
#include "xplmi_config.h"
#include "xloader_secure.h"
#include "xloader_auth_enc.h"
#include "xilpdi.h"
#include "xplmi.h"
#include "xplmi_status.h"
#include "xloader_plat_secure.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

#ifndef PLM_SECURE_EXCLUDE

#ifdef PLM_EN_ADD_PPKS
/*****************************************************************************/
/**
* @brief	This function reads Additional PPKs Enable bits.
*
* @return
*		- XST_SUCCESS on success.
*		- XLOADER_EFUSE_5_PPK_FEATURE_NOT_SUPPORTED if additional PPKs are
*		  not enabled.
*
******************************************************************************/
static int XLoader_IsAdditionalPpkFeatureEnabled(void)
{
	int Status = XST_FAILURE;
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;

	ReadReg = Xil_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	ReadRegTmp = Xil_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);

	ReadReg = ReadReg & XLOADER_EFUSE_ADDITIONAL_PPK_ENABLE_BITS_MASK;
	ReadRegTmp = ReadRegTmp & XLOADER_EFUSE_ADDITIONAL_PPK_ENABLE_BITS_MASK;

	if ((ReadReg == 0x0U) || (ReadRegTmp == 0x0U)) {
		Status = (int)XLOADER_EFUSE_5_PPK_FEATURE_NOT_SUPPORTED;
		goto END;
	}
	Status = XST_SUCCESS;
END:
	return Status;
}
#endif /* PLM_EN_ADD_PPKS */

/*****************************************************************************/
/**
* @brief	This function calculates the invalid mask and PPK offset based on
*		the selected PPK.
*
*		PPK 0/1/2 are supported on all platforms.  Extended PPKs (3+)
*		are platform-specific:
*		  - VERSAL_2VP_P (standalone): PPK 3-8 always available.
*		  - versal / versal_2vp with PLM_EN_ADD_PPKS: PPK 3-4 optional,
*		    gated by XLoader_IsAdditionalPpkFeatureEnabled.
*		  - versal_net / versal_2ve_2vm / versal_gen2_tr: PPK 0-2 only.
*
* @param	PpkSelect is PPK selection of eFUSE.
* @param	InvalidMask is pointer to store the invalid mask for the
*		selected PPK.
* @param	PpkOffset is pointer to store the PPK eFUSE offset.
*
* @return
*		- XST_SUCCESS on success.
*		- XLOADER_SEC_INVALID_PPK_CHOICE_ERR if invalid PPK selection.
*
******************************************************************************/
static int XLoader_GetPpkInvalidMaskAndOffset(u32 PpkSelect,
		u32 *InvalidMask, u32 *PpkOffset)
{
	volatile int Status = XST_FAILURE;

	switch ((u32)PpkSelect) {
		case XLOADER_PPK_SEL_0:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK0_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK0_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_1:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK1_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK1_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_2:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK2_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK2_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#if defined(VERSAL_2VP_P) && !defined(VERSAL_2VE_2VM)
		/** - VERSAL_2VP_P standalone: PPK 3-8 always enabled */
		case XLOADER_PPK_SEL_3:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK3_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK3_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_4:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK4_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK4_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_5:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK5_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK5_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_6:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK6_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK6_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_7:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK7_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK7_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_8:
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK8_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK8_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#elif defined(PLM_EN_ADD_PPKS)
		/** - versal / versal_2vp: optional PPK 3-4 gated by feature enable */
		case XLOADER_PPK_SEL_3:
			XSECURE_TEMPORAL_CHECK(END, Status, XLoader_IsAdditionalPpkFeatureEnabled);
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK3_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK3_START_OFFSET;
			Status = XST_SUCCESS;
			break;
		case XLOADER_PPK_SEL_4:
			XSECURE_TEMPORAL_CHECK(END, Status, XLoader_IsAdditionalPpkFeatureEnabled);
			*InvalidMask = XLOADER_EFUSE_MISC_CTRL_PPK4_INVLD;
			*PpkOffset = XLOADER_EFUSE_PPK4_START_OFFSET;
			Status = XST_SUCCESS;
			break;
#endif
		default:
			/** Unintended condition */
			XPLMI_STATUS_GLITCH_DETECT(Status);
			break;
	}

#ifdef PLM_EN_ADD_PPKS
END:
#endif
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function compares a calculated PPK hash with the eFUSE
*		stored PPK hash using a constant-time comparison.
*
* @param	EfusePpkOffset is the eFUSE PPK hash start address.
* @param	PpkHash is pointer to the PPK hash to compare.
*
* @return
*		- XST_SUCCESS on success.
*		- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if hash comparison failed.
*
******************************************************************************/
static int XLoader_PpkCompare(const u32 EfusePpkOffset, const u8 *PpkHash)
{
	int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;

	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, PpkHash,
			XLOADER_EFUSE_PPK_HASH_LEN, (void *)EfusePpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN, XLOADER_EFUSE_PPK_HASH_LEN);

	if ((HashStatus != XST_SUCCESS) || (HashStatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Error: PPK Hash comparison failed\r\n");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_COMPARE_FAIL, 0x0);
	}
	else {
		Status = XST_SUCCESS;
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function compares the provided PPK hash with the hash
*		stored in eFUSE and validates it is not all zeros.
*
* @param	PpkHash is pointer to the PPK hash to be verified.
* @param	PpkOffset is PPK eFUSE offset.
*
* @return
*		- XST_SUCCESS on success.
*		- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if hash comparison fails.
*		- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
*
******************************************************************************/
static int XLoader_ValidatePpkHash(const u8 *PpkHash, u32 PpkOffset)
{
	volatile int Status = XST_FAILURE;
	volatile int HashStatus = XST_FAILURE;
	volatile int HashStatusTmp = XST_FAILURE;
	const u8 HashZeros[XLOADER_EFUSE_PPK_HASH_LEN] = {0U};
#ifdef VERSAL_2VE_2VM
	u32 UserOffset = 0U;
#endif

	/** - Compare provided PPK hash with eFUSE stored hash */
	XSECURE_TEMPORAL_CHECK(END, Status, XLoader_PpkCompare, PpkOffset, PpkHash);

	Status = XST_FAILURE;
	/** - Check if PPK hash is all zeros */
	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, HashZeros,
			XLOADER_EFUSE_PPK_HASH_LEN, (void *)PpkOffset,
			XLOADER_EFUSE_PPK_HASH_LEN, XLOADER_EFUSE_PPK_HASH_LEN);
	if ((HashStatus == XST_SUCCESS) || (HashStatusTmp == XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0);
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}

#ifdef VERSAL_2VE_2VM
	if (PpkOffset == XLOADER_EFUSE_PPK0_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK0_USER_START_OFFSET;
	}
	else if (PpkOffset == XLOADER_EFUSE_PPK1_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK1_USER_START_OFFSET;
	}
	else if (PpkOffset == XLOADER_EFUSE_PPK2_START_OFFSET) {
		UserOffset = XLOADER_EFUSE_PPK2_USER_START_OFFSET;
	}
	else {
		Status = XST_FAILURE;
		goto END;
	}

	HashStatus = XST_FAILURE;
	/** - Check if upper 128 PPK bits hash is valid or not by reading the User EFUSE bits */
	HashStatus = Xil_SMemCmp_CT((void *)(PpkHash + XLOADER_EFUSE_PPK_HASH_LEN),
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN,
			(void *)UserOffset,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN);
	HashStatusTmp = HashStatus;
	if ((HashStatus != XST_SUCCESS) || (HashStatusTmp != XST_SUCCESS)) {
		XPlmi_Printf(DEBUG_INFO, "Error: PPK Hash - Upper 128 bits comparison failed\r\n");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_COMPARE_FAIL, 0x0);
		goto END;
	}

	Status = XST_FAILURE;
	/** - Check if valid upper 128 bit PPK hash is all zeros */
	XSECURE_TEMPORAL_IMPL(HashStatus, HashStatusTmp, Xil_SMemCmp_CT, HashZeros,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN, (void *)UserOffset,
			XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN, XLOADER_EFUSE_PPK_HASH_HIGH_BYTE_LEN);
	if ((HashStatus == XST_SUCCESS) || (HashStatusTmp == XST_SUCCESS)) {
		Status = XLoader_UpdateMinorErr(
			XLOADER_SEC_PPK_HASH_ALLZERO_INVLD, 0x0);
		goto END;
	}
	else {
		Status = XST_SUCCESS;
	}
#endif

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function checks if the selected PPK is revoked by reading
*		the PPK invalid bits from eFUSE.
*
* @param	PpkInvldMask is the PPK invalid bit mask to check in eFUSE.
*
* @return
*		- XST_SUCCESS if PPK is not revoked.
*		- XLOADER_SEC_SEL_PPK_REVOKED_ERR if PPK is revoked (invalid bit set).
*		- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
*
******************************************************************************/
static int XLoader_IsPpkRevoked(u32 PpkInvldMask)
{
	volatile int Status = XST_FAILURE;
	volatile u32 ReadReg;
	volatile u32 ReadRegTmp;

	/** - Check if all PPKs are revoked */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET);
	if ((ReadReg & XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD) ==
		(XLOADER_EFUSE_MISC_CTRL_ALL_PPK_INVLD)) {
		XPlmi_Printf(DEBUG_INFO, "All PPKs are invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_ALL_PPK_REVOKED_ERR, 0x0);
		goto END;
	}

	/** - Check if PPK is revoked by reading the PPK EFUSE invalid bits */
	ReadReg = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & PpkInvldMask;
	ReadRegTmp = XPlmi_In32(XLOADER_EFUSE_MISC_CTRL_OFFSET) & PpkInvldMask;
	if ((ReadReg != 0x0U) || (ReadRegTmp != 0x0U)) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_SEL_PPK_REVOKED_ERR, 0x0);
	}
	else {
		Status = XST_SUCCESS;
	}

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function validates PPK hash against eFUSE stored hash.
*		It iterates through all PPKs and checks if the provided hash
*		matches any valid (non-revoked) PPK.
*
* @param	PpkHash  Pointer to the PPK hash to be verified.
*
* @return
*		- XST_SUCCESS on success.
*		- XLOADER_SEC_INVALID_PPK_CHOICE_ERR if invalid PPK selection.
*		- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if PPK hash comparison failed.
*		- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
*		- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
*		- XLOADER_SEC_SEL_PPK_REVOKED_ERR if selected PPK is revoked.
*
******************************************************************************/
static int XLoader_IsPpkValid(const u8 *PpkHash)
{
	volatile int Status = XST_FAILURE;
	u32 PpkOffset = 0U;
	volatile u32 PpkIndex;
	u32 InvalidMask = 0U;

	/** - Validate PPK hash for all PPKs in a loop */
	for (PpkIndex = XLOADER_PPK_SEL_0; PpkIndex < XLOADER_PPK_SEL_MAX; PpkIndex++) {

		/** - Get invalid mask and PPK offset */
		Status = XLoader_GetPpkInvalidMaskAndOffset(PpkIndex, &InvalidMask, &PpkOffset);
		if (Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}

		Status = XST_FAILURE;

		/** - Compare PPK hash with efuse stored PPK hash */
		Status = XLoader_ValidatePpkHash(PpkHash, PpkOffset);
		if (Status == XST_SUCCESS) {
			break;
		}
	}

	if (Status != XST_SUCCESS) {
		if (PpkIndex != XLOADER_PPK_SEL_MAX) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
		}
		else {
			XPlmi_Printf(DEBUG_INFO, "Error: PPK validation failed for all PPKs\r\n");
			goto END;
		}
	}
	else {
		/** - Check if corresponding PPK is revoked*/
		Status = XLoader_IsPpkRevoked(InvalidMask);
	}

END:
	return Status;
}

#if !(defined(VERSAL_2VE_2VM) && defined(VERSAL_2VP_P))
/*****************************************************************************/
/**
* @brief	This function verifies PPK by calculating the SHA3-384 digest of
*		    the PPK and comparing it to the eFuse-stored PPK hashes via
*		    XLoader_IsPpkValid.
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance.
* @param	PpkSize   is size of the PPK in bytes.
*
* @return
*			- XST_SUCCESS on success.
*			- XLOADER_SEC_INVALID_PPK_CHOICE_ERR if invalid PPK selection.
*			- XLOADER_SEC_PPK_HASH_CALCULATION_FAIL if PPK hash calculation failed.
*			- XLOADER_SEC_PPK_HASH_COMPARE_FAIL if PPK hash comparison failed.
*			- XLOADER_SEC_PPK_HASH_ALLZERO_INVLD if PPK hash is all zeros.
*			- XLOADER_SEC_ALL_PPK_REVOKED_ERR if all PPKs are revoked.
*			- XLOADER_SEC_SEL_PPK_REVOKED_ERR if selected PPK is revoked.
*
******************************************************************************/
int XLoader_PpkVerify(const XLoader_SecureParams *SecurePtr, const u32 PpkSize)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int ClearStatus = XST_FAILURE;
	XSecure_Sha384Hash Sha3Hash;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

#if defined(VERSAL_2VP_P)
	/** - VERSAL_2VP_P standalone: Authenticated JTAG is not supported */
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		XPlmi_Printf(DEBUG_INFO, "Authenticated JTAG not supported\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_NOT_SUPPORTED,
			XIL_SIGNED_ZERO);
		goto END;
	}
#endif

	Status = XSecure_ShaStart(ShaInstPtr, XSECURE_SHA3_384);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}
	Status = XSecure_ShaLastUpdate(ShaInstPtr);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

#if defined(VERSAL_2VP_P)
	/** - VERSAL_2VP_P standalone: PPK is in the newer PPK.Header field of the AC */
	Status = XSecure_ShaUpdate(ShaInstPtr,
		(UINTPTR)&SecurePtr->AcPtr->PPK.Header, PpkSize);
#else
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
#if defined(VERSAL_2VE_2VM) && !defined(VERSAL_2VP_P)
		/** - VERSAL_2VE_2VM standalone: JTAG message uses AuthJtagData field */
		Status = XSecure_ShaUpdate(ShaInstPtr,
			(UINTPTR)&(SecurePtr->AuthJtagMessagePtr->AuthJtagData), PpkSize);
#else
		/** - versal / versal_net / versal_2vp: JTAG message uses PpkData field */
		Status = XSecure_ShaUpdate(ShaInstPtr,
			(UINTPTR)&(SecurePtr->AuthJtagMessagePtr->PpkData), PpkSize);
#endif
	}
	else {
		Status = XSecure_ShaUpdate(ShaInstPtr,
			(UINTPTR)&SecurePtr->AcPtr->Ppk, PpkSize);
	}
#endif
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	Status = XSecure_ShaFinish(ShaInstPtr, (u64)(UINTPTR)&Sha3Hash,
		XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (Status != XST_SUCCESS) {
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_PPK_HASH_CALCULATION_FAIL, Status);
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_IsPpkValid, Sha3Hash.Hash);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		XPLMI_STATUS_GLITCH_DETECT(Status);
		goto END;
	}

END:
	ClearStatus = XPlmi_MemSetBytes(&Sha3Hash, XSECURE_SHA_384_HASH_SIZE_IN_BYTES,
		0U, XSECURE_SHA_384_HASH_SIZE_IN_BYTES);
	if (ClearStatus != XST_SUCCESS) {
		Status = (int)((u32)Status | XLOADER_SEC_BUF_CLEAR_ERR);
	}
	return Status;
}

#endif /* !(VERSAL_2VE_2VM && VERSAL_2VP_P) */

#endif /* PLM_SECURE_EXCLUDE */

/** @} end of xloader_server_apis group */
