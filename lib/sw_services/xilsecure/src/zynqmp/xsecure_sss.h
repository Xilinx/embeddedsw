/******************************************************************************
*
* Copyright (C) 2020 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
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

#ifdef __cplusplus
}
#endif

#endif /* XSECURE_SSS_H_ */
/**@}*/
