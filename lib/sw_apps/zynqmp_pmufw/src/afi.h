/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
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
******************************************************************************/

#ifndef _AFI_H_
#define _AFI_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 * FPD SLCR Base Address
 */
#define FPD_SLCR_BASEADDR                       ((u32)0xFD610000U)

/*
 * Register: AFI_FS
 */
#define FPD_SLCR_AFI_FS_REG                     ( ( FPD_SLCR_BASEADDR ) + ((u32)0X00005000U) )

/*
 * AFI FM0 Base Address
 */
#define AFI_FM0_BASEADDR                        ((u32)0xFD360000U)

/*
 * AFI FM1 Base Address
 */
#define AFI_FM1_BASEADDR                        ((u32)0xFD370000U)

/*
 * AFI FM2 Base Address
 */
#define AFI_FM2_BASEADDR                        ((u32)0xFD380000U)

/*
 * AFI FM3 Base Address
 */
#define AFI_FM3_BASEADDR                        ((u32)0xFD390000U)

/*
 * AFI FM4 Base Address
 */
#define AFI_FM4_BASEADDR                        ((u32)0xFD3A0000U)

/*
 * AFI FM5 Base Address
 */
#define AFI_FM5_BASEADDR                        ((u32)0xFD3B0000U)

/*
 * AFI FM6 Base Address
 */
#define AFI_FM6_BASEADDR                        ((u32)0xFF9B0000U)

#endif /* _AFI_H_ */
