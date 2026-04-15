/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file server/core/mldsa/xsecure_mldsa.h
*
* This file contains constants and function declarations used in ML-DSA to provides interface to
* MLDSA operations
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*       tvp  03/09/26 Added XSecure_MldsaSignGenerate() API
*
* </pre>
*
***************************************************************************************************/

/**
 * @addtogroup xsecure_mldsa_server_apis XilSecure MLDSA Server APIs
 * @{
 */
#ifndef XSECURE_MLDSA_H_
#define XSECURE_MLDSA_H_

#ifdef __cplusplus
extern "C" {
#endif

/**************************************** Include Files *******************************************/
#include "xil_types.h"
#include "xsecure_defs.h"
#include "xsecure_utils.h"

/***************************************** Constant Definitions ***********************************/
/**
 * @cond xsecure_internal
 * @{
 */

#define XSECURE_MLDSA_MAX_CTX_LEN		(0xFFU)		/**< Maximum context length */
#define XSECURE_MLDSA_CT_LEN			(0x40U)		/**< CT length */
#define XSECURE_MLDSA_PK_LEN			(0xA20U)	/**< Public key length */
#define XSECURE_MLDSA_SIGN_LEN			(0x1213U)	/**< Signature length */
#define XSECURE_MLDSA_SK_LEN			(0x1320U)	/**< Secret key length (4896 bytes for MLDSA-87) */
#define XSECURE_MLDSA_SIGN_RND_LEN		(0x20U)		/**< Signing randomness length (256 bits) */
#define XSECURE_MLDSA_ENTROPY_LEN		(0x40U)		/**< Entropy length (512 bits) */

#define XSECURE_MLDSA_TIMEOUT_MAX		(0x001FFFFFU)	/**< Maximum timeout value */

#define XSECURE_MLDSA_CTRL_SIGN_VERIFY_CMD		(0x00000003U)
							/**< Sign verification command */
#define XSECURE_MLDSA_CTRL_SIGN_GEN_CMD			(0x00000002U)
							/**< Sign generation command */
#define XSECURE_MLDSA_CTRL_STREAM_MSG_MASK		(0x00000040U)
							/**< Control register streaming mode mask */
#define XSECURE_MLDSA_CTRL_ZEROIZE_MASK			(0x00000008U)
							/**< Control register zeroize mask */
#define XSECURE_MLDSA_STATUS_READY_MASK			(0x00000001U)
							/**< Status register ready mask */
#define XSECURE_MLDSA_STATUS_VALID_MASK			(0x00000002U)
							/**< Status register valid mask */
#define XSECURE_MLDSA_STATUS_MSG_STREAM_READY_MASK	(0x00000004U)
							/**< Status register message stream ready
							  mask */

#define XSECURE_MLDSA_MSG_STROBE_DATA_MASK	(0x0000000FU)	/**< Message strobe data mask */
#define XSECURE_MLDSA_3_BYTE_STROBE_MASK	(0x00000007U)	/**< 3 byte strobe mask */
#define XSECURE_MLDSA_2_BYTE_STROBE_MASK	(0x00000003U)	/**< 2 byte strobe mask */
#define XSECURE_MLDSA_1_BYTE_STROBE_MASK	(0x00000001U)	/**< 1 byte strobe mask */
#define XSECURE_MLDSA_0_BYTE_STROBE_MASK	(0x00000000U)	/**< 0 byte strobe mask */

/***************************************** Type Definitions ***************************************/
/** @}
 * @endcond
 */

/*************************************** Function Prototypes **************************************/
int XSecure_MldsaSignVerify(const XSecure_MldsaSignVerifyParams *MldsaParams);
int XSecure_MldsaSignGenerate(XSecure_MldsaSignGenParams *MldsaParams);

/** @} */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_MLDSA_H_ */
