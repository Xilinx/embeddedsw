/******************************************************************************/
/**
* Copyright (c) 2023 - 2026 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
* @file xil_cryptoalginfo.h
*
* @addtogroup xil_cryptoalginfo Crypto Algorithm Information
*
* Crypto algorithm information structure declaration.
*
* @{
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 9.0   mmd      07/04/23 First release.
* 9.4   mku      09/29/25 Added XSECURE_ALWAYS_INLINE macro useful for function inlining.
* 9.6   tus      08/27/26 Extended structure with library version/capability
*                         fields (LibVersion, Capability[]) and shared
*                         response-index macros, renamed the canonical type to
*                         Xil_LibInfo, retaining Xil_CryptoAlgInfo as a
*                         backward-compatible alias.
* 9.6   ml   10/01/26 Fix doxygen 1.15.0 and compliance checker warnings
* </pre>
*
*****************************************************************************/
#ifndef XIL_CRYPTOALGINFO_H
#define XIL_CRYPTOALGINFO_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"

/**************************** Constant Definitions ****************************/
#define XSECURE_ALWAYS_INLINE __attribute__((always_inline)) inline /**< Always inline function */

/*
 * Library-info response layout. These are the ABSOLUTE word indices the server
 * writes in the Features response (that is before any client-side status-word
 * shift). Raw clients (that read InstancePtr->Response directly) use these
 * indices as-is; clients whose transport strips the status word at Response[0]
 * read the same words one slot lower (INDEX - 1U).
 */
#define XIL_LIBINFO_VERSION_RESP_INDEX	(3U)	/**< Packed library version word */
#define XIL_LIBINFO_CAP0_RESP_INDEX	(4U)	/**< Library capability word 0 */

/*
 * Number of leading Xil_LibInfo words the server places in a Features response,
 * in structure order: Version, NistStatus, LibVersion, Capability[0]. The
 * trailing Capability[1] word is not emitted by the server and is zeroed by the
 * client after the copy. Client GetServerLibInfo helpers copy this many words
 * from the first data word of the response directly into Xil_LibInfo.
 */
#define XIL_LIBINFO_RESP_WORDS		(4U)	/**< Server-provided Xil_LibInfo words */

/*
 * Index of the first library-info data word in a NON-shifted (raw) response,
 * where the transport leaves the status word at Response[0]. Raw clients (that
 * read the response directly, for example xilocp) copy Xil_LibInfo starting here.
 */
#define XIL_LIBINFO_RAW_RESP_BASE	(1U)	/**< First library-info word in a raw response */

/**************************** Type Definitions *******************************/
/**
 * @brief NIST compliance status for a cryptographic algorithm or module.
 */
typedef enum _Xil_CryptoAlgNistStatus {
	NOT_APPLICABLE   = 0x00, /**< NIST status is not applicable. */
	NIST_COMPLIANT   = 0x11, /**< Algorithm is NIST compliant. */
	NIST_NON_COMPLIANT = 0xFE, /**< Algorithm is not NIST compliant. */
} Xil_CryptoAlgNistStatus;

/**
 * Common crypto algorithm and library information structure.
 *
 * The first two members (Version, NistStatus) describe a specific cryptographic
 * algorithm/module. The last two members (LibVersion, Capability) describe the
 * running server LIBRARY as a whole. The server reports all of them at fixed
 * indices in every Features response, so one Features round-trip carries both
 * the algorithm and the library information on the wire.
 *
 * Which members a client API actually populates:
 *   X*_GetServerLibInfo()
 *     Populates LibVersion and Capability. For the crypto library (xilsecure)
 *     Version and NistStatus are filled from the same response; for the
 *     non-crypto libraries (loader/nvm/puf/ocp) those two words receive
 *     incidental Features data and MUST be treated as reserved.
 *   XSecure_GetAlgInfo() and the XSecure_*GetCryptoAlgInfo() wrappers
 *     Populate ONLY Version and NistStatus. LibVersion and Capability are left
 *     unmodified, so callers MUST NOT read them from a structure filled by
 *     these APIs; obtain them through XSecure_GetServerLibInfo() instead.
 *
 * Response mapping (data words after the status word at Response[0]):
 *   Version    <- Response[1]  (per-algorithm module version)
 *   NistStatus <- Response[2]
 *   LibVersion <- Response[3]  (packed: Major in the upper 16 bits, Minor in
 *                               the lower 16 bits)
 *   Capability <- Response[4..] (library capability bitmap words)
 */
typedef struct _Xil_CryptoAlgInfo {
	u32 Version;			/**< Per-algorithm module version (packed) */
	Xil_CryptoAlgNistStatus NistStatus; /**< NIST compliance status */
	u32 LibVersion;			/**< Server library version, packed as
					     (Major << 16) | Minor. Valid only
					     after X*_GetServerLibInfo() */
	u32 Capability[2];		/**< Server library capability bitmap words.
					     Valid only after X*_GetServerLibInfo() */
} Xil_LibInfo;

/**
 * Backward-compatible alias. Existing code that used Xil_CryptoAlgInfo for the
 * algorithm Version/NistStatus continues to compile unchanged; the added
 * library fields are simply left unused by those callers.
 */
typedef Xil_LibInfo Xil_CryptoAlgInfo;

#ifdef __cplusplus
}
#endif

/** @} */
#endif /* XIL_CRYPTOALGINFO_H */
