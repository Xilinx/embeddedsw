/******************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc. All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
* 
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xplmi_dma.h
*
* This is the file which contains common code for the PLM.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   02/21/2017 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XPLMI_DMA_H
#define XPLMI_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xcsudma.h"
#include "xplmi_hw.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/** DMA XFER flags */
#define XPLMI_SRC_CH_AXI_FIXED		(0x1U)
#define XPLMI_DMA_SRC_NONBLK		(0x1U<<1U)
#define XPLMI_DST_CH_AXI_FIXED		(0x1U<<16U)
#define XPLMI_DMA_DST_NONBLK		(0x1U<<17U)
#define XPLMI_PMCDMA_0				(0x100U)
#define XPLMI_PMCDMA_1				(0x200U)
#define XPLMI_DMA_SRC_NPI			(0x4U)
#define XPLMI_DMA_DST_TYPE_SHIFT	(18U)
#define XPLMI_DMA_DST_TYPE_MASK		(0x3U<<18U)
#define XPLMI_READ_AXI_FIXED		(0x1U)

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CSUDMA_0_DEVICE_ID	XPAR_XCSUDMA_0_DEVICE_ID /* CSUDMA device Id */
#define CSUDMA_1_DEVICE_ID	XPAR_XCSUDMA_1_DEVICE_ID /* CSUDMA device Id */
#define CSUDMA_LOOPBACK_CFG	0x0000000F	/**< LOOP BACK configuration */

/* SSS configurations and masks */
#define XPLMI_SSSCFG_DMA0_MASK		(0x0000000FU)
#define XPLMI_SSSCFG_DMA1_MASK		(0x000000F0U)
#define XPLMI_SSSCFG_PTP1_MASK		(0x00000F00U)
#define XPLMI_SSSCFG_AES_MASK		(0x0000F000U)
#define XPLMI_SSSCFG_SHA_MASK		(0x000F0000U)
#define XPLMI_SSSCFG_SBI_MASK		(0x00F00000U)

#define XPLMI_SSS_SBI_DMA0			(0x00500000U)
#define XPLMI_SSS_SBI_DMA1			(0x00B00000U)

#define XPLMI_SSS_SHA_DMA0			(0x000C0000U)
#define XPLMI_SSS_SHA_DMA1			(0x00070000U)

#define XPLMI_SSS_AES_DMA0			(0x0000E000U)
#define XPLMI_SSS_AES_DMA1			(0x00005000U)

#define XPLMI_SSS_PTPI_DMA0			(0x00000D00U)
#define XPLMI_SSS_PTPI_DMA1			(0x00000A00U)

#define XPLMI_SSS_DMA1_DMA1			(0x00000090U)
#define XPLMI_SSS_DMA1_AES			(0x00000070U)
#define XPLMI_SSS_DMA1_SBI			(0x000000E0U)
#define XPLMI_SSS_DMA1_PZM			(0x00000040U)

#define XPLMI_SSS_DMA0_DMA0			(0x0000000DU)
#define XPLMI_SSS_DMA0_AES			(0x00000006U)
#define XPLMI_SSS_DMA0_SBI			(0x0000000BU)
#define XPLMI_SSS_DMA0_PZM			(0x00000003U)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_DmaInit();
XCsuDma *XPlmi_GetDmaInstance(u32 DeviceId);
int XPlmi_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
int XPlmi_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags);
int XPlmi_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags);
int XPlmi_EccInit(u64 Addr, u32 Len);
int XPlmi_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
		XCsuDma** DmaPtrAddr);
void XPlmi_WaitForNonBlkSrcDma(void);
void XPlmi_WaitForNonBlkDma(void);
void XPlmi_SetMaxOutCmds(u32 Val);
#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_DMA_H */
