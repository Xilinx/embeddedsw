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
* 1.0   har     03/26/20 Initial Release
* 4.2   har     03/26/20 Updated file version to sync with library version
* 4.3   rpo     09/10/20 Changed the return type of some prototypes
*       am      09/24/20 Resolved MISRA C violations
*       har     10/12/20 Addressed security review comments
*       bsv     10/19/20 Changed register writes to PMC SSS Cfg to mask writes
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
#define XSECURE_SSS_MAX_SRCS	(8U)	/**< Maximum resources */
#define XSECURE_SSS_SBI_MASK	(0xF00000U)
#define XSECURE_SSS_SHA_MASK	(0xF0000U)
#define XSECURE_SSS_AES_MASK	(0xF000U)
#define XSECURE_SSS_DMA1_MASK	(0xF0U)
#define XSECURE_SSS_DMA0_MASK	(0xFU)
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)
#define XSECURE_SSS_SBI_DMA0_VAL	(0x500000U)
#define XSECURE_SSS_SBI_DMA1_VAL	(0xB00000U)
#define XSECURE_SSS_SHA_DMA0_VAL	(0xC0000U)
#define XSECURE_SSS_SHA_DMA1_VAL	(0x70000U)
#define XSECURE_SSS_AES_DMA0_VAL	(0xE000U)
#define XSECURE_SSS_AES_DMA1_VAL	(0x5000U)

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
int XSecure_SssInitialize(XSecure_Sss *InstancePtr);
int XSecure_SssAes(const XSecure_Sss *InstancePtr, XSecure_SssSrc InputSrc,
		   XSecure_SssSrc OutputSrc);
int XSecure_SssSha(const XSecure_Sss *InstancePtr, u16 DmaId);
int XSecure_SssDmaLoopBack(const XSecure_Sss *InstancePtr, u16 DmaId);

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SSS_H_ */
/**@}*/
