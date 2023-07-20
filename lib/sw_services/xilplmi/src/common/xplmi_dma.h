/******************************************************************************
* Copyright (c) 2018 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
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
* 1.01  bsv  04/18/2019 Added APIs to support non blocking DMA
*       vnsl 04/22/2019 Added API to get DMA instance
*       kc   05/17/2019 Added ECC initiation function using PMC DMA
* 1.02  bsv  04/04/2020 Code clean up
*       bsv  04/07/2020 Renamed DMA to PMCDMA
* 1.03  bm   09/02/2020 Add XPlmi_MemSet API
*       bsv  09/30/2020 Added wait for non blocking SBI DMA
*       bm   10/14/2020 Code clean up
* 1.04  bsv  07/16/2021 Fix doxygen warnings
*       bsv  08/13/2021 Code clean up to reduce elf size
* 1.05  bm   01/20/2022 Fix compilation warnings in Xil_SMemCpy
* 1.06  bm   07/06/2022 Refactor versal and versal_net code
* 1.09  ng   07/06/2023 Added support for SDT flow
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
#include "xpmcdma.h"

/**@cond xplmi_internal
 * @{
 */

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/
/** DMA XFER flags */
#define XPLMI_SRC_CH_AXI_FIXED		(0x1U)
#define XPLMI_DMA_SRC_NONBLK		(0x1U << 1U)
#define XPLMI_DST_CH_AXI_FIXED		((u32)0x1U << 16U)
#define XPLMI_DMA_DST_NONBLK		((u32)0x1U << 17U)
#define XPLMI_PMCDMA_0			(0x100U)
#define XPLMI_PMCDMA_1			(0x200U)
#define XPLMI_DMA_SRC_NPI		(0x4U)

/* SSS configurations and masks */
#define XPLMI_SSSCFG_DMA0_MASK		(0x0000000FU)
#define XPLMI_SSSCFG_DMA1_MASK		(0x000000F0U)
#define XPLMI_SSSCFG_SBI_MASK		(0x00F00000U)

#define XPLMI_SSS_SBI_DMA0			(0x00500000U)
#define XPLMI_SSS_SBI_DMA1			(0x00B00000U)

#define XPLMI_SSS_DMA1_DMA1			(0x00000090U)
#define XPLMI_SSS_DMA1_SBI			(0x000000E0U)
#define XPLMI_SSS_DMA1_PZM			(0x00000040U)

#define XPLMI_SSS_DMA0_DMA0			(0x0000000DU)
#define XPLMI_SSS_DMA0_SBI			(0x0000000BU)
#define XPLMI_SSS_DMA0_PZM			(0x00000003U)

#define XPLMI_DATA_INIT_PZM			(0xDEADBEEFU)
#define XPLMI_PZM_WORD_LEN			(16U)
#define XPLMI_SET_CHUNK_SIZE			(128U)
#define XPLMI_WORD_LEN_MASK			(0x3U)
#define XPLMI_WORD_LEN_SHIFT			(0x2U)

/* Type Definition of XPlmi_WaitForDmaDone */
typedef int (*XPlmi_WaitForDmaDone_t)(XPmcDma *DmaPtr, XPmcDma_Channel Channel);

#ifndef SDT
#define PMCDMA_0_DEVICE ((u32)PMCDMA_0_DEVICE_ID)
#define PMCDMA_1_DEVICE ((u32)PMCDMA_1_DEVICE_ID)
#else
#define PMCDMA_0_DEVICE PMCDMA_0_DEVICE_ID
#define PMCDMA_1_DEVICE PMCDMA_1_DEVICE_ID
#endif

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int XPlmi_DmaInit(void);
#ifndef SDT
XPmcDma *XPlmi_GetDmaInstance(u32 DeviceId);
#else
XPmcDma *XPlmi_GetDmaInstance(UINTPTR BaseAddress);
#endif
int XPlmi_DmaXfr(u64 SrcAddr, u64 DestAddr, u32 Len, u32 Flags);
int XPlmi_SbiDmaXfer(u64 DestAddr, u32 Len, u32 Flags);
int XPlmi_DmaSbiXfer(u64 SrcAddr, u32 Len, u32 Flags);
int XPlmi_EccInit(u64 Addr, u32 Len);
int XPlmi_InitNVerifyMem(u64 Addr, u32 Len);
int XPlmi_WaitForNonBlkSrcDma(u32 DmaFlags);
int XPlmi_WaitForNonBlkDestDma(u32 DmaFlags);
int XPlmi_WaitForNonBlkDma(u32 DmaFlags);
void XPlmi_SetMaxOutCmds(u8 Val);
int XPlmi_MemSet(u64 DestAddr, u32 Val, u32 Len);
int XPlmi_MemSetBytes(void *const DestPtr, u32 DestLen, u8 Val, u32 Len);
int XPlmi_MemCpy64(u64 DestAddr, u64 SrcAddr, u32 Len);

/**
 * @}
 * @endcond
 */

#ifdef __cplusplus
}
#endif

#endif  /* XPLMI_DMA_H */
