/******************************************************************************
* Copyright (c) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xplm_load.h
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.00  bm   05/31/24 Initial release
 * </pre>
 *
 ******************************************************************************/

/**
 * @addtogroup spartanup_plm_apis SpartanUP PLM APIs
 * @{
 */

#ifndef XPLM_LOAD_H
#define XPLM_LOAD_H

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xplm_util.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
/** @cond spartanup_plm_internal */
#define XPLM_SMAP_WD_PATTERN_SIZE (0x10U) /** SMAP Width Size */
#define XPLM_BOOT_HDR_TOTAL_SIZE (0x340U)  /** Boot Header Total Size */
#define XPLM_BOOT_HDR_SIZE_WITHOUT_SMAP_WIDTH	(XPLM_BOOT_HDR_TOTAL_SIZE - XPLM_SMAP_WD_PATTERN_SIZE)
#define XPLM_BOOT_HDR_CHECKSUM_SIZE		(4U)
#define XPLM_BOOT_HDR_CHECKSUM_END_OFFSET	(XPLM_BOOT_HDR_SIZE_WITHOUT_SMAP_WIDTH - XPLM_BOOT_HDR_CHECKSUM_SIZE)

#define XPLM_HASH_BLOCK_ADDR_IN_RAM	(0x0402FBA0U)

#define XPLM_SECURE_GCM_TAG_SIZE	(16U) /** GCM Tag Size in Bytes */

#define WIDTH_DETECT_WORD	(0x665599AA)
#define WIDTH_DETECT_WORD_LEN_B	(0x4U)
#define WIDTH_DETECT_WORD_LEN_W	(XPLM_BYTES_TO_WORDS(WIDTH_DETECT_WORD_LEN_B))
/** @endcond */

/**
 * Constants to indicate Boot modes.
 */
typedef enum {
	XPLM_BOOT_MODE_RESERVED = 0x0U, /**< reserved */
	XPLM_BOOT_MODE_QSPI24 = 0x1U, /**< QSPI 24-bit addressing boot mode */
	XPLM_BOOT_MODE_QSPI32 = 0x2U, /**< QSPI 32-bit addressing boot mode */
	XPLM_BOOT_MODE_OSPI = 0x3U, /**< OSPI boot mode */
	XPLM_BOOT_MODE_DFT = 0x4U, /**< DFT boot mode */
	XPLM_BOOT_MODE_JTAG = 0x5U, /**< JTAG boot mode */
	XPLM_BOOT_MODE_SMAP = 0x6U, /**< SMAP boot mode */
	XPLM_BOOT_MODE_SELECT_SERIAL = 0x7U, /**< Select serial boot mode */
} XPlm_BootModes;

u32 XPlm_LoadFullPdi(void);
u32 XPlm_LoadPartialPdi(void);
void XPlm_CaptureCriticalInfo(void);

#endif /* XPLM_LOAD_H */

/** @} end of spartanup_plm_apis group*/
