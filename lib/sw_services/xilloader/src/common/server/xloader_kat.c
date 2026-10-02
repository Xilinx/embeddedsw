/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xloader_kat.c
*
* This file contains KAT related functions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- ---------- ----------------------------------------------------------------------------
* 2.4	rpu  04/15/2026 First release
* 2.5   rama 09/21/2026 Added XLoader_RsaKat, XLoader_EccKat, XLoader_AuthKat, and
*                       XLoader_LmsKat, consolidated from the platform-specific
*                       xloader_plat_secure.c files and from xloader_auth_enc.c
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xloader_kat_apis XilLoader KAT APIs
 * @{
 */

/******************************** Include Files ***************************************************/
#include "xplmi_tamper.h"
#include "xloader_auth_enc.h"
#include "xsecure_sha.h"
#include "xloader_kat.h"
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P)
#include "xsecure_plat_kat.h"
#endif
#include "xsecure_resourcehandling.h"
#ifndef PLM_RSA_EXCLUDE
#include "xsecure_error.h"
#endif
/************************** Constant Definitions **************************************************/

/**************************** Type Definitions ****************************************************/

/***************** Macros (Inline Functions) Definitions ******************************************/
/************************** Function Prototypes ***************************************************/

static void XLoader_ClearKatStatusOnCfg(XilPdi *PdiPtr, u32 PlmKatMask);

/************************** Variable Definitions **************************************************/

/**************************************************************************************************/
/**
* @brief    This function runs SHA3 KAT
*
* @param    PdiPtr is pointer to the xilpdi instance
*
* @return
* 	    - XST_SUCCESS on success
* 	    - XST_FAILURE if setting SHA3 data context lost fails
* 	    - XLOADER_ERR_KAT_FAILED on SHA3 KAT failure
*
***************************************************************************************************/
int XLoader_Sha3Kat(XilPdi *PdiPtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(PdiPtr, XPLMI_SECURE_SHA3_KAT_MASK);

	/** - Set the data context of previous SHA operation */
	Status = XSecure_SetDataContextLost(XPLMI_SHA3_CORE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if ((PdiPtr->PlmKatStatus & XPLMI_SECURE_SHA3_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for SHA3 if it is already run KAT will be run only when the
		 * CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_Sha3Kat, ShaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "SHA3 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		PdiPtr->PlmKatStatus |= XPLMI_SECURE_SHA3_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
 * @brief	This function updates the KAT status
 *
 * @param	PdiPtr is pointer to the XilPdi instance
 * @param	PlmKatMask is the mask of the KAT that is going to run
 *
 **************************************************************************************************/
void XLoader_ClearKatOnPPDI(XilPdi *PdiPtr, u32 PlmKatMask)
{
	if (PdiPtr->PdiType == XLOADER_PDI_TYPE_PARTIAL) {
		XLoader_ClearKatStatusOnCfg(PdiPtr, PlmKatMask);
	}
}

/**************************************************************************************************/
/**
 * @brief	This function masks the KAT status based on KatOnConfig value
 *			when PDI type is partial pdi
 *
 * @param	PdiPtr is pointer to the XilPdi instance
 * @param	PlmKatMask is the mask of the KAT that is going to run
 *
 **************************************************************************************************/
static void XLoader_ClearKatStatusOnCfg(XilPdi *PdiPtr, u32 PlmKatMask)
{
	u8 KatOnConfig = (u8)(XPlmi_In32(XPLMI_RTCFG_SECURE_CTRL_ADDR) &
						XLOADER_PPDI_KAT_MASK);

	if (KatOnConfig != 0U) {
		if (PlmKatMask != 0U) {
			PdiPtr->PpdiKatStatus &= PdiPtr->PlmKatStatus;
			if ((PdiPtr->PpdiKatStatus & PlmKatMask) != PlmKatMask) {
				/** - Update RTC area before running KAT */
				PdiPtr->PlmKatStatus &= ~PlmKatMask;
				XPlmi_UpdateKatStatus(PdiPtr->PlmKatStatus);
				PdiPtr->PpdiKatStatus |= PlmKatMask;
			}
		}
		else {
			PdiPtr->PpdiKatStatus = 0U;
		}
	}
}

#ifndef PLM_SECURE_EXCLUDE
/**************************************************************************************************/
/**
* @brief    This function performs KAT test on AES crypto Engine
*
* @param    SecurePtr Pointer to the XLoader_SecureParams instance.
*
* @return
* 	    - XST_SUCCESS on success
* 	    - XST_FAILURE if setting AES data context lost fails
* 	    - XLOADER_ERR_KAT_FAILED on AES DPACM or AES decrypt KAT failure
*
***************************************************************************************************/
int XLoader_AesKatTest(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	u32 DpacmEfuseStatus;
	u32 PlmDpacmKatStatus;

	/** - Update KAT status based on the user configuration */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SECURE_AES_CMKAT_MASK);

	/** - Set the data context of previous AES operation */
	Status = XSecure_SetDataContextLost(XPLMI_AES_CORE);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	/**
	 * - Skip running the KAT for AES DPACM or AES if it is already run KAT will be run only
	 * when the CRYPTO_KAT_EN bits in eFUSE are set
	 * If KAT fails device will go into a secure lockdown state
	 */
	DpacmEfuseStatus = XPlmi_In32(XLOADER_EFUSE_SEC_MISC1_OFFSET) & XLOADER_EFUSE_SEC_DPA_DIS_MASK;
	PlmDpacmKatStatus = SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SECURE_AES_CMKAT_MASK;

	if((DpacmEfuseStatus == 0U) && (PlmDpacmKatStatus == 0U)) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_AesDecryptCmKat, SecurePtr->AesInstPtr)
		if(Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			XPlmi_Printf(DEBUG_INFO, "DPACM KAT failed\n\r");
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SECURE_AES_CMKAT_MASK;

		/** - Update KAT status in RTC area */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}

	/** - Update KAT status based on the user configuration */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SECURE_AES_DEC_KAT_MASK);

	if((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SECURE_AES_DEC_KAT_MASK) == 0U) {
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_AesDecryptKat, SecurePtr->AesInstPtr);
		if(Status != XST_SUCCESS) {
			XPLMI_STATUS_GLITCH_DETECT(Status);
			XPlmi_Printf(DEBUG_INFO, "AES KAT failed\n\r");
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SECURE_AES_DEC_KAT_MASK;

		/** - Update KAT status in RTC area */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	XPlmi_Printf(DEBUG_INFO, "KAT test on AES crypto engine is successful\r\n");

	Status = XST_SUCCESS;

END:
	return Status;
}

#ifndef PLM_RSA_EXCLUDE
#if defined(VERSAL_2VE_2VM) || defined(VERSAL_NET)
/**************************************************************************************************/
/**
* @brief	This function runs the KAT for RSA PSS verification
*
* @return
* 		- XST_SUCCESS on success and error code on failure
*
***************************************************************************************************/
static int XLoader_RsaPssSignVeirfyKat(void) {
	XSecure_Rsa *RsaInstance = XSecure_GetRsaInstance();
	u32 *RsaModulus = XSecure_GetKatRsaModulus();
	u32 *RsaModExt = XSecure_GetKatRsaModExt();
	u32 RsaPubExp = XSECURE_KAT_RSA_PUB_EXP;
	u8 *MsgHash = XSecure_GetKatSha3ExpHash();
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	volatile int SStatus = XST_FAILURE;
	static u32 RsaPssSign[XSECURE_RSA_2048_SIZE_WORDS] = {
		0xD5B0D757U, 0xB402678AU, 0xD846E353U, 0xE615E8BDU,
		0x08000C3EU, 0x0D2BE166U, 0x8ADCA321U, 0xC0653B5BU,
		0xBC9CF06AU, 0x17232DA6U, 0x08124662U, 0xC58F1095U,
		0x9D3ABD1AU, 0x3EB117CBU, 0xE1B702AAU, 0x11267C5FU,
		0x74AEE03EU, 0x20AE3E4AU, 0x584CB9D0U, 0x201B42D7U,
		0xEE21C763U, 0x2E93CF7FU, 0x874C4496U, 0x94A3782EU,
		0x94B084C8U, 0x411698A1U, 0xE64E2B78U, 0x906CC80EU,
		0x8B76DD72U, 0x25F1E9C0U, 0xEDA7E18BU, 0x277C7905U,
		0xAD014078U, 0x0F4C0DB8U, 0x6A01665DU, 0x80BEE426U,
		0xF295C6A2U, 0xF3EFB25DU, 0x7255C0BAU, 0x41D3927BU,
		0x939251B3U, 0x49C59DD2U, 0x616F0890U, 0x230B31C0U,
		0x10A0EC9BU, 0x661EC873U, 0x132DD504U, 0xBDADB525U,
		0x4D7387D8U, 0x8C4E967AU, 0x681936A0U, 0xB4CE964FU,
		0xE632A6C2U, 0xD57C6294U, 0xAD3C1BD8U, 0xCE326E0DU,
		0x27CE58D3U, 0xE975377DU, 0x1F3007D2U, 0x27F80607U,
		0xF7E9821AU, 0x9A61ECCFU, 0xB1EA8EDCU, 0x989E39B0U
	};

	Status = XSecure_RsaInitialize(RsaInstance, (u8 *)RsaModulus,
		(u8 *)RsaModExt, (u8 *)&RsaPubExp);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	XSECURE_TEMPORAL_IMPL(Status, StatusTmp, XLoader_RsaPssSignVerify,
			(u8*)MsgHash, RsaInstance, (u8*)RsaPssSign, XSECURE_RSA_2048_KEY_SIZE);
	if ((Status != XST_SUCCESS) || (StatusTmp != XST_SUCCESS)) {
		Status = (int)XSECURE_RSA_KAT_PSS_SIGN_VER_ERROR;
		XPlmi_Printf(DEBUG_GENERAL,"RSA Pss sign verification KAT failed with status:%02x",
			Status);
	}
END:
	SStatus = Xil_SMemSet(RsaInstance, sizeof(XSecure_Rsa), 0U, sizeof(XSecure_Rsa));
	if (Status == XST_SUCCESS) {
		Status = SStatus;
	}

	return Status;
}
#endif

/**************************************************************************************************/
/**
* @brief	This function runs the KAT for RSA
*
* @return
* 		- XST_SUCCESS on success and error code on failure
*
***************************************************************************************************/
int XLoader_RsaKat(void) {
	int Status = XST_FAILURE;

#if defined(VERSAL_2VE_2VM) || defined(VERSAL_NET)
	Status = XLoader_RsaPssSignVeirfyKat();
#else
	Status = XSecure_RsaPublicEncryptKat();
#endif

	return Status;
}
#endif /* PLM_RSA_EXCLUDE */

#ifndef PLM_ECDSA_EXCLUDE
/**************************************************************************************************/
/**
* @brief	This function runs the KAT for ECDSA sign verification
*
* @param	CrvClass is the elliptic curve class (prime/binary) for which KAT is to be run
*
* @return
* 		- XST_SUCCESS on success and error code on failure
*
***************************************************************************************************/
int XLoader_EccKat(XSecure_EllipticCrvClass CrvClass)
{
	return XSecure_EllipticVerifySignKat(CrvClass);
}
#endif /* PLM_ECDSA_EXCLUDE */

/*****************************************************************************/
/**
* @brief	This function runs RSA or Ecdsa KAT based on pdi
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
* 			- XST_SUCCESS on success.
* 			- XLOADER_SEC_INVALID_AUTH on invalid authentication type.
* 			- XLOADER_ERR_RSA_NOT_ENABLED if RSA is not enabled.
* 			- XLOADER_ERR_ECDSA_NOT_ENABLED if ECDSA is not enabled.
*
******************************************************************************/
int XLoader_AuthKat(XLoader_SecureParams *SecurePtr) {
	u32 AuthType;
	u32 AuthKatMask;
	u32 AuthKatStatus = 0U;
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
#ifndef PLM_ECDSA_EXCLUDE
	XSecure_EllipticCrvClass CrvClass = XSECURE_ECC_PRIME;
#endif

	/** Get the Authentication type. */
#ifndef VERSAL_2VP_P
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		AuthType = XLoader_GetAuthPubAlgo(&(SecurePtr->AuthJtagMessagePtr->AuthHdr));
	}
	else {
		AuthType = XLoader_GetAuthPubAlgo(&SecurePtr->AcPtr->AuthHdr);
	}
#else
	if (SecurePtr->AuthJtagMessagePtr != NULL) {
		XPlmi_Printf(DEBUG_INFO, "Authenticated JTAG not supported\n\r");
		Status = XPlmi_UpdateStatus(XLOADER_ERR_AUTH_JTAG_NOT_SUPPORTED, XIL_SIGNED_ZERO);
		goto END;
	}

	AuthType = XLoader_GetAuthPubAlgo(SecurePtr->AcPtr->PPK.Header);
#endif

	if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
		AuthKatMask = XPLMI_SECURE_RSA_KAT_MASK;
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
		AuthKatMask = XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK;
#ifndef PLM_ECDSA_EXCLUDE
		CrvClass = XSECURE_ECC_PRIME;
#endif
	}
	else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
		AuthKatMask = XPLMI_SECURE_ECC_SIGN_VERIFY_SHA3_384_KAT_MASK;
#if !defined(PLM_ECDSA_EXCLUDE) && defined(XSECURE_ECC_SUPPORT_NIST_P521)
		CrvClass = XSECURE_ECC_PRIME;
#endif
	}
	else {
		/* Not supported */
		XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
		Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
		goto END;
	}

	if (SecurePtr->AuthJtagMessagePtr == NULL) {
		/** - Update KAT status based on the user configuration. */
		XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, AuthKatMask);
		AuthKatStatus = SecurePtr->PdiPtr->PlmKatStatus & AuthKatMask;
	}

	/**
	 * - Skip running the KAT for ECDSA or RSA if it is already run.
	 * KAT will be run only when the CYRPTO_KAT_EN bits in eFUSE are set.
	 * If KAT fails device will go into a secure lockdown state.
	 * For AUTH JTAG KAT will be run every time when request is made.
	 */
	if (AuthKatStatus == 0U) {
		if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
#ifndef PLM_RSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XLoader_RsaKat);
#else
			XPlmi_Printf(DEBUG_INFO, "RSA code is excluded\n\r");
			Status = XPlmi_UpdateStatus(XLOADER_ERR_RSA_NOT_ENABLED, XIL_SIGNED_ZERO);
			goto END;
#endif

		}
		else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P384) {
#ifndef PLM_ECDSA_EXCLUDE
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XLoader_EccKat, CrvClass);
#else
                        XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
                        Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
                        goto END;
#endif
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_ECDSA_P521) {
#ifndef PLM_ECDSA_EXCLUDE
#ifdef XSECURE_ECC_SUPPORT_NIST_P521
			XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
					XLoader_EccKat, CrvClass);
#else
			XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
                        Status = XLoader_UpdateMinorErr(XLOADER_SEC_CURVE_NOT_SUPPORTED, 0U);
                        goto END;
#endif
#else
                        XPlmi_Printf(DEBUG_INFO, "ECDSA code is excluded\n\r");
                        Status = XPlmi_UpdateStatus(XLOADER_ERR_ECDSA_NOT_ENABLED, XIL_SIGNED_ZERO);
                        goto END;
#endif
		}
		else {
			/* Not supported */
			XPlmi_Printf(DEBUG_INFO, "Authentication type is invalid\n\r");
			Status = XLoader_UpdateMinorErr(XLOADER_SEC_INVALID_AUTH, 0);
			goto END;
		}
		if (Status != XST_SUCCESS) {
			if (AuthType == XLOADER_PUB_STRENGTH_RSA_4096) {
				XPlmi_Printf(DEBUG_INFO, "RSA KAT Failed\n\r");
			}
			else {
				XPlmi_Printf(DEBUG_INFO, "ECDSA KAT Failed\n\r");
			}
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		if (SecurePtr->AuthJtagMessagePtr == NULL) {
			SecurePtr->PdiPtr->PlmKatStatus |= AuthKatMask;
			/* Update KAT status */
			XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
		}
	}
#if (defined(PLM_RSA_EXCLUDE) && defined(PLM_ECDSA_EXCLUDE))
	(void)StatusTmp;
#endif
	Status = XST_SUCCESS;
END:
	return Status;
}

#if(defined(VERSAL_2VE_2VM) || defined(VERSAL_2VP_P))
/**************************************************************************************************/
/**
* @brief	This function runs Shake 256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_KAT_FAILED on SHAKE 256 KAT failure
*
***************************************************************************************************/
int XLoader_Shake256Kat(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SHAKE_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SHAKE_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for SHAKE256 if it is already run KAT will be run only when
		 * the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_ShakeKat, ShaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "SHAKE 256 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SHAKE_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief	This function runs Sha2-256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_KAT_FAILED on SHA2-256 KAT failure
*
***************************************************************************************************/
int XLoader_Sha2256Kat(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_SHA2_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_SHA2_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for SHA2_256 if it is already run KAT will be run only when
		 * the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_Sha2256Kat, ShaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "SHA2 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_SHA2_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief	This function runs LMS HSS Sha2-256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_LMS_HSS_GET_DMA if DMA instance is NULL
*		- XLOADER_ERR_KAT_FAILED on HSS SHA2-256 KAT failure
*
***************************************************************************************************/
int XLoader_HssSha256Kat(XLoader_SecureParams *SecurePtr)
{
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	/** - Get DMA instance */
	if (PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_LMS_HSS_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_HSS_SHA2_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_HSS_SHA2_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for LMS_HSS_SHA2_256 if it is already run KAT will be run
		 * only when the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_HssSha2256Kat, ShaInstPtr, PmcDmaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "HSS SHA2-256 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_HSS_SHA2_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief	This function runs LMS HSS Shake256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_LMS_HSS_GET_DMA if DMA instance is NULL
*		- XLOADER_ERR_KAT_FAILED on HSS SHAKE256 KAT failure
*
***************************************************************************************************/
int XLoader_HssShake256Kat(XLoader_SecureParams *SecurePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	/** - Get DMA instance */
	if (PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_LMS_HSS_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_HSS_SHAKE_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_HSS_SHAKE_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for LMS_SHAKE_256 if it is already run KAT will be run only
		 * when the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_HssShake256Kat, ShaInstPtr, PmcDmaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "HSS SHAKE256 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_HSS_SHAKE_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief	This function runs LMS Sha2-256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_LMS_GET_DMA if DMA instance is NULL
*		- XLOADER_ERR_KAT_FAILED on LMS SHA2-256 KAT failure
*
***************************************************************************************************/
int XLoader_LmsSha2256Kat(XLoader_SecureParams *SecurePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha2Instance(XSECURE_SHA_1_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	/** - Get DMA instance */
	if (PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_LMS_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_LMS_SHA2_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_LMS_SHA2_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for LMS_SHA2_256 if it is already run KAT will be run only
		 * when the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_LmsSha2256Kat, ShaInstPtr, PmcDmaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "LMS SHA2-256 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_LMS_SHA2_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/**************************************************************************************************/
/**
* @brief	This function runs LMS Shake256 KAT
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
*
* @return
*		- XST_SUCCESS on success
*		- XLOADER_ERR_LMS_GET_DMA if DMA instance is NULL
*		- XLOADER_ERR_KAT_FAILED on LMS SHAKE256 KAT failure
*
***************************************************************************************************/
int XLoader_LmsShake256Kat(XLoader_SecureParams *SecurePtr) {
	volatile int Status = XST_FAILURE;
	volatile int StatusTmp = XST_FAILURE;
	XSecure_Sha *ShaInstPtr = XSecure_GetSha3Instance(XSECURE_SHA_0_DEVICE_ID);
	XPmcDma *PmcDmaInstPtr = XPlmi_GetDmaInstance(PMCDMA_0_DEVICE);

	/** - Get DMA instance */
	if (PmcDmaInstPtr == NULL) {
		Status = XPlmi_UpdateStatus(XLOADER_ERR_LMS_GET_DMA, XIL_SIGNED_ZERO);
		goto END;
	}

	/** - Update KAT status */
	XLoader_ClearKatOnPPDI(SecurePtr->PdiPtr, XPLMI_LMS_SHAKE_256_KAT_MASK);

	if ((SecurePtr->PdiPtr->PlmKatStatus & XPLMI_LMS_SHAKE_256_KAT_MASK) == 0U) {
		/**
		 * - Skip running the KAT for LMS_HSS_SHAKE_256 if it is already run KAT will be run
		 * only when the CRYPTO_KAT_EN bits in eFUSE are set
		 * If KAT fails device will go into a secure lockdown state
		 */
		XPLMI_HALT_BOOT_SLD_TEMPORAL_CHECK(XLOADER_ERR_KAT_FAILED, Status, StatusTmp,
			XSecure_LmsShake256Kat, ShaInstPtr, PmcDmaInstPtr);
		if(Status != XST_SUCCESS) {
			XPlmi_Printf(DEBUG_INFO, "LMS SHAKE256 KAT failed\n\r");
			XPLMI_STATUS_GLITCH_DETECT(Status);
			goto END;
		}
		SecurePtr->PdiPtr->PlmKatStatus |= XPLMI_LMS_SHAKE_256_KAT_MASK;

		/** - Update KAT status */
		XPlmi_UpdateKatStatus(SecurePtr->PdiPtr->PlmKatStatus);
	}
	Status = XST_SUCCESS;
END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This function runs LMS KAT's
*
* @param	SecurePtr is pointer to the XLoader_SecureParams instance
* @param	AuthType is public algorithm type
*
* @return
* 		- XST_SUCCESS on success.
* 		- XLOADER_ERR_LMS_HSS_GET_DMA if DMA instance acquire fails for HSS variant.
* 		- XLOADER_ERR_LMS_GET_DMA if DMA instance acquire fails for LMS variant.
* 		- XLOADER_SEC_KAT_FAILED_ERROR if any LMS KAT fails.
*
******************************************************************************/
int XLoader_LmsKat(XLoader_SecureParams *SecurePtr, u32 AuthType)
{
	int Status = XST_FAILURE;

	if (SecurePtr->SignHashAlgo == XSECURE_SHAKE_256) {
		/**
		 * - If LMS signature algorithm is SHAKE,
		 * run Shake256 KAT prior and then LMS_HSS or LMS KATs
		 */
		Status = XLoader_Shake256Kat(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
			Status = XLoader_HssShake256Kat(SecurePtr);
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
			Status = XLoader_LmsShake256Kat(SecurePtr);
		}
	}
	else if (SecurePtr->SignHashAlgo == XSECURE_SHA2_256) {
		/**
		 * - If LMS signature algorithm is SHA2-256,
		 * run Sha2-256 KAT prior and then LMS_HSS or LMS KATs
		 */
		Status = XLoader_Sha2256Kat(SecurePtr);
		if (Status != XST_SUCCESS) {
			goto END;
		}
		if (AuthType == XLOADER_PUB_STRENGTH_LMS_HSS) {
			Status = XLoader_HssSha256Kat(SecurePtr);
		}
		else if (AuthType == XLOADER_PUB_STRENGTH_LMS) {
			Status = XLoader_LmsSha2256Kat(SecurePtr);
		}
	}

END:
	return Status;
}

#endif /* VERSAL_2VE_2VM */
#endif /* PLM SECURE EXCLUDE */

#ifdef __cplusplus
}
#endif

/** @} end of xloader_kat_apis group */
