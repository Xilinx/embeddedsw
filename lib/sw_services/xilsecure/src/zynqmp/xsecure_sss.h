/******************************************************************************
* Copyright (c) 2020 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xsecure_sss.h
*
* This file contains macros and functions required for the SSS configuration
* for Zynqmp
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.2   har     03/26/20 Initial Release
* 4.5   bsv     04/01/21 Added API to set SSS CFG register to PCAP
* 4.6   am      09/17/21 Resolved compiler warnings
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
#include "xparameters.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal */
#define XSECURE_SSS_CFG_LEN_IN_BITS	(4U) /**< Length is bits */
#define XSECURE_CSU_REG_BASE_ADDR	(0xFFCA0000U)
					/**< CSU base address */
#define XSECURE_SSS_ADDRESS		(0xFFCA0008U)/**< SSS base address */
#define XSECURE_SSS_MAX_SRCS		(5U)	/**< Maximum resources */

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
typedef enum{
	XSECURE_SSS_IGNORE = -1,
	XSECURE_SSS_PCAP = 0,
	XSECURE_SSS_DMA0,
	XSECURE_SSS_AES,
	XSECURE_SSS_SHA,
	XSECURE_SSS_INVALID
}XSecure_SssSrc;

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/

/************************** Function Prototypes ******************************/
void XSecure_SssInitialize(XSecure_Sss *InstancePtr);
u32 XSecure_SssAes(XSecure_Sss *InstancePtr, XSecure_SssSrc InputSrc,
		XSecure_SssSrc OutputSrc);
u32 XSecure_SssSha(XSecure_Sss *InstancePtr, u16 DmaId);
u32 XSecure_SssDmaLoopBack(XSecure_Sss *InstancePtr, u16 DmaId);
#ifdef XSECURE_TPM_ENABLE
u32 XSecure_SssPcap(XSecure_Sss *InstancePtr, u16 DmaId);
#endif

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SSS_H_ */
/**@}*/
