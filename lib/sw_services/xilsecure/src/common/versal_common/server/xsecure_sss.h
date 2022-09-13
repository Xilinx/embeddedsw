/******************************************************************************
* Copyright (c) 2020 - 2022 Xilinx, Inc.  All rights reserved.
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
* 5.0   bm      07/06/22 Refactor versal and versal_net code
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
#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */
#define XSECURE_SSS_SBI_MASK	(0xF00000U)
#define XSECURE_SSS_AES_MASK	(0xF000U)
#define XSECURE_SSS_DMA1_MASK	(0xF0U)
#define XSECURE_SSS_DMA0_MASK	(0xFU)
#define XSECURE_SSS_SRC_SEL_MASK	(0xFU)
#define XSECURE_SSS_SBI_DMA0_VAL	(0x500000U)
#define XSECURE_SSS_SBI_DMA1_VAL	(0xB00000U)
#define XSECURE_SSS_AES_DMA0_VAL	(0xE000U)
#define XSECURE_SSS_AES_DMA1_VAL	(0x5000U)

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
