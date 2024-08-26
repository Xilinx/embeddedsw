/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf.h
*
* This file contains PUF interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kpt  08/23/2024 Initial release
*
* </pre>
*
*******************************************************************************/
#ifndef XPUF_H
#define XPUF_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"

/*************************** Constant Definitions *****************************/
/**
 * @cond xpuf_internal
 * @{
 */

#define XPUF_4K_PUF_SYN_LEN_IN_WORDS			(140U)
#define XPUF_4K_PUF_TOT_SYN_LEN_IN_WORDS		(142U)
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS		(127U)
#define XPUF_ID_LEN_IN_WORDS					(0x8U)
#define XPUF_WORD_LENGTH						(0x4U)

#define XPUF_MAX_SYNDROME_DATA_LEN_IN_BYTES		(XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)
#define XPUF_4K_PUF_SYN_LEN_IN_BYTES			(XPUF_4K_PUF_SYN_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)

#define XPUF_ID_LEN_IN_BYTES				(XPUF_ID_LEN_IN_WORDS * \
								XPUF_WORD_LENGTH)

#define XPUF_EFUSE_TRIM_SYN_DATA_IN_BYTES		(XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS * \
								XPUF_WORD_LENGTH)

#define XPUF_EFUSE_TRIM_MASK				(0xFFFFF000U)
		/**< Mask for trimming syndrome data to be stored in eFuses */
#define XPUF_LAST_WORD_OFFSET				(126U)
		/**< Offset for last word for PUF Syndrome data */
#define XPUF_LAST_WORD_MASK				(0xFFFFFFF0U)
		/**< Mask for last word for PUF Syndrome data */

/* Key registration time error codes */
#define XPUF_ERROR_INVALID_PARAM			(0x02)
		/**< Error due to invalid parameter */
#define XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT		(0x04)
		/** Error due to timeout while waiting for syndrome data to be
		  * generated */
#define XPUF_ERROR_SYNDROME_SEG_WAIT_TIMEOUT	(0x05)
		/** Error due to timeout while waiting for syndrome segment ready */

#define XPUF_ERROR_KEY_NOT_CONVERGED			(0x06)
		/** Error when iterative convergence not done */

#define XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT		(0x07)
		/** Error due to timeout while waiting for Done bit to be set
		  * in PUF Status */
#define XPUF_ERROR_SYN_DATA_ERROR			(0x08)
		/** Error if Size of PUF Syndrome Data is not same as
		  * expected size */
#define XPUF_ERROR_PUF_ID_ZERO_TIMEOUT			(0x09)
		/** Error due to timeout while zeroizing PUF ID */

#define XPUF_SHUTTER_VALUE				(0x01000020U)
		/** Shutter value */

#define XPUF_AUX_SHIFT_VALUE 			(4U)
				/**< No of bits aux has to shift*/

/***************************** Type Definitions *******************************/
typedef struct _XPuf_Data {
	/**< PUF Registration/ Regeneration On-Demand/ ID only regeneration) */
	u8 GlobalVarFilter;	/**< Option to configure Global Variation Filter */
	u32 ShutterValue;		/**< Option to configure Shutter Value */
	u32 SyndromeData[XPUF_4K_PUF_SYN_LEN_IN_WORDS];
					/**< Syndrome data for PUF regeneration */
	u32 Chash;			/**< Chash for PUF regeneration */
	u32 Aux;			/**< Auxiliary data for PUF regeneration */
	u32 PufID[XPUF_ID_LEN_IN_WORDS];/**< PUF ID */
	u32 SyndromeAddr;		/**< Address of syndrome data */
					/**< Trimmed data */
	u32 TrimmedSynData[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS];
					/**< Trimmed data to be written in efuse */
	u32 RoSwapVal;			/**< PUF Ring Oscillator Swap setting */
} XPuf_Data;

/**
 * @}
 * @endcond
 */

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
int XPuf_Registration(XPuf_Data *PufData);
int XPuf_Regeneration(XPuf_Data *PufData);
int XPuf_ClearPufID(void);
int XPuf_TrimPufData(XPuf_Data *PufData);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_H */
