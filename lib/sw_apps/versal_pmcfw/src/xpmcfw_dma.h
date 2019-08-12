/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* FITNESS FOR A PRTNICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
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
* @file xpmcfw_dma.h
*
* This is the file which contains common code for the PMCFW.
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

#ifndef XPMCFW_DMA_H
#define XPMCFW_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xpmcfw_default.h"
#include "xcsudma.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

#define XPMCFW_SRC_CH_AXI_FIXED 	(0x1U)
#define XPMCFW_READ_AXI_FIXED		(0x1U)
#define XPMCFW_DST_CH_AXI_FIXED 	(0x1U<<16U)
#define XPMCFW_PMCDMA_0			(0x100U)
#define XPMCFW_PMCDMA_1			(0x200U)
#define XPMCFW_DMA_SRC_NONBLK	(0x2U)
#define XPMCFW_DMA_DST_NONBLK   (0x1U<<17U)
#define XPMCFW_DMA_SRC_NPI		(0x4U)
#define XPMCFW_DMA_DST_TYPE_SHIFT	(18U)
#define XPMCFW_DMA_DST_TYPE_MASK	(0x3U<<18U)
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CSUDMA_0_DEVICE_ID	XPAR_XCSUDMA_0_DEVICE_ID /* CSUDMA device Id */
#define CSUDMA_1_DEVICE_ID	XPAR_XCSUDMA_1_DEVICE_ID /* CSUDMA device Id */
#define CSUDMA_LOOPBACK_CFG	0x0000000F	/**< LOOP BACK configuration */

/* SSS configurations and masks */
#define XPMCFW_SSSCFG_DMA0_MASK		(0x0000000FU)
#define XPMCFW_SSSCFG_DMA1_MASK		(0x000000F0U)
#define XPMCFW_SSSCFG_PTP1_MASK		(0x00000F00U)
#define XPMCFW_SSSCFG_AES_MASK		(0x0000F000U)
#define XPMCFW_SSSCFG_SHA_MASK		(0x000F0000U)
#define XPMCFW_SSSCFG_SBI_MASK		(0x00F00000U)

#define XPMCFW_SSS_SBI_DMA0			(0x00500000U)
#define XPMCFW_SSS_SBI_DMA1			(0x00B00000U)

#define XPMCFW_SSS_SHA_DMA0			(0x000C0000U)
#define XPMCFW_SSS_SHA_DMA1			(0x00070000U)

#define XPMCFW_SSS_AES_DMA0			(0x0000E000U)
#define XPMCFW_SSS_AES_DMA1			(0x00005000U)

#define XPMCFW_SSS_PTPI_DMA0		(0x00000D00U)
#define XPMCFW_SSS_PTPI_DMA1		(0x00000A00U)

#define XPMCFW_SSS_DMA1_DMA1		(0x00000090U)
#define XPMCFW_SSS_DMA1_AES			(0x00000070U)
#define XPMCFW_SSS_DMA1_SBI			(0x000000E0U)

#define XPMCFW_SSS_DMA0_DMA0		(0x0000000DU)
#define XPMCFW_SSS_DMA0_AES			(0x00000006U)
#define XPMCFW_SSS_DMA0_SBI			(0x0000000BU)

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
XStatus XPmcFw_DmaInit();
XStatus XPmcFw_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
XStatus XPmcFw_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags);
XStatus XPmcFw_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags);
XStatus XPmcFw_EccInit(u64 Addr, u32 Len);
u32 XPmcFw_StartDma(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags,
		XCsuDma** DmaPtrAddr);
void XPmcFw_WaitForNonBlkSrcDma(void);
#ifdef __cplusplus
}
#endif

#endif  /* XPMCFW_DMA_H */
