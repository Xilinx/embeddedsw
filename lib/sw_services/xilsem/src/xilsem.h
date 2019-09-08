/*****************************************************************************
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
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
*
******************************************************************************/
/**
* @file xil_sem.h
*  This file contains structures, global variables and Macro definitions
*  required for the common routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date        Changes
* ----  ----     --------    --------------------------------------------------
* 0.1   mw       06/26/2018  Initial creation
* 0.2   mw       08/01/2019  Added PLMI Scheduler support
*
* </pre>
******************************************************************************/
#ifndef XIL_SEM_H		/* prevent circular inclusions */
#define XIL_SEM_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif


int XSem_Init();

#ifdef __cplusplus
}
#endif

#endif		/* XIL_SEM_H */
