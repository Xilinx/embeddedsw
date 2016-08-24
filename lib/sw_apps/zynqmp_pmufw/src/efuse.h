/*
 * Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
 */

#ifndef _EFUSE_H_
#define _EFUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* EFUSE Base Address */
#define EFUSE_BASEADDR      0XFFCC0000

 /* Register: EFUSE_IPDISABLE */
#define EFUSE_IPDISABLE    ( ( EFUSE_BASEADDR ) + 0X00001018 )

#define EFUSE_IPDISABLE_GPU_DIS_SHIFT   5
#define EFUSE_IPDISABLE_GPU_DIS_WIDTH   1
#define EFUSE_IPDISABLE_GPU_DIS_MASK    0X00000020

#define EFUSE_IPDISABLE_APU3_DIS_SHIFT   3
#define EFUSE_IPDISABLE_APU3_DIS_WIDTH   1
#define EFUSE_IPDISABLE_APU3_DIS_MASK    0X00000008

#define EFUSE_IPDISABLE_APU2_DIS_SHIFT   2
#define EFUSE_IPDISABLE_APU2_DIS_WIDTH   1
#define EFUSE_IPDISABLE_APU2_DIS_MASK    0X00000004

#define EFUSE_IPDISABLE_APU1_DIS_SHIFT   1
#define EFUSE_IPDISABLE_APU1_DIS_WIDTH   1
#define EFUSE_IPDISABLE_APU1_DIS_MASK    0X00000002

#define EFUSE_IPDISABLE_APU0_DIS_SHIFT   0
#define EFUSE_IPDISABLE_APU0_DIS_WIDTH   1
#define EFUSE_IPDISABLE_APU0_DIS_MASK    0X00000001

#ifdef __cplusplus
}
#endif


#endif /* _EFUSE_H_ */
