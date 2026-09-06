/******************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file server/core/generic/xsecure_server_libcaps.h
*
* This server-only header derives the XilSecure library capability bitmap at
* compile time. Each build-gated feature contributes one permanently-assigned
* bit (see XSecure_LibCapBit in xsecure_defs.h) to the aggregate
* XSECURE_LIB_CAPABILITIES constant, which the Features command reports
* to the client alongside the library version.
*
* It lives in server/core (shared server code) rather than a per-platform
* directory so a single derivation is reused across platforms: the gate macros
* are named consistently, and a feature not compiled into a given platform
* simply leaves its gate undefined and contributes 0.
*
* Derivation is entirely compile time (a chain of ifdef checks OR'd together),
* so there is no runtime cost: a feature that is not compiled in simply
* contributes 0. Bits are append-only - a new capability consumes a new,
* previously-zero bit and never disturbs existing assignments, so older clients
* stay compatible.
*
* Feature gating:
*  - PLM_SECURE_EXCLUDE is the umbrella switch: when defined, the whole secure
*    crypto set (RSA + ECDSA + AES) is excluded, so all of those bits read 0.
*  - Inside it, PLM_RSA_EXCLUDE and PLM_ECDSA_EXCLUDE are the finer per-engine
*    gates. AES has no finer gate, so AES presence == !PLM_SECURE_EXCLUDE.
*  - ECC curve support macros: P-192/P-224/P-256/P-521 are BSP configurable
*    (xsecure_config.h for SDT, xparameters.h for the classic flow); P-384 is
*    always supported when ECDSA is in the build.
*  - XSECURE_CRYPTOALGINFO_ENABLED is independent of the secure engines.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- --------    -------------------------------------------------------
* 5.8   tus  08/21/26    Initial release with ECDSA/RSA/AES/crypto-alg-info and
*                        ECC big-endian capability bits
*
* </pre>
*
******************************************************************************/
/**
* @addtogroup xsecure_common_apis XilSecure Common Apis
* @{
*/
#ifndef XSECURE_SERVER_LIBCAPS_H
#define XSECURE_SERVER_LIBCAPS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xparameters.h"
#ifdef SDT
#include "xsecure_config.h"
#endif
#include "xsecure_defs.h"

/************************** Constant Definitions ****************************/

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef PLM_SECURE_EXCLUDE
/*
 * The secure crypto set (RSA + ECDSA + AES) is part of this build.
 */

/* AES has no finer per-engine gate: present whenever the secure set is in. */
#define XSECURE_CAP_AES		((u32)1U << XSECURE_CAP_AES_BIT)	/**< AES engine present */

/* RSA engine */
#ifndef PLM_RSA_EXCLUDE
#define XSECURE_CAP_RSA		((u32)1U << XSECURE_CAP_RSA_BIT)	/**< RSA engine present */
#else
#define XSECURE_CAP_RSA		(0U)	/**< RSA engine excluded from the build */
#endif

/* ECDSA engine and its NIST curves */
#ifndef PLM_ECDSA_EXCLUDE
#define XSECURE_CAP_ECDSA	((u32)1U << XSECURE_CAP_ECDSA_BIT)	/**< ECDSA engine present */

/*
 * P-384 is unconditionally supported whenever ECDSA is part of the build
 * (XSECURE_ECC_SUPPORT_NIST_P384 is defined in xsecure_ellipticcrvs.h). The
 * remaining curves are BSP configurable.
 */
#define XSECURE_CAP_ECC_NIST_P384	((u32)1U << XSECURE_CAP_ECC_NIST_P384_BIT)	/**< NIST P-384 curve supported */

#ifdef XSECURE_ECC_SUPPORT_NIST_P192
#define XSECURE_CAP_ECC_NIST_P192	((u32)1U << XSECURE_CAP_ECC_NIST_P192_BIT)	/**< NIST P-192 curve supported */
#else
#define XSECURE_CAP_ECC_NIST_P192	(0U)	/**< NIST P-192 curve not supported */
#endif

#ifdef XSECURE_ECC_SUPPORT_NIST_P224
#define XSECURE_CAP_ECC_NIST_P224	((u32)1U << XSECURE_CAP_ECC_NIST_P224_BIT)	/**< NIST P-224 curve supported */
#else
#define XSECURE_CAP_ECC_NIST_P224	(0U)	/**< NIST P-224 curve not supported */
#endif

#ifdef XSECURE_ECC_SUPPORT_NIST_P256
#define XSECURE_CAP_ECC_NIST_P256	((u32)1U << XSECURE_CAP_ECC_NIST_P256_BIT)	/**< NIST P-256 curve supported */
#else
#define XSECURE_CAP_ECC_NIST_P256	(0U)	/**< NIST P-256 curve not supported */
#endif

#ifdef XSECURE_ECC_SUPPORT_NIST_P521
#define XSECURE_CAP_ECC_NIST_P521	((u32)1U << XSECURE_CAP_ECC_NIST_P521_BIT)	/**< NIST P-521 curve supported */
#else
#define XSECURE_CAP_ECC_NIST_P521	(0U)	/**< NIST P-521 curve not supported */
#endif

/*
 * ECC data endianness (XSECURE_ELLIPTIC_ENDIANNESS: 0 = little, 1 = big). The
 * bit is set only for big-endian; little-endian (the default) leaves it 0.
 */
#if defined(XSECURE_ELLIPTIC_ENDIANNESS) && (XSECURE_ELLIPTIC_ENDIANNESS == 1U)
#define XSECURE_CAP_ECC_BIG_ENDIAN	((u32)1U << XSECURE_CAP_ECC_BIG_ENDIAN_BIT)	/**< ECC APIs use big-endian data */
#else
#define XSECURE_CAP_ECC_BIG_ENDIAN	(0U)	/**< ECC APIs use little-endian data */
#endif

/** Aggregate of all ECC-domain capability bits (curves + endianness) */
#define XSECURE_CAP_ECC_ALL	(XSECURE_CAP_ECC_NIST_P192 | XSECURE_CAP_ECC_NIST_P224 | \
				 XSECURE_CAP_ECC_NIST_P256 | XSECURE_CAP_ECC_NIST_P384 | \
				 XSECURE_CAP_ECC_NIST_P521 | XSECURE_CAP_ECC_BIG_ENDIAN)
#else
#define XSECURE_CAP_ECDSA	(0U)	/**< ECDSA engine excluded from the build */
#define XSECURE_CAP_ECC_ALL	(0U)	/**< No ECC capabilities advertised */
#endif /* PLM_ECDSA_EXCLUDE */

#else /* PLM_SECURE_EXCLUDE */
#define XSECURE_CAP_AES		(0U)	/**< AES engine excluded from the build */
#define XSECURE_CAP_RSA		(0U)	/**< RSA engine excluded from the build */
#define XSECURE_CAP_ECDSA	(0U)	/**< ECDSA engine excluded from the build */
#define XSECURE_CAP_ECC_ALL	(0U)	/**< No ECC capabilities advertised */
#endif /* PLM_SECURE_EXCLUDE */

/*
 * Crypto algorithm-info reporting is independent of the secure engines. The bit
 * must track where the algorithm version/NIST status words are actually written
 * in the Features response: on plain Versal the fill is gated by
 * XSECURE_CRYPTOALGINFO_ENABLED, whereas VersalNet and Versal_2ve_2vm write them
 * unconditionally (VERSAL_NET covers both). Gate on the same condition so the
 * advertised capability matches the emitted data.
 */
#if defined(XSECURE_CRYPTOALGINFO_ENABLED) || defined(VERSAL_NET)
#define XSECURE_CAP_LIB_CRYPTOALGINFO	((u32)1U << XSECURE_CAP_LIB_CRYPTOALGINFO_BIT)	/**< Crypto algorithm-info reporting present */
#else
#define XSECURE_CAP_LIB_CRYPTOALGINFO	(0U)	/**< Crypto algorithm-info reporting excluded */
#endif

/*
 * Platform-specific capabilities. Some capabilities exist only on certain
 * platforms. TRNG (IPI) and ECDH key agreement are served on both VersalNet and
 * Versal_2ve_2vm, whereas LMS post-quantum sign-verify is specific to Versal_2ve_2vm.
 * They are guarded by the platform macros so they never light up on platforms
 * that do not implement them (e.g. plain Versal), and stay gated by the relevant
 * build-exclude switches where applicable.
 */
#ifdef VERSAL_NET
#ifndef PLM_SECURE_EXCLUDE
/* TRNG is part of the secure set (shares the PLM_SECURE_EXCLUDE umbrella). */
#define XSECURE_CAP_TRNG	((u32)1U << XSECURE_CAP_TRNG_BIT)	/**< TRNG engine present */

/* ECDH key agreement rides on the ECDSA/ECC engine. */
#ifndef PLM_ECDSA_EXCLUDE
#define XSECURE_CAP_ECDH	((u32)1U << XSECURE_CAP_ECDH_BIT)	/**< ECDH key agreement present */
#else
#define XSECURE_CAP_ECDH	(0U)	/**< ECDH excluded with ECDSA */
#endif
#else /* PLM_SECURE_EXCLUDE */
#define XSECURE_CAP_TRNG	(0U)	/**< TRNG excluded with the secure set */
#define XSECURE_CAP_ECDH	(0U)	/**< ECDH excluded with the secure set */
#endif

/* LMS post-quantum sign-verify is specific to Versal_2ve_2vm. */
#ifdef VERSAL_2VE_2VM
#define XSECURE_CAP_LMS		((u32)1U << XSECURE_CAP_LMS_BIT)	/**< LMS sign-verify present */
#else
#define XSECURE_CAP_LMS		(0U)	/**< LMS sign-verify not available on this platform */
#endif

/** Aggregate of platform-specific capability bits (VersalNet / Versal_2ve_2vm) */
#define XSECURE_CAP_PLAT_ALL	(XSECURE_CAP_ECDH | XSECURE_CAP_TRNG | XSECURE_CAP_LMS)
#else /* !VERSAL_NET */
#define XSECURE_CAP_PLAT_ALL	(0U)	/**< No platform-specific capabilities on this platform */
#endif

/**
 * Library-wide capability bitmap reported to the client. New algorithm families
 * and library flags append their own bits here as they are added.
 */
#define XSECURE_LIB_CAPABILITIES	(XSECURE_CAP_ECC_ALL | XSECURE_CAP_ECDSA | \
					 XSECURE_CAP_RSA | XSECURE_CAP_AES | \
					 XSECURE_CAP_LIB_CRYPTOALGINFO | XSECURE_CAP_PLAT_ALL)

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SERVER_LIBCAPS_H */
/** @} */
