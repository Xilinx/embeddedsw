/***************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/**************************************************************************************************/
/**
*
* @file xsecure_mldsa_hw.h
*
* This file contains hardware register offsets for MLDSA core
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 5.7   tvp  02/19/26 Initial release
*       tvp  03/09/26 Added MLDSA hardware register macros required for signature generation
*
* </pre>
*
***************************************************************************************************/
/**
* @addtogroup xsecure_mldsa_server_apis XilSecure MLDSA Server APIs
* @{
*/
#ifndef XSECURE_MLDSA_HW_H_
#define XSECURE_MLDSA_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

/***************************************** Constant Definitions ***********************************/
/**
 * @cond xsecure_internal
 * @{
 */

#define XSECURE_MLDSA_BYTE_MASK			(0xFFU)
						/**< Byte mask for extracting byte values */

#define XSECURE_MLDSA_BASEADDR			(0xF1204000U)
						/**< MLDSA core base address */

#define XSECURE_MLDSA_RESET_OFFSET		(0x0000A100U)
						/**< MLDSA reset register offset */

#define XSECURE_MLDSA_STATUS_OFFSET		(0x00000014U)
						/**< MLDSA status register offset */

#define XSECURE_MLDSA_CTX_CONFIG_OFFSET		(0x0000015CU)
						/**< MLDSA context config register offset */

#define XSECURE_MLDSA_CTRL_OFFSET		(0x00000010U)
						/**< MLDSA control register offset */

#define XSECURE_MLDSA_CTX_0_OFFSET		(0x00000160U)
						/**< MLDSA context register offset */

#define XSECURE_MLDSA_PK_0_OFFSET		(0x00001000U)
						/**< MLDSA public key register offset */

#define XSECURE_MLDSA_SIGN_0_OFFSET		(0x00002000U)
						/**< MLDSA signature register offset
						  (for verification input and generation output) */

#define XSECURE_MLDSA_VERIF_RESULT_0_OFFSET	(0x000000D8U)
						/**< MLDSA signature verification result register
						  offset */

#define XSECURE_MLDSA_MSG_STROBE_OFFSET		(0x00000158U)
						/**< MLDSA message strobe register offset */

#define XSECURE_MLDSA_MSG_0_OFFSET		(0x00000098U)
						/**< MLDSA message 0 register offset */

#define XSECURE_MLDSA_SK_IN_0_OFFSET		(0x00006000U)
						/**< MLDSA secret key input register offset
						  (for signing) */

#define XSECURE_MLDSA_SIGN_RND_0_OFFSET		(0x00000078U)
						/**< MLDSA signing randomness register offset */

#define XSECURE_MLDSA_ENTROPY_0_OFFSET		(0x00000018U)
						/**< MLDSA entropy register offset */
/** @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_MLDSA_HW_H_ */
/** @} */
