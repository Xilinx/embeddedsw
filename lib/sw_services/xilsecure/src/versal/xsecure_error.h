/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
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
 * 1.0   rpo  03/19/2020 Initial release
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
/**
 * @addtogroup xilsecure_versal_error_codes List of Error Codes
 * @{
 */
typedef enum {
	XSECURE_SHA3_INIT_ERROR = 0x02U,      /**< 0x02 - Error when SHA3
                                            init fails. */
	XSECURE_SHA3_LAST_UPDATE_ERROR,    /**< 0x03 - Error when SHA3
                                            last update fails. */
	XSECURE_SHA3_PMC_DMA_UPDATE_ERROR, /**< 0x04 - Error when DMA driver
                                            fails to update the data to SHA3 */
	XSECURE_SHA3_TIMEOUT_ERROR, /**< 0x05 - Error when timeout */

	XSECURE_SHA3_KAT_FAILED_ERROR,  /**< 0x06 - Error when SHA3 hash
                                         not matched with expected hash */

	XSECURE_AES_GCM_TAG_MISMATCH = 0x40U,
					/**< 0x40 - user provided GCM tag does
						not match calculated tag */
	XSECURE_AES_KEY_CLEAR_ERROR,
					/**< 0x41 - AES key clear error */
	XSECURE_AES_DPA_CM_NOT_SUPPORTED,
					/**< 0x42 - AES DPA CM is not supported on device */
	XSECURE_AES_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x43 - Error when AES key write
                                                  fails. */
	XSECURE_AES_KAT_DECRYPT_INIT_FAILED_ERROR, /**< 0x44 - Error when AES
					             decrypt init fails. */
	XSECURE_AES_KAT_GCM_TAG_MISMATCH_ERROR, /**< 0x45 - Error when GCM tag
                                                  not matched with
						  user provided tag */
	XSECURE_AES_KAT_DATA_MISMATCH_ERROR,  /**< 0x46 - Error when AES data
                                                   not matched with
						   expected data  */
	XSECURE_AES_KAT_FAILED_ERROR,         /**< 0x47 - AES KAT Failes  */

	XSECURE_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x48 - Error when AESDPACM
							key write fails. */
	XSECURE_AESDPACM_KAT_KEYLOAD_FAILED_ERROR,   /**< 0x49 - Error when AESDPACM
                                                        key load fails. */
	XSECURE_AESDPACM_SSS_CFG_FAILED_ERROR,       /**< 0x4A - Eroor ehen AESDPACM
							sss config fails */
	XSECURE_AESDPACM_KAT_FAILED_ERROR,           /**< 0x4B - AESDPACM KAT Failes*/
	XSECURE_AESDPACM_KAT_CHECK1_FAILED_ERROR,    /**< 0x4C - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK2_FAILED_ERROR,    /**< 0x4D - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK3_FAILED_ERROR,    /**< 0x4E - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK4_FAILED_ERROR,    /**< 0x4F - Error when AESDPACM
							data not matched with
							expected data  */
	XSECURE_AESDPACM_KAT_CHECK5_FAILED_ERROR,     /**< 0x50 - Error when AESDPACM
							data not matched with
							expected data  */

	XSECURE_RSA_KAT_ENCRYPT_FAILED_ERROR = 0x80U, /**< 0x80 - RSA KAT Failes  */

	XSECURE_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR, /**< 0x81 - Error when RSA data
                                                       not matched with
						       expected data  */
	XSECURE_ECC_KAT_KEY_NOTVALID_ERROR = 0xC0U, /**< 0xC0 -ECC key is not valid */

	XSECURE_ECC_KAT_FAILED_ERROR,   /**< 0xC1 - ECC KAT Failes */

} XSecure_ErrorCodes;
/**
 * @}
*/
/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_ERROR_H_ */
