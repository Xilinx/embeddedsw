/******************************************************************************
*
* Copyright (C) 2015 - 17 Xilinx, Inc.  All rights reserved.
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
* 2.0   bv   12/05/16 Made compliance to MISRAC 2012 guidelines
*                     Modified bitstream chunk size to 56KB
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

#define PL_DONE_POLL_COUNT  (u32)(10000U)
#define PL_RESET_PERIOD_IN_US  1U
#define XFSBL_PL_PWRUP_WAIT_MICROSEC    0U

/* Dummy address to indicate that destination is PCAP */
#define XFSBL_DESTINATION_PCAP_ADDR    (0XFFFFFFFFU)

/*
 * Buffer sizes required for bitstream
 * if block size is 8MB and taking chunk size as 56KB(READ_BUFFER_SIZE)
 * we may require a buffer to store hashs of the chunks is:
 * HASH_BUFFER_SIZE = (8MB/56KB)* (Sha3/2 hash length)
 */
#define READ_BUFFER_SIZE			(56*1024)
					/**< Buffer Size to store chunk
					of data */
#define HASH_BUFFER_SIZE			(7*1024)
					 /**< Buffer to store chunk's
						hashs of each block. */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
u32 XFsbl_BitstreamLoad(XFsblPs * FsblInstancePtr,
				u32 PartitionNum, PTRSIZE LoadAddress);
u32 XFsbl_ChunkedBSTxfer(XFsblPs *FsblInstancePtr, u32 PartitionNum);
u32 XFsbl_PcapInit(void);
u32 XFsbl_PLWaitForDone(void);
u32 XFsbl_WriteToPcap(u32 WrSize, u8 *WrAddr);
u32 XFsbl_PLCheckForDone(void);

/************************** Variable Definitions *****************************/

extern XCsuDma CsuDma;  /* CSU DMA instance */


#ifdef __cplusplus
}
#endif

#endif  /* XFSBL_BS_H */
