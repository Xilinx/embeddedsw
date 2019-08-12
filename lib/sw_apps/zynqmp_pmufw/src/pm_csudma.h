/*
 * Copyright (C) 2018 - 2019 Xilinx, Inc.  All rights reserved.
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
