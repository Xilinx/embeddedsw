/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/


/*****************************************************************************/
/**
*
* @file xcbr_csu_dma.h
*
* Contains declarations for CSU DMA initialization
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  kc   07/22/14 Initial release
* 2.0   bv   01/29/17 XFSBL_CSU_SSS_SRC_DEST_DMA and
*                     XFSBL_CSU_SSS_DMA_MASK masks
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

/**************************** Type Definitions *******************************/

/**************************** Macros Definitions *****************************/
#define XFSBL_CSU_SSS_SRC_SRC_DMA    0x5U
#define XFSBL_CSU_SSS_SRC_DEST_DMA	 0x50U
#define XFSBL_CSU_SSS_DMA_MASK		 0XF000U

/************************** Function Prototypes ******************************/
u32 XFsbl_CsuDmaInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XFSBL_CSU_DMA_H*/
