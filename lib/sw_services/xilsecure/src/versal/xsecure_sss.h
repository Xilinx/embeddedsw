/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_sss.h
*
* This file contains macros and functions required for SSS configuration for
* Versal
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.2   har     03/26/20 Initial Release
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

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal */
#define XSECURE_SSS_CFG_LEN_IN_BITS	(4U) /**< Length is bits */
#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */
#define XSECURE_SSS_MAX_SRCS		(8U)	/**< Maximum resources */

/***************************** Type Definitions******************************/
/**
 * Instance structure of secure stream switch
 */
typedef struct {
	u32 Address; /**< Address of SSS CFG register */
}XSecure_Sss;

/*
 * Sources to be selected to configure secure stream switch.
 * XSECURE_SSS__IGNORE is added to make enum type int
 * irrespective of compiler used.
 */
typedef enum {
	XSECURE_SSS_IGNORE = -1,
	XSECURE_SSS_DMA0 = 0,
	XSECURE_SSS_DMA1,
	XSECURE_SSS_PTPI,
	XSECURE_SSS_AES,
	XSECURE_SSS_SHA,
	XSECURE_SSS_SBI,
	XSECURE_SSS_PZI,
	XSECURE_SSS_INVALID
}XSecure_SssSrc;

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
void XSecure_SssInitialize(XSecure_Sss *InstancePtr);
u32 XSecure_SssAes(XSecure_Sss *InstancePtr, XSecure_SssSrc InputSrc,
		XSecure_SssSrc OutputSrc);
u32 XSecure_SssSha(XSecure_Sss *InstancePtr, u16 DmaId);
u32 XSecure_SssDmaLoopBack(XSecure_Sss *InstancePtr, u16 DmaId);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SSS_H_ */
/**@}*/
