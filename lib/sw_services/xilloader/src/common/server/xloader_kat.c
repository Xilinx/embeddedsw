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

#endif /* VERSAL_2VE_2VM */
#endif /* PLM SECURE EXCLUDE */

#ifdef __cplusplus
}
#endif

/** @} end of xloader_kat_apis group */
