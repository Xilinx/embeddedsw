/******************************************************************************
* Copyright (C) 2017 - 2019 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/
#ifndef XPFW_RESTART_H_
#define XPFW_RESTART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "pm_master.h"

#define FSBL_STORE_ADDR					(XPAR_MICROBLAZE_DDR_RESERVE_SA + 0x80000U)
#define FSBL_IMAGE_HASH_ADDR			(XPAR_MICROBLAZE_DDR_RESERVE_SA + 0xC0000U)
#define FSBL_IMAGE_HASH_VERIFY_ADDR		(XPAR_MICROBLAZE_DDR_RESERVE_SA + 0xD0000U)
#define FSBL_LOAD_ADDR					0xFFFC0000U
#define FSBL_IMAGE_SIZE					(170U*1024U)

#define FSBL_STATE_PROC_SHIFT			(0x1U)

#define FSBL_RUNNING_ON_A53				(0x1U << FSBL_STATE_PROC_SHIFT)
#define FSBL_RUNNING_ON_R5_0			(0x2U << FSBL_STATE_PROC_SHIFT)
#define FSBL_RUNNING_ON_R5_L			(0x3U << FSBL_STATE_PROC_SHIFT)

#define FSBL_STATE_PROC_INFO_MASK		(0x3U << FSBL_STATE_PROC_SHIFT)

s32 XPfw_RecoveryInit(void);
void XPfw_RecoveryHandler(u8 ErrorId);
void XPfw_RecoveryAck(PmMaster *Master);

s32 XPfw_StoreFsblToDDR(void);
s32 XPfw_RestoreFsblToOCM(void);

#ifdef __cplusplus
}
#endif

#endif /* XPFW_RESTART_H_ */
