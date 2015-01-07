/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* XILINX CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
*******************************************************************************/

/*****************************************************************************/
/**
*
* @file xcbr_csu_dma.h
*
* Contains enums/typedefs and macros for CSU DMA driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/22/14 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_CSU_DMA_H
#define XFSBL_CSU_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xfsbl_hw.h"

/**************************** Macros Definitions *****************************/
#define XASSERT_VOID(expression)
#define mb() 	asm("dsb sy")

u8 XFsbl_RsaSha3Array[512];
/**************************** Type Definitions *******************************/

/**
* Definition for DMA channels.
*/
typedef enum
{
	XFSBL_CSU_DMA_SRC = 0x000U,
    XFSBL_CSU_DMA_DST = 0x800U
}XFSBL_CSU_DMA_CHANNEL;

/**
* Definition for DMA registers.
*/
typedef enum
{
    XFSBL_CSU_DMA_ADDR_REG	 	  = 0x00U,
    XFSBL_CSU_DMA_SIZE_REG  	 	  = 0x04U,
    XFSBL_CSU_DMA_STATUS_REG	 	  = 0x08U,
    XFSBL_CSU_DMA_CTRL_REG         = 0x0CU,
    XFSBL_CSU_DMA_CRC_REG          = 0x10U,
    XFSBL_CSU_DMA_INT_STATUS_REG	  = 0x14U,
    XFSBL_CSU_DMA_INT_ENABLE_REG   = 0x18U,
    XFSBL_CSU_DMA_INT_DISABLE_REG  = 0x1CU,
    XFSBL_CSU_DMA_INT_MASK_REG     = 0x20U,
}XFSBL_CSU_DMA_REG_OFFSET;

/**
* Definition for DMA EOP.
*/
enum
{
    XFSBL_CSU_DMA_SIZE_EOP         = 0x1U
};

/**
* Definition for DMA Status reg bits.
*/
typedef enum
{
    XFSBL_CSU_DMA_STATUS_BUSY      		= (0x1U << 0x0U),
    XFSBL_CSU_DMA_STATUS_DONE_CNT_SHIFT 	= (13U),
    XFSBL_CSU_DMA_STATUS_DONE_CNT_MASK 	= (0x7U << XFSBL_CSU_DMA_STATUS_DONE_CNT_SHIFT)
}XFSBL_CSU_DMA_STATUS;

/**
* Definition for DMA Control reg bits.
*/
typedef enum
{
	XFSBL_CSU_DMA_CTRL_PAUSE_MEM              = (0x1U << 0x0U),
    XFSBL_CSU_DMA_CTRL_PAUSE_STREAM           = (0x1U << 0x1U),
    XFSBL_CSU_DMA_CTRL_FIFO_THRESH_SHIFT      = 0x2U,
    XFSBL_CSU_DMA_CTRL_TIMEOUT_SHIFT          = 10U,
    XFSBL_CSU_DMA_CTRL_AXI_BURST_INCR         = (0x0U << 22U),
    XFSBL_CSU_DMA_CTRL_AXI_BURST_FIXED        = (0x1U << 22U),
    XFSBL_CSU_DMA_CTRL_ENDIANNESS             = (0x1U << 23U),
    XFSBL_CSU_DMA_CTRL_ERR_RESPONSE           = (0x1U << 24U),
    XFSBL_CSU_DMA_CTRL_IF_FIFO_THRESH_SHIFT   = 25U
}XFSBL_CSU_DMA_CTRL;

/**
* Definition for DMA Interrupt reg bits.
*/
typedef enum
{
    XFSBL_CSU_DMA_INT_INVALID_APB_ACCESS          = (0x1U << 0x5U),
    XFSBL_CSU_DMA_INT_FIFO_THRESH_HIT             = (0x1U << 0x4U),
    XFSBL_CSU_DMA_INT_DMA_TIMEOUT                 = (0x1U << 0x3U),
    XFSBL_CSU_DMA_INT_AXI_RD__ERR                 = (0x1U << 0x2U),
    XFSBL_CSU_DMA_INT_DMA_DONE                    = (0x1U << 0x1U),
    XFSBL_CSU_DMA_INT_DMA_MEM_DONE                = (0x1U << 0x0U)
}XFSBL_CSU_DMA_INT;

/*
 * Definition for SSS reg Source bits.
 */
typedef enum
{
    XFSBL_CSU_SSS_SRC_PCAP 		= 0x3U,
    XFSBL_CSU_SSS_SRC_SRC_DMA 	= 0x5U,
    XFSBL_CSU_SSS_SRC_AES  		= 0xAU,
    XFSBL_CSU_SSS_SRC_PSTP 		= 0xCU,
    XFSBL_CSU_SSS_SRC_NONE 		= 0x0U,
	XFSBL_CSU_SSS_SRC_MASK 		= 0xFU
}XFSBL_CSU_SSS_SRC;

/**
* Definition for SSS reg Destination bits.
*/
typedef enum
{
    XFSBL_CSU_SSS_PCAP_SHIFT = 0U,
    XFSBL_CSU_SSS_DMA_SHIFT = 4U,
    XFSBL_CSU_SSS_AES_SHIFT = 8U,
    XFSBL_CSU_SSS_SHA_SHIFT = 12U,
    XFSBL_CSU_SSS_PSTP_SHIFT = 16U
}XFSBL_CSU_SSS_DEST_SHIFT;


/**************************** Macros Definitions *****************************/

/**
* Definition for DMA inline functions
*/
static inline void XFsbl_DmaControl(XFSBL_CSU_DMA_CHANNEL Channel, u32 Val)
{
	XFsbl_Out32((CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_CTRL_REG), Val);
}

static inline void XFsbl_DmaEndian(XFSBL_CSU_DMA_CHANNEL Channel, u32 Enable)
{
	u32 Value = XFsbl_In32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_CTRL_REG);
	if(Enable)
	{
		Value |= XFSBL_CSU_DMA_CTRL_ENDIANNESS;
	}
	else
	{
		Value &= ~XFSBL_CSU_DMA_CTRL_ENDIANNESS;
	}

	XFsbl_Out32((CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_CTRL_REG), Value);
}

static inline int XFsbl_DmaIsBusy(XFSBL_CSU_DMA_CHANNEL Channel)
{
	return (XFsbl_In32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_STATUS_REG) &
			XFSBL_CSU_DMA_STATUS_BUSY);
}

static inline unsigned int XFsbl_DmaDoneCount(XFSBL_CSU_DMA_CHANNEL Channel)
{
	u32 Count;

	Count = XFsbl_In32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_STATUS_REG);
	Count &= XFSBL_CSU_DMA_STATUS_DONE_CNT_MASK;
	Count >>= XFSBL_CSU_DMA_STATUS_DONE_CNT_SHIFT;
	return Count;
}

static inline void XFsbl_DmaZeroDoneCount(XFSBL_CSU_DMA_CHANNEL Channel)
{
	XFsbl_Out32(CSUDMA_BASEADDR + Channel + XFSBL_CSU_DMA_STATUS_REG,
			XFSBL_CSU_DMA_STATUS_DONE_CNT_MASK);
	/** ASM Code */
	mb();
}

/* The CRC function is only available on SRC channels.  */
static inline u32 XFsbl_DmaSrcGetCrc(void)
{
	return (XFsbl_In32(CSUDMA_BASEADDR + XFSBL_CSU_DMA_SRC +
				XFSBL_CSU_DMA_CRC_REG));
}

static inline void XFsbl_DmaSrcSetCrc(u32 Crc)
{
	XFsbl_Out32(CSUDMA_BASEADDR + XFSBL_CSU_DMA_SRC +
					XFSBL_CSU_DMA_CRC_REG, Crc);
	/** ASM Code */
	mb();
}

void XFsbl_CsuDmaWaitForDone(XFSBL_CSU_DMA_CHANNEL Channel);
void XFsbl_CsuDmaStart(XFSBL_CSU_DMA_CHANNEL Channel, u32 Addr, u32 Size);
void XFsbl_CsuDmaXfer(u32 SrcAddr, u32 DestAddr, u32 ImgLen);


/**
* Definition for SSS inline functions
*/
static inline u32 XFsbl_SssInputPcap(XFSBL_CSU_SSS_SRC Src)
{
    Src &= XFSBL_CSU_SSS_SRC_MASK;
    return (Src << XFSBL_CSU_SSS_PCAP_SHIFT);
}

static inline u32 XFsbl_SssInputDstDma(XFSBL_CSU_SSS_SRC Src)
{
    Src &= XFSBL_CSU_SSS_SRC_MASK;
    return (Src << XFSBL_CSU_SSS_DMA_SHIFT);
}

static inline u32 XFsbl_SssInputAes(XFSBL_CSU_SSS_SRC Src)
{
    Src &= XFSBL_CSU_SSS_SRC_MASK;
    return (Src << XFSBL_CSU_SSS_AES_SHIFT);
}

static inline u32 XFsbl_SssInputSha3(XFSBL_CSU_SSS_SRC Src)
{
    Src &= XFSBL_CSU_SSS_SRC_MASK;
    return (Src << XFSBL_CSU_SSS_SHA_SHIFT);
}

static inline u32 XFsbl_SssInputPstp(XFSBL_CSU_SSS_SRC Src)
{
    Src &= XFSBL_CSU_SSS_SRC_MASK;
    return (Src << XFSBL_CSU_SSS_PSTP_SHIFT);
}

static inline void XFsbl_SssSetup(u32 Cfg)
{
    XFsbl_Out32(CSU_CSU_SSS_CFG, Cfg);
}

#endif /* XFSBL_CSU_DMA_H*/
