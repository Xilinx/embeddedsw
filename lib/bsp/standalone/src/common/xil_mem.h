/******************************************************************************/
/**
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
* @file xil_mem.h
*
* @addtogroup common_mem_operation_api Customized APIs for Memory Operations
*
* The xil_mem.h file contains prototype for functions related
* to memory operations. These APIs are applicable for all processors supported
* by Xilinx.
*
* @{
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 6.1   nsk      11/07/16 First release.
* 7.0   mus      01/07/19 Add cpp extern macro
*
* </pre>
*
*****************************************************************************/
#ifndef XIL_MEM_H		/* prevent circular inclusions */
#define XIL_MEM_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/************************** Function Prototypes *****************************/

void Xil_MemCpy(void* dst, const void* src, u32 cnt);

#ifdef __cplusplus
}
#endif

#endif /* XIL_MEM_H */
/**
* @} End of "addtogroup common_mem_operation_api".
*/