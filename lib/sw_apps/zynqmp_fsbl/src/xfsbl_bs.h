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
*******************************************************************************/
/*****************************************************************************/
/**
*
* @file xfsbl_bs.h
*
* This is the header file which contains definitions for the PCAP hardware
* registers and declarations of bitstream download functions
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date        Changes
* ----- ---- -------- -------------------------------------------------------
* 1.00  ba   11/17/14 Initial release
*
* </pre>
*
* @note
*
******************************************************************************/

#ifndef XFSBL_BS_H
#define XFSBL_BS_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

#include "xfsbl_main.h"
#include "xfsbl_csu_dma.h"
#include "xfsbl_hw.h"
#include "xcsudma.h"
/************************** Constant Definitions *****************************/

#define PL_DONE_POLL_COUNT  10000U
#define PL_RESET_PERIOD_IN_US  1U

/* Dummy address to indicate that destination is PCAP */
#define XFSBL_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XFsbl_BitstreamLoad(XFsblPs * FsblInstancePtr,
				u32 PartitionNum, PTRSIZE LoadAddress);
u32 XFsbl_ChunkedBSTxfer(XFsblPs *FsblInstancePtr, u32 PartitionNum);
u32 XFsbl_PcapInit(void);
u32 XFsbl_PLWaitForDone(void);
u32 XFsbl_WriteToPcap(u32 WrSize, u8 *WrAddr);

/************************** Variable Definitions *****************************/

extern XCsuDma CsuDma;  /* CSU DMA instance */


#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_BS_H */
