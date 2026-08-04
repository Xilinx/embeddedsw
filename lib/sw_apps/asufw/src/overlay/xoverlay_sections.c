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

#define OVERLAY_SECTION(name)	__attribute__((section(name)))	/**< Place a symbol into the named
								linker section */

#define XOVERLAY_RSA_ECC_HDR	OVERLAY_SECTION(".overlay_rsa_ecc.header")	/**< RSA-ECC overlay
										header section */
#ifdef XASU_LMS_ENABLE
#define XOVERLAY_LMS_HDR	OVERLAY_SECTION(".overlay_lms.header")		/**< LMS overlay
										header section */
#endif

/********************************** Variable Definitions *****************************************/
extern char __overlay_rsa_ecc_img_size[];	/**< Size of the RSA-ECC overlay image */
extern char __overlay_rsa_ecc_txt_size[];	/**< Size of the RSA-ECC overlay text image */
extern char __overlay_rsa_ecc_data_size[];	/**< Size of the RSA-ECC overlay data image */

#ifdef XASU_LMS_ENABLE
extern char __overlay_lms_img_size[];	/**< Size of the LMS overlay image */
extern char __overlay_lms_txt_size[];	/**< Size of the LMS overlay text image */
extern char __overlay_lms_data_size[];	/**< Size of the LMS overlay data image */
#endif

/**< RSA-ECC overlay header */
XOVERLAY_RSA_ECC_HDR
const XOverlay_Header XRsaEcc_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_RSA_ECC,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_rsa_ecc_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_rsa_ecc_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_rsa_ecc_data_size,
};

#ifdef XASU_LMS_ENABLE
/**< LMS overlay header */
XOVERLAY_LMS_HDR
const XOverlay_Header XLms_OverlayHeader = {
	.Magic = XOVERLAY_MAGIC,
	.AlgoId = XOVERLAY_ALGO_LMS,
	.Version = XOVERLAY_HEADER_VERSION,
	.ImgSize = (u32)(UINTPTR)__overlay_lms_img_size,
	.TxtSize = (u32)(UINTPTR)__overlay_lms_txt_size,
	.DataSize = (u32)(UINTPTR)__overlay_lms_data_size,
};
#endif /* XASU_LMS_ENABLE */
/** @} */
