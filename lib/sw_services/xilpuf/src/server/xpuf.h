/******************************************************************************
* Copyright (c) 2019 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpuf.h
* @addtogroup xpuf_apis XilPuf APIs
* @{
* @cond xpuf_internal
* This file contains PUF interface APIs
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- ---------- -------------------------------------------------------
* 1.0   kal  08/01/2019 Initial release
* 1.1   har  01/27/2020 Updated XPufData structure to support on demand regeneration
*                       from efuse cache
*                       Added macros for supporting ID only regeneration and
*                       black key programming
* 1.2   har  07/03/2020 Renamed XPUF_ID_LENGTH macro as XPUF_ID_LEN_IN_WORDS
*       am   08/04/2020 Resolved MISRA C Violations
*       am   08/19/2020 Resolved MISRA C violations.
*       har  09/30/2020 Removed header files which were not required
*       har  10/17/2020 Updated default PUF shutter value
*                       Added error code for mismatch in MSB of PUF shutter value
*                       and Global Variation Filter option
* 1.3   har  01/06/2021 Added prototype for the XPuf_ClearPufId and related macros
*            02/19/2021 Added error code for syndrome data underflow and overflow
*       har  03/08/2021 Added error code for IRO frequency mismatch
*       har  05/03/2021 Renamed error code XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY
*                       as XPUF_ERROR_PUF_DONE_KEY_NT_RDY
* 1.4   kpt  12/02/2021 Added macro XPUF_4K_PUF_SYN_LEN_IN_BYTES
* 1.5   kpt  03/23/2022 Added macro's and error code related to IRO frequency
* 2.0   har  06/09/2022 Added support for Versal_Net
*                       Removed support for 12K mode
*
* </pre>
*
* @endcond
*
*******************************************************************************/
#ifndef XPUF_H
#define XPUF_H

#ifdef __cplusplus
extern "C" {
#endif

/****************************** Include Files *********************************/
#include "xil_types.h"
#include "xpuf_plat.h"
#include "xpuf_defs.h"

/*************************** Constant Definitions *****************************/
/** @cond xpuf_internal
@{
*/
#define XPUF_EFUSE_TRIM_MASK				(0xFFFFF000U)
		/**< Mask for trimming syndrome data to be stored in eFuses */
#define XPUF_LAST_WORD_OFFSET				(126U)
		/**< Offset for last word for PUF Syndrome data */
#define XPUF_LAST_WORD_MASK				(0xFFFFFFF0U)
		/**< Mask for last word for PUF Syndrome data */

#define XPUF_IRO_FREQ_320MHZ				(0x0U)
		/**< Selected IRO Frequency - 320 MHz */
#define XPUF_IRO_FREQ_400MHZ				(0x1U)
		/**< Selected IRO Frequency - 400 MHz */

/* Key registration time error codes */
#define XPUF_ERROR_INVALID_PARAM			(0x02)
		/**< Error due to invalid parameter */
#define XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT		(0x04)
		/** Error due to timeout while waiting for syndrome data to be
		  * generated */
#define XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT		(0x07)
		/** Error due to timeout while waiting for Done bit to be set
		  * in PUF Status */
#define XPUF_ERROR_REGISTRATION_INVALID			(0x08)
		/** Error as PUF registration is not allowed */
#define XPUF_SHUTTER_GVF_MISMATCH			(0x09)
		/** Error if MSB of PUF shutter value does not match with
		  * Global Variation Filter option */
#define XPUF_ERROR_SYN_DATA_ERROR			(0x0A)
		/** Error if Size of PUF Syndrome Data is not same as
		  * expected size */
#define XPUF_IRO_FREQ_WRITE_MISMATCH			(0x0B)
		/** Error if configured IRO frequency does not match
		  * intended IRO frequency */

/* Key regeneration time error codes */
#define XPUF_ERROR_CHASH_NOT_PROGRAMMED			(0x10)
		/** Error if PUF helper data is not provided */
#define XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT		(0x11)
		/** Error due to timeout while waiting for Done bit to be
		  * set in PUF Status */
#define XPUF_ERROR_INVALID_REGENERATION_TYPE		(0x12)
		/** Error due to invalid regeneration type */
#define XPUF_ERROR_INVALID_PUF_OPERATION		(0x13)
		/** Error due to invalid PUF operation */
#define XPUF_ERROR_REGENERATION_INVALID			(0x14)
		/** Error if PUF Regeneration is invalid */
#define XPUF_ERROR_REGEN_PUF_HD_INVALID			(0x15)
		/** Error in Regeneration as PUF Helper data in eFuse is
		  * invalidated */
#define XPUF_ERROR_INVALID_READ_HD_INPUT		(0x16)
		/** Error due to invalid option to read PUF helper data */
#define XPUF_ERROR_PUF_DONE_KEY_NT_RDY			(0x17)
		/** Error if PUF operation is done but Key Ready bit is not set */
#define XPUF_ERROR_PUF_DONE_ID_NT_RDY			(0x18)
		/** Error if PUF operation is done but ID Ready bit is not set */
#define XPUF_ERROR_PUF_ID_ZERO_TIMEOUT			(0x19)
		/** Error due to timeout while zeroizing PUF ID */

/***************************** Type Definitions *******************************/
typedef struct _XPuf_Data {
	u8 PufOperation;
	/**< PUF Registration/ Regeneration On-Demand/ ID only regeneration) */
	u8 GlobalVarFilter;	/**< Option to configure Global Variation Filter */
	XPuf_ReadOption ReadOption;	/**< Read helper data from eFuse Cache/DDR */
	u32 ShutterValue;		/**< Option to configure Shutter Value */
	u32 SyndromeData[XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS];
					/**< Syndrome data for PUF regeneration */
	u32 Chash;			/**< Chash for PUF regeneration */
	u32 Aux;			/**< Auxillary data for PUF regeneration */
	u32 PufID[XPUF_ID_LEN_IN_WORDS];/**< PUF ID */
	u32 SyndromeAddr;		/**< Address of syndrome data */
	u32 EfuseSynData[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS];
					/**< Trimmed data to be written in efuse */
#if defined (VERSAL_NET)
	u32 RoSwapVal;			/**< PUF Ring Oscillator Swap setting */
#endif
} XPuf_Data;

/** @}
@endcond */

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
int XPuf_Registration(XPuf_Data *PufData);
int XPuf_Regeneration(XPuf_Data *PufData);
int XPuf_GenerateFuseFormat(XPuf_Data *PufData);
int XPuf_ClearPufID(void);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_H */
/**@}*/
