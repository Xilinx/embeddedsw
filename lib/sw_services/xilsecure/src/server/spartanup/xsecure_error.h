/******************************************************************************
* Copyright (c) 2024-2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xsecure_error.h
 *
 * This file contains xilsecure error codes
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 5.5   kpt  08/18/2024 Initial release
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XSECURE_ERROR_H_
#define XSECURE_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
typedef enum {
	XSECURE_SHA_INIT_ERROR = 0x02,		/**< 0x02 - Error when SHA
						   init fails. */
	XSECURE_SHA_LAST_UPDATE_ERROR,		/**< 0x03 - Error when SHA
						   last update fails. */
	XSECURE_SHA_PMC_DMA_UPDATE_ERROR,	/**< 0x04 - Error when DMA driver
						   fails to update the data to SHA */
	XSECURE_SHA_TIMEOUT_ERROR,		/**< 0x05 - Error when timeout */

	XSECURE_SHA_KAT_FAILED_ERROR,		/**< 0x06 - Error when SHA hash
						   not matched with expected hash */
	XSECURE_SHA_FINISH_ERROR,		/**<0x07 - Error when SHA finish fails */

	XSECURE_SHA_INVALID_PARAM,		/**< 0x08 - Invalid Argument */

	XSECURE_SSS_INVALID_PARAM,		/**< 0x09 - Invalid Argument */

	XSECURE_SHA_STATE_MISMATCH_ERROR,	/**< 0x0A - State mismatch */

	XSECURE_SHA384_INVALID_PARAM,		/**< 0x0B - Invalid Param for SHA384 digest calculation*/

	XSECURE_SHA384_KAT_ERROR,		/**< 0x0C - Error when SHA384 KAT fails */

	XSECURE_AES_GCM_TAG_MISMATCH = 0x40,	/**< 0x40 - user provided GCM tag does
						   not match calculated tag */
	XSECURE_AES_KEY_CLEAR_ERROR,
						/**< 0x41 - AES key clear error */
	XSECURE_AES_DPA_CM_NOT_SUPPORTED,	/**< 0x42 - AES DPA CM is not
						   supported on device */
	XSECURE_AES_INVALID_PARAM,		     /**< 0x43 - Invalid Argument */
	XSECURE_AES_STATE_MISMATCH_ERROR,	     /**< 0x44 - State mismatch */
	XSECURE_AES_DEVICE_KEY_NOT_ALLOWED,	     /**< 0x45 - Access to device keys
							is not allowed for IPI for
							any operations */
	XSECURE_AES_ZERO_PUF_KEY_NOT_ALLOWED,	/**< 0x46 - Error when PUF Key is selected as key source and PUF key is zeroized */
	XSECURE_AES_UNALIGNED_SIZE_ERROR,      /**< 0x47 - Error when data is unaligned*/
	XSECURE_AES_INVALID_MODE,             /**< 0x48 - Error when invalid mode is used for AES operation */
	XSECURE_AES_KAT_DATA_MISMATCH_ERROR,	/**< 0x46 - Error when AES data
                                                   not matched with
						   expected data  */
	XSECURE_AES_KAT_FAILED_ERROR,		/**< 0x47 - AES KAT fails  */

	XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x48 - Error when AESDPACM
							key write fails. */
	XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR,   /**< 0x49 - Error when AESDPACM
                                                        key load fails. */
	XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR,       /**< 0x4A - Error when AESDPACM
							sss config fails */
	XSECURE_AESDPACM_KAT_FAILED_ERROR,  /**< 0x4B - Error when AESDPACM
							KAT failed */
	XSECURE_AESKAT_INVALID_PARAM		     /**< 0x4C - Invalid Argument */
} XSecure_ErrorCodes;

#define XSECURE_SHA3_KAT_FAILED_ERROR		XSECURE_SHA_KAT_FAILED_ERROR

#define XSECURE_SHA3_INVALID_PARAM		XSECURE_SHA_INVALID_PARAM

#define XSECURE_SHA3_PMC_DMA_UPDATE_ERROR	XSECURE_SHA_PMC_DMA_UPDATE_ERROR

#define XSECURE_SHA3_FINISH_ERROR		XSECURE_SHA_FINISH_ERROR
/**
 * @}
*/
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ERROR_H_ */
