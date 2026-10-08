/**************************************************************************************************
* Copyright (c) 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
**************************************************************************************************/

/*************************************************************************************************/
/**
*
* @file xoverlay_sections.c
*
* This file defines overlay section-related definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date       Changes
* ----- ---- -------- ----------------------------------------------------------------------------
* 1.0   rmv  06/11/26 Initial release
*       kk   08/25/26 Added SLH-DSA overlay section definitions
*       vm   09/15/26 Added the key vault overlay image header
*
* </pre>
*
*************************************************************************************************/
/**
* @addtogroup xoverlay_server_apis Overlay Manager APIs
* @{
*/
/************************************** Include Files ********************************************/
#include "xoverlay.h"

/************************************ Macro Definitions ******************************************/
/*
 * The overlay image header is placed into dedicated sections so the OVERLAY
 * blocks in lscript.ld can position them at the base of each overlay image.
 */

#define OVERLAY_SECTION(name)	__attribute__((section(name), used))	/**< Place a symbol into the named
								linker section */

#if defined(XASU_ECC_ENABLE) || defined(XASU_RSA_ECC_ENABLE)
#define XOVERLAY_ECC_HDR	OVERLAY_SECTION(".overlay_ecc.header")		/**< ECC overlay
										header section */
#endif
#ifdef XASU_RSA_ECC_ENABLE
#define XOVERLAY_RSA_HDR	OVERLAY_SECTION(".overlay_rsa.header")		/**< RSA overlay
										header section */
#endif
#ifdef XASU_LMS_ENABLE
#define XOVERLAY_LMS_HDR	OVERLAY_SECTION(".overlay_lms.header")		/**< LMS overlay
										header section */
#endif
#ifdef XASU_MLDSA_ENABLE
#define XOVERLAY_MLDSA_HDR	OVERLAY_SECTION(".overlay_mldsa.header")	/**< ML-DSA overlay
										header section */
#endif
#ifdef XASU_MLKEM_ENABLE
#define XOVERLAY_MLKEM_HDR	OVERLAY_SECTION(".overlay_mlkem.header")	/**< ML-KEM overlay
										header section */
#endif
#ifdef XASU_SLHDSA_ENABLE
#define XOVERLAY_SLHDSA_HDR	OVERLAY_SECTION(".overlay_slhdsa.header")	/**< SLH-DSA overlay
										header section */
#endif
#ifdef XASU_KEYMANAGER_ENABLE
#define XOVERLAY_KEYVAULT_HDR	OVERLAY_SECTION(".overlay_keyvault.header")	/**< Key vault
										overlay header
										section */
#endif

/********************************** Variable Definitions *****************************************/
#if defined(XASU_ECC_ENABLE) || defined(XASU_RSA_ECC_ENABLE)
extern char __overlay_ecc_img_size[];	/**< Size of the ECC overlay image */
extern char __overlay_ecc_txt_size[];	/**< Size of the ECC overlay text image */
extern char __overlay_ecc_data_size[];	/**< Size of the ECC overlay data image */
#endif

#ifdef XASU_RSA_ECC_ENABLE
extern char __overlay_rsa_img_size[];	/**< Size of the RSA overlay image */
extern char __overlay_rsa_txt_size[];	/**< Size of the RSA overlay text image */
extern char __overlay_rsa_data_size[];	/**< Size of the RSA overlay data image */
#endif

#ifdef XASU_LMS_ENABLE
extern char __overlay_lms_img_size[];	/**< Size of the LMS overlay image */
extern char __overlay_lms_txt_size[];	/**< Size of the LMS overlay text image */
extern char __overlay_lms_data_size[];	/**< Size of the LMS overlay data image */
#endif
#ifdef XASU_SLHDSA_ENABLE
extern char __overlay_slhdsa_img_size[];	/**< Size of the SLH-DSA overlay image */
extern char __overlay_slhdsa_txt_size[];	/**< Size of the SLH-DSA overlay text image */
extern char __overlay_slhdsa_data_size[];	/**< Size of the SLH-DSA overlay data image */
#endif

#ifdef XASU_MLDSA_ENABLE
extern char __overlay_mldsa_img_size[];		/**< Size of the ML-DSA overlay image */
extern char __overlay_mldsa_txt_size[];		/**< Size of the ML-DSA overlay text image */
extern char __overlay_mldsa_data_size[];	/**< Size of the ML-DSA overlay data image */
#endif

#ifdef XASU_MLKEM_ENABLE
extern char __overlay_mlkem_img_size[];		/**< Size of the ML-KEM overlay image */
extern char __overlay_mlkem_txt_size[];		/**< Size of the ML-KEM overlay text image */
extern char __overlay_mlkem_data_size[];	/**< Size of the ML-KEM overlay data image */
#endif

#ifdef XASU_KEYMANAGER_ENABLE
extern char __overlay_keyvault_img_size[];	/**< Size of the key vault overlay image */
extern char __overlay_keyvault_txt_size[];	/**< Size of the key vault overlay text image */
extern char __overlay_keyvault_data_size[];	/**< Size of the key vault overlay data image */
#endif

#if defined(XASU_ECC_ENABLE) || defined(XASU_RSA_ECC_ENABLE)
/**< ECC overlay header */
XOVERLAY_ECC_HDR
static const XOverlay_Header XEcc_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_ECC,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_ecc_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_ecc_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_ecc_data_size,
};
#endif

#ifdef XASU_RSA_ECC_ENABLE
/**< RSA overlay header */
XOVERLAY_RSA_HDR
static const XOverlay_Header XRsa_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_RSA,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_rsa_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_rsa_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_rsa_data_size,
};
#endif

#ifdef XASU_LMS_ENABLE
/**< LMS overlay header */
XOVERLAY_LMS_HDR
static const XOverlay_Header XLms_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_LMS,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_lms_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_lms_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_lms_data_size,
};
#endif /* XASU_LMS_ENABLE */

#ifdef XASU_MLDSA_ENABLE
/**< ML-DSA overlay header */
XOVERLAY_MLDSA_HDR
static const XOverlay_Header XMldsa_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_MLDSA,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_mldsa_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_mldsa_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_mldsa_data_size,
};
#endif /* XASU_MLDSA_ENABLE */

#ifdef XASU_MLKEM_ENABLE
/**< ML-KEM overlay header */
XOVERLAY_MLKEM_HDR
static const XOverlay_Header XMlkem_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_MLKEM,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_mlkem_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_mlkem_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_mlkem_data_size,
};
#endif /* XASU_MLKEM_ENABLE */

#ifdef XASU_SLHDSA_ENABLE
/**< SLH-DSA overlay header */
XOVERLAY_SLHDSA_HDR
static const XOverlay_Header XSlhdsa_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_SLHDSA,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_slhdsa_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_slhdsa_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_slhdsa_data_size,
};
#endif /* XASU_SLHDSA_ENABLE */

#ifdef XASU_KEYMANAGER_ENABLE
/**< Key vault overlay header */
XOVERLAY_KEYVAULT_HDR
static const XOverlay_Header XKeyVault_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_KEYVAULT,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_keyvault_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_keyvault_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_keyvault_data_size,
};
#endif /* XASU_KEYMANAGER_ENABLE */
/** @} */
