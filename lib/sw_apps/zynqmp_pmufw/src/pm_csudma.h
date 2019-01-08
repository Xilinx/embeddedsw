/*
* Copyright (c) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
 */


#ifndef PM_CSUDMA_H_
#define PM_CSUDMA_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xcsudma.h"
#include "pm_common.h"

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
/* CSU DMA device Id */
#define CSUDMA_DEVICE_ID	XPAR_XCSUDMA_0_DEVICE_ID
/* CSU DMA Source control */
#define CSUDMA_SRC_CTRL		(XPAR_XCSUDMA_0_BASEADDR + 0xCU)
/* CSU DMA Destination control */
#define CSUDMA_DEST_CTRL	(XPAR_XCSUDMA_0_BASEADDR + 0x80CU)
/*CSU DMA APB error response mask */
#define CSUDMA_APB_ERR_RESP_MASK	0x01000000U
/* CSU SSS_CFG Offset */
#define CSU_SSS_CONFIG_OFFSET	0x00000008U
/* LOOP BACK configuration macro */
#define CSUDMA_LOOPBACK_CFG	0x00000050U

extern XCsuDma CsuDma;          /* Instance of the Csu_Dma Device */

XStatus PmDmaInit(void);
void PmDma64BitTransfer(u32 DstAddrLow, u32 DstAddrHigh,
			 u32 SrcAddrLow, u32 SrcAddrHigh, u32 Size);
void PmSetCsuDmaLoopbackMode(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_CSUDMA_H_ */
