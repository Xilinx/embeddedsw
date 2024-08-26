/******************************************************************************
* Copyright (c) 2022-2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsecure_sss.h
*
* This file contains macros and functions required for SSS configuration for
* spartan ultrascale plus
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   kpt     08/18/24 Initial Release
*
* </pre>
* @endcond
******************************************************************************/
#ifndef XSECURE_SSS_H
#define XSECURE_SSS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xsecure_plat.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal */
#define XSECURE_SSS_CFG_LEN_IN_BITS	(4U) /**< Length is bits */

/***************************** Type Definitions******************************/
/**
 * Instance structure of secure stream switch
 */
typedef struct {
	u32 Address; /**< Address of SSS CFG register */
}XSecure_Sss;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XSecure_SssInitialize(XSecure_Sss *InstancePtr);
int XSecure_SssAes(const XSecure_Sss *InstancePtr, XSecure_SssSrc InputSrc,
		   XSecure_SssSrc OutputSrc);
int XSecure_SssSha(const XSecure_Sss *InstancePtr, u16 DmaId,
		XSecure_SssSrc Resource);
int XSecure_SssDmaLoopBack(const XSecure_Sss *InstancePtr, u16 DmaId);

/* Functions defined in xsecure_plat.c */
u32 XSecure_SssMask(XSecure_SssSrc InputSrc, XSecure_SssSrc OutputSrc, u32 Value);

/************************** Variable Prototypes ******************************/
extern const u8 XSecure_SssLookupTable[XSECURE_SSS_MAX_SRCS][XSECURE_SSS_MAX_SRCS];

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SSS_H_ */
/**@}*/
