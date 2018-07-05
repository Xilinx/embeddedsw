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

#ifndef _EFUSE_H_
#define _EFUSE_H_

#ifdef __cplusplus
extern "C" {
#endif

/* EFUSE Base Address */
#define EFUSE_BASEADDR      0XFFCC0000U

 /* Register: EFUSE_IPDISABLE */
#define EFUSE_IPDISABLE    ( ( EFUSE_BASEADDR ) + 0X00001018U )

#define EFUSE_IPDISABLE_GPU_DIS_SHIFT   5U
#define EFUSE_IPDISABLE_GPU_DIS_WIDTH   1U
#define EFUSE_IPDISABLE_GPU_DIS_MASK    0X00000020U

#define EFUSE_IPDISABLE_APU3_DIS_SHIFT   3U
#define EFUSE_IPDISABLE_APU3_DIS_WIDTH   1U
#define EFUSE_IPDISABLE_APU3_DIS_MASK    0X00000008U

#define EFUSE_IPDISABLE_APU2_DIS_SHIFT   2U
#define EFUSE_IPDISABLE_APU2_DIS_WIDTH   1U
#define EFUSE_IPDISABLE_APU2_DIS_MASK    0X00000004U

#define EFUSE_IPDISABLE_APU1_DIS_SHIFT   1U
#define EFUSE_IPDISABLE_APU1_DIS_WIDTH   1U
#define EFUSE_IPDISABLE_APU1_DIS_MASK    0X00000002U

#define EFUSE_IPDISABLE_APU0_DIS_SHIFT   0U
#define EFUSE_IPDISABLE_APU0_DIS_WIDTH   1U
#define EFUSE_IPDISABLE_APU0_DIS_MASK    0X00000001U

#define EFUSE_IPDISABLE_VERSION		0x1FFU

#ifdef __cplusplus
}
#endif


#endif /* _EFUSE_H_ */
