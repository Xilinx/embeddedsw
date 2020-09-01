/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
*		am 	 08/04/2020 Resolved MISRA C Violations
*		am 	 08/19/2020 Resolved MISRA C violations.
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
#include "xil_io.h"
#include "xil_printf.h"

/*************************** Constant Definitions *****************************/
/** @cond xpuf_internal
@{
*/
#if defined XPUF_DEBUG
#define XPUF_DEBUG_GENERAL (1U)
#else
#define XPUF_DEBUG_GENERAL (0U)
#endif

#define xPuf_printf(type, ...)	if ((type) == (1U)) {xil_printf (__VA_ARGS__);}

#define XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS		(350U)
#define XPUF_4K_PUF_SYN_LEN_IN_WORDS			(140U)
#define XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS		(127U)
#define XPUF_12K_PUF_SYN_LEN_IN_WORDS			(350U)
#define XPUF_SHUTTER_VALUE				(0x1000040U)
#define XPUF_ID_LEN_IN_WORDS					(0x8U)
#define XPUF_WORD_LENGTH				(0x4U)

#define XPUF_REGISTRATION				(0x0U)
#define XPUF_REGEN_ON_DEMAND				(0x1U)
#define XPUF_REGEN_ID_ONLY				(0x2U)

#define XPUF_SYNDROME_MODE_4K				(0x0U)
#define XPUF_SYNDROME_MODE_12K				(0x1U)

#define XPUF_EFUSE_TRIM_MASK				(0xFFFFF000U)
#define XPUF_LAST_WORD_OFFSET				(126U)
#define XPUF_LAST_WORD_MASK				(0xFFFFFFF0U)

/* Key registration time error codes */
#define XPUF_ERROR_INVALID_PARAM				(0x02)
#define XPUF_ERROR_INVALID_SYNDROME_MODE		(0x03)
#define XPUF_ERROR_SYNDROME_WORD_WAIT_TIMEOUT	(0x04)
#define XPUF_ERROR_PUF_DONE_WAIT_TIMEOUT		(0x07)
#define XPUF_ERROR_REGISTRATION_INVALID			(0x08)

/* Key regeneration time error codes */
#define XPUF_ERROR_CHASH_NOT_PROGRAMMED			(0x10)
#define XPUF_ERROR_PUF_STATUS_DONE_TIMEOUT		(0x11)
#define XPUF_ERROR_INVALID_REGENERATION_TYPE	(0x12)
#define XPUF_ERROR_INVALID_PUF_OPERATION		(0x13)
#define XPUF_ERROR_REGENERATION_INVALID			(0x14)
#define XPUF_ERROR_REGEN_PUF_HD_INVALID			(0x15)
#define XPUF_ERROR_INVALID_READ_HD_INPUT		(0x16)
#define XPUF_ERROR_PUF_DONE_KEY_ID_NT_RDY		(0x17)
#define XPUF_ERROR_PUF_DONE_ID_NT_RDY			(0x18)

/***************************** Type Definitions *******************************/
typedef enum {
	XPUF_READ_FROM_RAM,
	XPUF_READ_FROM_EFUSE_CACHE
} XPuf_ReadOption;

typedef struct {
	u8 RegMode;		/* PUF Registration Mode 4K/12K*/
	u8 PufOperation;
	   /* PUF Registration/ Regeneration On Demand/ ID only regeneration) */
	u8 GlobalVarFilter;
	XPuf_ReadOption ReadOption;	/* Read helper data from eFuse Cache/DDR */
	u32 ShutterValue;
	u32 SyndromeData[XPUF_MAX_SYNDROME_DATA_LEN_IN_WORDS];
	u32 Chash;
	u32 Aux;
	u32 PufID[XPUF_ID_LEN_IN_WORDS];
	u32 SyndromeAddr;
	u32 EfuseSynData[XPUF_EFUSE_TRIM_SYN_DATA_IN_WORDS];
				 /* Trimmed data to be written in efuse */
} XPuf_Data;

/** @}
@endcond */

/***************** Macros (Inline Functions) Definitions *********************/

/*************************** Function Prototypes ******************************/
int XPuf_Registration(XPuf_Data *PufData);
int XPuf_Regeneration(XPuf_Data *PufData);
void XPuf_GenerateFuseFormat(XPuf_Data *PufData);

#ifdef __cplusplus
}
#endif

#endif  /* XPUF_H */
/**@}*/
