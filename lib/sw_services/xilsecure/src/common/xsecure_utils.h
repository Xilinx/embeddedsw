/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* @file xsecure_utils.h
* @addtogroup xsecure_common_apis XILSECURE_UTILITIES
* @{
* @cond xsecure_internal
* This file contains common APIs which are used across the library.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     03/12/19 Initial Release
* 4.1	kal	05/20/19 Updated doxygen tags
*       psl     08/05/19 Fixed MISRA-C violation
* </pre>
* @endcond
******************************************************************************/

#ifndef XSECURE_UTILS_H_
#define XSECURE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif
/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xparameters.h"
#include "xil_types.h"
#include "sleep.h"
#include "xstatus.h"
#include "xil_assert.h"

/************************** Constant Definitions ****************************/
/** @cond xsecure_internal */
#ifdef versal
#define XSECURE_VERSAL		/**< Definition for VERSAL */
#else
#define XSECURE_ZYNQMP		/**< Definition for ZynqMP */
#endif

#define XSECURE_RESET_SET		(1U) /**< To set the core into reset */
#define XSECURE_RESET_UNSET		(0U)
					/**< To take the core out of reset */

#define XSECURE_SSS_CFG_LEN_IN_BITS	(4U) /**< Length is bits */

#ifdef XSECURE_VERSAL
#define XSECURE_SSS_ADDRESS		(0xF1110500U) /**< SSS base address */
#define XSECURE_SSS_MAX_SRCS		(8U)	/**< Maximum resources */
#else
#define XSECURE_CSU_REG_BASE_ADDR	(0xFFCA0000U)
					/**< CSU base address */
#define XSECURE_SSS_ADDRESS		(0xFFCA0008U)/**< SSS base address */
#define XSECURE_SSS_MAX_SRCS		(5U)	/**< Maximum resources */
#endif

#define XSECURE_TIMEOUT_MAX		(0x1FFFFFU)

/***************************** Type Definitions******************************/
/**
 * Instance structure of secure stream switch
 */
typedef struct {
	u32 Address; /**< Address of SSS CFG register */
}XSecure_Sss;

/* Sources to be selected to configure secure stream switch */
#ifdef XSECURE_VERSAL
typedef enum {
	XSECURE_SSS_DMA0,
	XSECURE_SSS_DMA1,
	XSECURE_SSS_PTPI,
	XSECURE_SSS_AES,
	XSECURE_SSS_SHA,
	XSECURE_SSS_SBI,
	XSECURE_SSS_PZI,
	XSECURE_SSS_INVALID
}XSecure_SssSrc;
#else
typedef enum{
	XSECURE_SSS_PCAP,
	XSECURE_SSS_DMA0,
	XSECURE_SSS_AES,
	XSECURE_SSS_SHA,
	XSECURE_SSS_INVALID
}XSecure_SssSrc;
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/*****************************************************************************/
/**
* Read from the register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
*
* @return	The value read from the register.
*
* @note		C-Style signature:
*		u32 XSecure_ReadReg(u32 BaseAddress, u16 RegOffset)
*
******************************************************************************/
#define XSecure_ReadReg(BaseAddress, RegOffset) \
				Xil_In32((BaseAddress) + (RegOffset))

/***************************************************************************/
/**
* Write to the register.
*
* @param	BaseAddress contains the base address of the device.
* @param	RegOffset contains the offset from the base address of
*		the device.
* @param	RegisterValue is the value to be written to the register
*
* @return	None.
*
* @note		C-Style signature:
*			void XSecure_WriteReg(u32 BaseAddress, u16 RegOffset,
*			u16 RegisterValue)
*
******************************************************************************/
#define XSecure_WriteReg(BaseAddress, RegOffset, RegisterValue) \
			Xil_Out32((BaseAddress) + (RegOffset), (RegisterValue))

#define XSecure_In32(Addr)			Xil_In32((Addr))

#define XSecure_In64(Addr)			Xil_In64((Addr))

#define XSecure_Out32(Addr, Data)		Xil_Out32((Addr), (Data))

#define XSecure_Out64(Addr, Data)		Xil_Out64((Addr), (Data))
/**  @endcond */
/************************** Function Prototypes ******************************/

void XSecure_SetReset(u32 BaseAddress, u32 Offset);
void XSecure_ReleaseReset(u32 BaseAddress, u32 Offset);

void XSecure_SssInitialize(XSecure_Sss *InstancePtr);
u32 XSecure_SssAes(XSecure_Sss *InstancePtr, XSecure_SssSrc InputSrc,
		XSecure_SssSrc OutputSrc);
u32 XSecure_SssSha(XSecure_Sss *InstancePtr, u16 DmaId);
u32 XSecure_SssDmaLoopBack(XSecure_Sss *InstancePtr, u16 DmaId);
/** @cond xsecure_internal */
void* XSecure_MemCpy(void * DestPtr, const void * SrcPtr, u32 Len);
/**  @endcond */
#ifdef __cplusplus
}
#endif

#endif /* XSECURE_UTILS_H_ */
/**@}*/
