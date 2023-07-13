/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023, Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file versal_net/xloader_plat_secure.h
* @addtogroup xloader_apis XilLoader versal_net specific APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------   -------------------------------------------------------
* 1.00  bm   07/06/2022 Initial release
*       kpt  07/07/2022 Added support to update KAT status
*       dc   07/12/2022 Added support to recognize device state change
*       kpt  07/24/2022 Added XLoader_RsaKati
* 1.01  har  11/17/2022 Added XLoader_CheckSecureStateAuth
*       ng   11/23/2022 Fixed doxygen file name error
*       sk   03/17/2023 Renamed Kekstatus to DecKeyMask
*       dd   03/28/2023 Updated doxygen comments
*       sk   06/12/2023 Renamed XLoader_UpdateKekSrc to XLoader_GetKekSrc
*       am   06/19/2023 Added KAT error codes
* 1.9   kpt  07/13/2023 Added mask generation function
*
* </pre>
*
* @note
* @endcond
*
******************************************************************************/

#ifndef XLOADER_PLAT_SECURE_H
#define XLOADER_PLAT_SECURE_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xilpdi.h"
#include "xplmi_config.h"
#ifndef PLM_SECURE_EXCLUDE
#include "xsecure_kat.h"
#include "xloader.h"

/************************** Constant Definitions *****************************/

#define XLOADER_EFUSE_CACHE_JTAG_DIS_MASK		(0x00000004U) /**< eFUSE cache JTAG disable mask */
#define XLOADER_PMC_TAP_JTAG_STATUS_0			(0xF11A004CU) /**< PMC TAP JTAG status 0 */
#define XLOADER_PMC_TAP_JTAG_STATUS_DAP_STATUS_MASK	(0x00000001U << 23U) /**< DAP status mask */
#define XLOADER_JTAG_SEC_GATE_CLOSE			(0x00U) /**< JTAG secure gate close */
#define XLOADER_JTAG_SEC_GATE_OPEN			(0x01U) /**< JTAG secure gate open */

#define XLOADER_DEVICE_STATE_POLL_INTERVAL	(1000U) /**< Device state poll interval */
/**************************** Type Definitions *******************************/
typedef enum {
	/* Add platform specific error codes from 0xA0 */
	XLOADER_SEC_AUTH_EN_PPK_HASH_NONZERO = 0x02,
			/**< 0x02 Incorrect Authentication type selected */
	XLOADER_SEC_PPK_HASH_CALCULATION_FAIL,
			/**< 0x03 PPK Hash calculation failed */
	XLOADER_SEC_ALL_PPK_REVOKED_ERR,
			/**< 0x04 All PPKs are revoked */
	XLOADER_SEC_PPK_INVALID_BIT_ERR,
			/**< 0x05 PPK Invalid bit is set */
	XLOADER_SEC_PPK_HASH_ALLZERO_INVLD,
			/**< 0x06 PPK HAsh is all zero hence inavalid */
	XLOADER_SEC_PPK_HASH_COMPARE_FAIL,
			/**< 0x07 HAsh comparison failed */
	XLOADER_SEC_ALL_PPK_INVALID_ERR,
			/**< 0x08 All PPKs are invalid */
	XLOADER_SEC_SPK_HASH_CALCULATION_FAIL,
			/**< 0x09 SPK HAsh calculation failed */
	XLOADER_SEC_RSA_AUTH_FAIL,
			/**< 0x0A RSA signature is not verified */
	XLOADER_SEC_RSA_PSS_SIGN_VERIFY_FAIL,
			/**< 0x0B RSA Pss signature verification failed */
	XLOADER_SEC_ECDSA_AUTH_FAIL,
			/**< 0x0C ECDSA signature is not verified */
	XLOADER_SEC_ECDSA_INVLD_KEY_COORDINATES,
			/**< 0x0D ECDSA invalid key coordinates */
	XLOADER_SEC_INVALID_AUTH,
			/**< 0x0E Only RSA and ECDSA are supported */
	XLOADER_SEC_REVOCATION_ID_OUTOFRANGE_ERR = 0x10,
			/**< 0x10 Revocation ID is out of range */
	XLOADER_SEC_ID_REVOKED,
			/**< 0x11 Revocation ID range not verified */
	XLOADER_SEC_BLACK_KEY_DEC_ERR,
			/**< 0x12 Black key decryption error */
	XLOADER_SEC_OBFUS_KEY_DEC_ERR,
			/**< 0x13 Obfuscated key decryption error */
	XLOADER_SEC_DEC_INVALID_KEYSRC_SEL,
			/**< 0x14 Invalid key source selected for decryption */
	XLOADER_SEC_DATA_LEFT_FOR_DECRYPT_ERR,
			/**< 0x15 Data still remaining for decryption */
	XLOADER_SEC_DECRYPT_REM_DATA_SIZE_MISMATCH,
			/**< 0x16 Size mismatch for data remaining for
				 decryption */
	XLOADER_SEC_AES_OPERATION_FAILED,
			/**< 0x17 AES Operation failed */
	XLOADER_SEC_DPA_CM_ERR,
			/**< 0x18 DPA CM Cfg Error */
	XLOADER_SEC_PUF_REGN_ERRR,
			/**< 0x19 PUF regeneration error */
	XLOADER_SEC_AES_KEK_DEC,
			/**< 0x1A AES KEK decryption */
	XLOADER_SEC_RSA_PSS_ENC_BC_VALUE_NOT_MATCHED,
			/**< 0x1B RSA ENC 0xbc value is not matched */
	XLOADER_SEC_RSA_PSS_HASH_COMPARE_FAILURE,
			/**< 0x1C RSA PSS verification hash is not matched */
	XLOADER_SEC_ENC_ONLY_KEYSRC_ERR,
	        /**< 0x1D Keysrc should be efuse black key for enc only */
	XLOADER_SEC_ENC_ONLY_PUFHD_LOC_ERR,
			/**< 0x1E PUFHD location should be from eFuse for enc only */
	XLOADER_SEC_METAHDR_IV_ZERO_ERR,
	        /**< 0x1F eFuse IV should be non-zero for enc only */
	XLOADER_SEC_BLACK_IV_ZERO_ERR,
			 /**< 0x20 eFuse IV should be non-zero for enc only */
	XLOADER_SEC_IV_METAHDR_RANGE_ERROR,
		   /**< 0x21 Metahdr IV Range not matched with eFuse IV */
	XLOADER_SEC_EFUSE_DPA_CM_MISMATCH_ERROR,
		/**< 0x22 Metahdr DpaCm & eFuse DpaCm values are not matched */
	XLOADER_SEC_RSA_MEMSET_SHA3_ARRAY_FAIL,
		/**< 0x23 Error during memset for XSecure_RsaSha3Array */
	XLOADER_SEC_RSA_MEMSET_VARSCOM_FAIL,
		/**< 0x24 Error during memset for Xsecure_Varsocm */
	XLOADER_SEC_MASKED_DB_MSB_ERROR,
		/**< 0x25 Error in RSA EM MSB */
	XLOADER_SEC_EFUSE_DB_PATTERN_MISMATCH_ERROR,
		/**< 0x26 Failed to verify DB check */
	XLOADER_SEC_MEMSET_ERROR,
		/**< 0x27 Error during XPlmi_MemSetBytes */
	XLOADER_SEC_GLITCH_DETECTED_ERROR,
		/**<0x28 Error glitch detected */
	XLOADER_SEC_ENC_DATA_NOT_ALIGNED_ERROR,
		/**<0x29 Error encrypted data is not 128 bit aligned */
	XLOADER_SEC_KAT_FAILED_ERROR,
		/**<0x30 Secure KAT failed error */
} XLoader_SecErrCodes;


/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XLoader_GetKekSrc(void);
int XLoader_AesObfusKeySelect(u32 PdiKeySrc, u32 DecKeyMask, void *KeySrcPtr);
int XLoader_AddDeviceStateChangeToScheduler(void);
int XLoader_CheckDeviceStateChange(void *Arg);
#ifndef PLM_RSA_EXCLUDE
int XLoader_RsaKat(XPmcDma *PmcDmaPtr);
int XLoader_MaskGenFunc(XSecure_Sha3 *Sha3InstancePtr,
	u8 * Out, u32 OutLen, u8 *Input);
#endif

/************************** Variable Definitions *****************************/

#endif /* END OF PLM_SECURE_EXCLUDE */
#ifdef __cplusplus
}
#endif

#endif  /* XLOADER_PLAT_SECURE_H */
