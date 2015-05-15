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
******************************************************************************/
#ifndef PM_IPI_BUFFER_H_
#define PM_IPI_BUFFER_H_

#define IPI_BUFFER_BASEADDR	0xFF990000U

#define IPI_BUFFER_RPU_0_BASE	(IPI_BUFFER_BASEADDR + 0x0U)
#define IPI_BUFFER_RPU_1_BASE	(IPI_BUFFER_BASEADDR + 0x200U)
#define IPI_BUFFER_APU_BASE	(IPI_BUFFER_BASEADDR + 0x400U)
#define IPI_BUFFER_PL_0_BASE	(IPI_BUFFER_BASEADDR + 0x600U)
#define IPI_BUFFER_PL_1_BASE	(IPI_BUFFER_BASEADDR + 0x800U)
#define IPI_BUFFER_PL_2_BASE	(IPI_BUFFER_BASEADDR + 0xA00U)
#define IPI_BUFFER_PL_3_BASE	(IPI_BUFFER_BASEADDR + 0xC00U)
#define IPI_BUFFER_PMU_BASE	(IPI_BUFFER_BASEADDR + 0xE00U)

#define IPI_BUFFER_TARGET_RPU_0_OFFSET	0x0U
#define IPI_BUFFER_TARGET_RPU_1_OFFSET	0x40U
#define IPI_BUFFER_TARGET_APU_OFFSET	0x80U
#define IPI_BUFFER_TARGET_PL_0_OFFSET	0xC0U
#define IPI_BUFFER_TARGET_PL_1_OFFSET	0x100U
#define IPI_BUFFER_TARGET_PL_2_OFFSET	0x140U
#define IPI_BUFFER_TARGET_PL_3_OFFSET	0x180U
#define IPI_BUFFER_TARGET_PMU_OFFSET	0x1C0U

#define IPI_BUFFER_REQ_OFFSET	0x0U
#define IPI_BUFFER_RESP_OFFSET	0x20U

#endif
