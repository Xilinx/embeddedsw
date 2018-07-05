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

#ifndef _IPI_H_
#define _IPI_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IPI Base Address
 */
#define IPI_BASEADDR      ((u32)0XFF300000U)

#define IPI_PMU_0_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00030010U) )
#define IPI_PMU_1_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00031010U) )
#define IPI_PMU_2_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00032010U) )
#define IPI_PMU_3_ISR    ( ( IPI_BASEADDR ) + ((u32)0X00033010U) )

#ifdef __cplusplus
}
#endif


#endif /* _IPI_H_ */
