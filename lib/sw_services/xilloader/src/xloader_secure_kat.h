/******************************************************************************
 * Copyright (C) 2020 Xilinx, Inc. All rights reserved.
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
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 *
 ******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xloader_secure_kat.h
 *
 * This file contains KAT interface APIs
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- ---------- -------------------------------------------------------
 * 1.0   rpo  02/25/2020 Initial release
 * </pre>
 *
 * @note
 *
 ******************************************************************************/
#ifndef XLOADER_SECURE_KAT_H_
#define XLOADER_SECURE_KAT_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsecure_sha.h"
#include "xsecure_aes.h"
#include "xsecure_ecdsa.h"
#include "xsecure_utils.h"
#include "xsecure_rsa_core.h"
#include "xsecure_rsa.h"
#include "xsecure_ecdsa_rsa_hw.h"
#include "xloader_secure.h"
#include "xplmi_dma.h"
#include "xloader.h"
/************************** Constant Definitions *****************************/
#define XLOADER_HASH_SIZE_IN_BYTES			    48U
#define XLOADER_SHA3_CHUNK_SIZE  			    104U
#define XLOADER_RSA_DATA_SIZE_WORDS			    128U
#define XLOADER_ECC_DATA_SIZE_WORDS			    12U

#define XLOADER_PMC_GLOBAL_SSS_CONFIG                       0xF1110500U

#define XLOADER_SHA3_KAT_MASK                               0x00000010U
#define XLOADER_RSA_KAT_MASK                                0x00000020U
#define XLOADER_ECDSA_KAT_MASK                              0x00000040U
#define XLOADER_AES_KAT_MASK                                0x00000080U
#define XLOADER_DPACM_KAT_MASK                              0x00000100U
#define XLOADER_SHA3_RESET                                  0xF1210004U
#define XLOADER_ECDSA_RSA_RESET                             0xF1200040U

/**************************** Type Definitions *******************************/
typedef enum {
	XLOADER_SHA3_INIT_ERROR = 2U,      /**< 0x2 - Error when SHA3
                                            init fails. */
	XLOADER_SHA3_LAST_UPDATE_ERROR,    /**< 0x3 - Error when SHA3
                                            last update fails. */
	XLOADER_SHA3_PMC_DMA_UPDATE_ERROR, /**< 0x4 - Error when DMA driver
                                            fails to update the data to SHA3 */
	XLOADER_SHA3_KAT_FAILED_ERROR,  /**< 0x5 - Error when SHA3 hash
                                         not matched with expected hash */

	XLOADER_AES_KAT_WRITE_KEY_FAILED_ERROR, /**< 0x6 - Error when AES key write
                                                  fails. */
	XLOADER_AES_KAT_DECRYPT_INIT_FAILED_ERROR, /**< 0x7 - Error when AES
					             decrypt init fails. */
	XLOADER_AES_KAT_GCM_TAG_MISMATCH_ERROR, /**< 0x8 - Error when GCM tag
                                                  not matched with
						  user provided tag */
	XLOADER_AES_KAT_DATA_MISMATCH_ERROR,  /**< 0x9 - Error when AES data
                                                   not matched with
						   expected data  */
	XLOADER_AES_KAT_FAILED_ERROR,         /**< 0xA - AES KAT Failes  */

	XLOADER_RSA_KAT_ENCRYPT_FAILED_ERROR, /**< 0xB - RSA KAT Failes  */
	XLOADER_RSA_KAT_ENCRYPT_DATA_MISMATCH_ERROR, /**< 0xC - Error when RSA data
                                                       not matched with
						       expected data  */
	XLOADER_ECC_KAT_KEY_NOTVALID_ERROR,   /**< 0xD - ECC key is not valid */
	XLOADER_ECC_KAT_FAILED_ERROR,   /**< 0xE - ECC KAT Failes */

	XLOADER_AESDPACM_KAT_WRITE_KEY_FAILED_ERROR, /**< 0xF - Error when AESDPACM
							key write fails. */
	XLOADER_AESDPACM_KAT_KEYLOAD_FAILED_ERROR,   /**< 0x10 - Error when AESDPACM
                                                        key load fails. */
	XLOADER_AESDPACM_SSS_CFG_FAILED_ERROR,       /**< 0x11 Eroor ehen AESDPACM
							sss config fails */
	XLOADER_AESDPACM_KAT_FAILED_ERROR,           /**< 0x12 - AESDPACM KAT Failes*/
	XLOADER_AESDPACM_KAT_CHECK1_FAILED_ERROR,    /**< 0x13 - Error when AESDPACM
							data not matched with
							expected data  */
	XLOADER_AESDPACM_KAT_CHECK2_FAILED_ERROR,    /**< 0x14 - Error when AESDPACM
							data not matched with
							expected data  */
	XLOADER_AESDPACM_KAT_CHECK3_FAILED_ERROR,    /**< 0x15 - Error when AESDPACM
							data not matched with
							expected data  */
	XLOADER_AESDPACM_KAT_CHECK4_FAILED_ERROR,    /**< 0x16 - Error when AESDPACM
							data not matched with
							expected data  */
	XLOADER_AESDPACM_KAT_CHECK5_FAILED_ERROR     /**< 0x17 - Error when AESDPACM
							data not matched with
							expected data  */
}Xloader_Kat_Error;

/************************** Function Prototypes ******************************/
u32 XLoader_Sha3Kat(void);
u32 XLoader_AesKat(void);
u32 XLoader_RsaKat(void);
u32 XLoader_EcdsaKat(void);
u32 XLoader_AesCmKat(void);
#ifdef __cplusplus
}
#endif


#endif /* XLOADER_SECURE_KAT_H_ */
