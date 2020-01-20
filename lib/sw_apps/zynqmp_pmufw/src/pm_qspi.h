/*
 * Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
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

#ifndef PM_QSPI_H_
#define PM_QSPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "pm_common.h"

#ifdef XPAR_PSU_QSPI_0_DEVICE_ID
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define QSPIPSU_DEVICE_ID		XPAR_XQSPIPSU_0_DEVICE_ID
#define QSPIDMA_DST_CTRL		(XPAR_XQSPIPSU_0_BASEADDR + 0x80CU)
#endif

s32 PmQspiInit(void);
s32 PmQspiWrite(u8 *WriteBufrPtr, u32 ByteCount);
s32 PmQspiRead(u32 ByteCount, u8 *ReadBfrPtr);
s32 PmQspiHWInit(void);

#ifdef __cplusplus
}
#endif

#endif /* PM_QSPI_H_ */
