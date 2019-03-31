/******************************************************************************
 * Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 *
 *****************************************************************************/
/*****************************************************************************/
/* MODIFICATION HISTORY:
 *
 * Ver   Who  Date        Changes
 * ----- ---- -------- -------------------------------------------------------
 * 5.0   Nava 11/05/18 Added full bitstream loading support for versal Platform
 *
 * </pre>
 *
 * @note
 *
 ******************************************************************************/

#ifndef XILFPGA_CFI_H
#define XILFPGA_CFI_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xil_io.h"
#include "xcframe.h"
#include "xcfupmc.h"
#include "xcsudma.h"
#include "sleep.h"
#include "xilfpga.h"
/************************** Constant Definitions *****************************/

/************************** Function Prototypes ******************************/
/**************************** Type Definitions *******************************/
/**
 * Structure to store the lower level interface details used for PL programming.
 * @CfupmcIns	The XCfupmc driver instance data structure.
 * @CframeIns   The XCframe driver instance data structure.
 * @PmcDmaIns	The XCsuDma driver instance data structure.
 */
typedef struct {
	XCfupmc CfupmcIns;
	XCframe CframeIns;
	XCsuDma PmcDmaIns;
} XFpga_Info;

/**
 * Structure to store the PL Write Image details.
 * @BitstreamAddr	Bitstream image base address.
 * @AddrPtr_Size	Aes key address which is used for Decryption (or)
 *			In none Secure Bitstream case it is used to store the size
 *			of the Bitstream Image.
 * @Flags		Flags are used to specify the type of Bitstream file.
 *			* BIT(0) - Bitstream type
 *                                     * 0 - Full Bitstream.
 *                                     * 1 - Reserved.
 *			* BIT(31) to BIT(1) - Reserved.
 */
typedef struct {
		UINTPTR BitstreamAddr;
		UINTPTR	AddrPtr_Size;
		u32 Flags;
}XFpga_Write;

/**
 * Structure to store the PL Read Image details.
 * @ReadbackAddr	Address which is used to store the PL readback data.
 * @ConfigReg		Configuration register value to be returned (or)
 * 			The number of Fpga configuration frames to read
 */
typedef struct {
		UINTPTR ReadbackAddr;
		u32 ConfigReg_NumFrames;
}XFpga_Read;

/************************** Variable Definitions *****************************/

/***************** Macros (Inline Functions) Definitions *********************/
/* TRIM Types */
#define XFPGA_FABRIC_TRIM_VGG		(0x1U)
#define XFPGA_FABRIC_TRIM_CRAM		(0x2U)
#define XFPGA_FABRIC_TRIM_BRAM		(0x3U)
#define XFPGA_FABRIC_TRIM_URAM		(0x4U)

/**
 * EFUSE_CACHE Base Address
 */
#define EFUSE_CACHE_BASEADDR      0XF1250000
/**
 * Register: EFUSE_CACHE_TRIM_CFRM_VGG_0
 */
#define EFUSE_CACHE_TRIM_CFRM_VGG_0    ( ( EFUSE_CACHE_BASEADDR ) + 0X000001B4 )

#define EFUSE_CACHE_TRIM_CFRM_VGG_0_TRIM_CFRM_VGG_31_0_SHIFT   0
#define EFUSE_CACHE_TRIM_CFRM_VGG_0_TRIM_CFRM_VGG_31_0_WIDTH   32
#define EFUSE_CACHE_TRIM_CFRM_VGG_0_TRIM_CFRM_VGG_31_0_MASK    0XFFFFFFFF

/**
 * Register: EFUSE_CACHE_TRIM_CFRM_VGG_1
 */
#define EFUSE_CACHE_TRIM_CFRM_VGG_1    ( ( EFUSE_CACHE_BASEADDR ) + 0X000001B8 )

#define EFUSE_CACHE_TRIM_CFRM_VGG_1_TRIM_CFRM_VGG_63_32_SHIFT   0
#define EFUSE_CACHE_TRIM_CFRM_VGG_1_TRIM_CFRM_VGG_63_32_WIDTH   32
#define EFUSE_CACHE_TRIM_CFRM_VGG_1_TRIM_CFRM_VGG_63_32_MASK    0XFFFFFFFF

/**
 * Register: EFUSE_CACHE_TRIM_CFRM_VGG_2
 */
#define EFUSE_CACHE_TRIM_CFRM_VGG_2    ( ( EFUSE_CACHE_BASEADDR ) + 0X000001BC )

#define EFUSE_CACHE_TRIM_CFRM_VGG_2_TRIM_CFRM_VGG_95_64_SHIFT   0
#define EFUSE_CACHE_TRIM_CFRM_VGG_2_TRIM_CFRM_VGG_95_64_WIDTH   32
#define EFUSE_CACHE_TRIM_CFRM_VGG_2_TRIM_CFRM_VGG_95_64_MASK    0XFFFFFFFF

/**
 * Register: EFUSE_CACHE_TRIM_CRAM
 */
#define EFUSE_CACHE_TRIM_CRAM    ( ( EFUSE_CACHE_BASEADDR ) + 0X000001C0 )

#define EFUSE_CACHE_TRIM_CRAM_TRIM_CRAM_31_0_SHIFT   0
#define EFUSE_CACHE_TRIM_CRAM_TRIM_CRAM_31_0_WIDTH   32
#define EFUSE_CACHE_TRIM_CRAM_TRIM_CRAM_31_0_MASK    0XFFFFFFFF

/**
 * Register: EFUSE_CACHE_TRIM_BRAM
 */
#define EFUSE_CACHE_TRIM_BRAM    ( ( EFUSE_CACHE_BASEADDR ) + 0X00000098 )

#define EFUSE_CACHE_TRIM_BRAM_TRIM_BRAM_31_0_SHIFT   0
#define EFUSE_CACHE_TRIM_BRAM_TRIM_BRAM_31_0_WIDTH   32
#define EFUSE_CACHE_TRIM_BRAM_TRIM_BRAM_31_0_MASK    0XFFFFFFFF

/**
 * Register: EFUSE_CACHE_TRIM_URAM
 */
#define EFUSE_CACHE_TRIM_URAM    ( ( EFUSE_CACHE_BASEADDR ) + 0X0000009C )

#define EFUSE_CACHE_TRIM_URAM_TRIM_URAM_31_0_SHIFT   0
#define EFUSE_CACHE_TRIM_URAM_TRIM_URAM_31_0_WIDTH   32
#define EFUSE_CACHE_TRIM_URAM_TRIM_URAM_31_0_MASK    0XFFFFFFFF

/**
 * PMC_GLOBAL Base Address
 */
#define PMC_GLOBAL_BASEADDR      0XF1110000

/**
 * Register: PMC_GLOBAL_PMC_SSS_CFG
 */
#define PMC_GLOBAL_PMC_SSS_CFG    ( ( PMC_GLOBAL_BASEADDR ) + 0X00000500 )

#define PMC_GLOBAL_PMC_SSS_CFG_DMA1_CFG_SHIFT   4
#define PMC_GLOBAL_PMC_SSS_CFG_DMA1_CFG_WIDTH   4
#define PMC_GLOBAL_PMC_SSS_CFG_DMA1_CFG_MASK    0X000000F0

#define PMC_GLOBAL_PMC_SSS_CFG_DMA0_CFG_SHIFT   0
#define PMC_GLOBAL_PMC_SSS_CFG_DMA0_CFG_WIDTH   4
#define PMC_GLOBAL_PMC_SSS_CFG_DMA0_CFG_MASK    0X0000000F

/* SSS configurations and masks */
#define XFPGA_SSSCFG_DMA0_MASK		(0x0000000FU)
#define XFPGA_SSSCFG_DMA1_MASK		(0x000000F0U)
#define XFPGA_SSS_DMA0_DMA0		(0x0000000DU)
#define XFPGA_SSS_DMA1_DMA1		(0x00000090U)

/*XCSUDMA Burst Type */
#define XFPGA_CSUDMA_INCR_BURST		(0x00000000U)	/* INCR type burst */
#define XFPGA_CSUDMA_FIXED_BURST	(0x00000001U)	/* FIXED type burst */

/* PMC DMA Type */
#define XFPGA_DMATYPEIS_PMCDMA0		XPAR_PSU_PMCDMA_0_DEVICE_ID
#define XFPGA_DMATYPEIS_PMCDMA1		XPAR_PSU_PMCDMA_1_DEVICE_ID
#define XFPGA_PMC_DMA_NONE		(3U)

/* Return Codes */
#define XFPGA_CFI_SUCCESS		(0x0U)
#define XFPGA_CFI_FAIL			(0x1U)
#define XFPGA_ERR_CFU_LOOKUP		(0x2U)
#define XFPGA_ERR_CFU_CFG		(0x3U)
#define XFPGA_ERR_CFU_SELFTEST		(0x4U)
#define XFPGA_ERR_CFRAME_LOOKUP 	(0x5U)
#define XFPGA_ERR_CFRAME_CFG		(0x6U)
#define XFPGA_ERR_CFRAME_SELFTEST	(0x7U)
#define XFPGA_ERR_CFI_LOAD		(0x8U)
#define XFPGA_ERR_DMA_LOOKUP		(0x9U)
#define XFPGA_ERR_DMA_CFG		(0xAU)
#define XFPGA_ERR_DMA_SELFTEST		(0xBU)
#define XFPGA_ERR_STREAM_BUSY		(0xCU)
#define XFPGA_ERR_CFU_SETTINGS		(0xDU)
#define XFPGA_ERR_DMA_XFR			(0XEU)


/* CFI Error Update Macro */
#define XFPGA_CFI_ERR_MASK		(0xFF00U)
#define XFPGA_ERR_MODULE_MASK		(0xFFFF0000U)
#define XFPGA_CFI_UPDATE_ERR(XfpgaCfiErr, ModuleErr)          \
                ((ModuleErr << 16) & XFPGA_ERR_MODULE_MASK) + \
                ((XfpgaCfiErr << 8) & XFPGA_CFI_ERR_MASK)

/************************** Function Prototypes ******************************/

/*****************************************************************************/

#ifdef __cplusplus
}
#endif

#endif  /* XILFPGA_CFI_H */
/** @} */
