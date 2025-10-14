/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2025 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf_defs.h
*
* This file contains the xilpuf API IDs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   kpt  01/04/22 Initial release
* 2.3   ng   11/22/23 Fixed doxygen grouping
*
* </pre>
*
******************************************************************************/

#ifndef XPUF_DEFS_H
#define XPUF_DEFS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_printf.h"
#include "xil_types.h"

/************************** Constant Definitions ****************************/
#define XPUF_DEBUG	(0U)	/**< Enable client printfs by setting XPUF_DEBUG to 1 */

#if (XPUF_DEBUG)
#define XPUF_DEBUG_GENERAL (1U)	/**< Enable general debug prints */
#else
#define XPUF_DEBUG_GENERAL (0U)	/**< Disable general debug prints */
#endif

#define XPUF_REGISTRATION				(0x0U)
		/**< PUF Operation - PUF Registration */
#define XPUF_REGEN_ON_DEMAND				(0x1U)
		/**< PUF Operation - PUF On demand regeneration */
#define XPUF_REGEN_ID_ONLY				(0x2U)
		/**< PUF Operation - PUF ID only regeneration */

#define XPUF_SYNDROME_MODE_4K				(0x0U)
		/**< PUF Mode - 4K Syndrome mode */

#define XPUF_4K_PUF_SYN_LEN_IN_WORDS			(140U)
						/**< PUF 4K Syndrome length in words */
#define XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS		(XPUF_4K_PUF_SYN_LEN_IN_WORDS)
						/**< Max syndrome data length in words */
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS		(127U)
						/**< eFuse trim syndrome data length in words */
#define XPUF_ID_LEN_IN_WORDS				(0x8U)
						/**< PUF ID length in words */
#define XPUF_WORD_LENGTH				(0x4U)
						/**< PUF word length in bytes */
#define XPUF_MAX_SYNDROME_DATA_LEN_IN_BYTES		(XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)
						/**< Max syndrome data length in bytes */
#define XPUF_4K_PUF_SYN_LEN_IN_BYTES			(XPUF_4K_PUF_SYN_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)
						/**< PUF 4K Syndrome length in bytes */
#define XPUF_ID_LEN_IN_BYTES				(XPUF_ID_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)
						/**< PUF ID length in bytes */
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES		(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * \
								XPUF_WORD_LENGTH)
						/**< eFuse trim syndrome data length in bytes */
#if defined (VERSAL_NET)
#define XPUF_SHUTTER_VALUE				(0x01000080U)
		/**< PUF Shutter Value - Versal Net */
#else
#define XPUF_SHUTTER_VALUE				(0x81000100U)
		/**< PUF Shutter Value - Versal */
#endif

/***************** Macros (Inline Functions) Definitions *********************/
#define XPuf_Printf(DebugType, ...)	\
	if ((DebugType) == 1U) {xil_printf (__VA_ARGS__);}	/**< Print debug messages */

#define XPUF_API(ApiId)	((u32)ApiId)	/**< Typecast XPUF API ID to u32 */

#define XPUF_API_ID_MASK	(0xFFU)	/**< Mask for API ID */

/************************** Variable Definitions *****************************/

/**************************** Type Definitions *******************************/
/** PUF read options */
typedef enum {
	XPUF_READ_FROM_RAM,		/**< Read helper data from memory */
	XPUF_READ_FROM_EFUSE_CACHE	/**< Read helper data from eFuse cache */
} XPuf_ReadOption;

/** PUF Data configuration parameters */
typedef struct {
	u8 PufOperation;
	   /**< PUF Registration/ Regeneration On Demand/ ID only regeneration) */
	u8 GlobalVarFilter;	/**< Global variable filter setting */
	u8 ReadOption;		/**< Read helper data from eFuse Cache/DDR */
	u32 ShutterValue;	/**< Shutter value for PUF registration or regeneration */
	u64 SyndromeDataAddr;	/**< Address for syndrome data */
	u64 ChashAddr;		/**< Address for chash data */
	u64 AuxAddr;		/**< Address for auxiliary data */
	u64 PufIDAddr;		/**< Address for PUF ID data */
	u64 SyndromeAddr;	/**< Address for syndrome data */
	u64 EfuseSynDataAddr;	/**< Address for eFuse syndrome data */
#if defined (VERSAL_NET)
	u32 RoSwapVal;		/**< PUF Ring Oscillator Swap setting */
#endif
} XPuf_DataAddr;


/** XilPUF API ids  */
typedef enum {
	XPUF_API_FEATURES = 0U,		/**< API id for features */
	XPUF_PUF_REGISTRATION,		/**< API id for PUF registration */
	XPUF_PUF_REGENERATION,		/**< API id for PUF regeneration */
	XPUF_PUF_CLEAR_PUF_ID,		/**< API id for PUF clear id */
	XPUF_API_MAX,			/**< Number of API features */
} XPuf_ApiId;

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_DEFS_H */
