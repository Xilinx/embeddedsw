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
/**
* @file xsem_cfr_common.h
*  This file contains structures, global variables and Macro definitions
*  that are common for all the SEM components
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	  Who      Date		     Changes
* ----  ----     --------    --------------------------------------------------
* 0.1	  mw	 06/26/2018  Initial creation
*
* </pre>
*
******************************************************************************/
#ifndef SEM_CFR_COMMON_H                /* prevent circular inclusions */
#define SEM_CFR_COMMON_H              /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

#include "xil_types.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xcframe.h"
#include "xcfupmc.h"

#include "ppu1_iomodule.h"
#include "pmc_global.h"
#include "psm_global_reg.h"
#include "cframe0_reg.h"
#include "cframe_bcast_reg.h"
#include "cfu_apb.h"

#include "xsem_glbl_regs.h"
#include "xsem_common.h"

#define XSEM_NUM_OF_FRAMES               1
#define XSEM_ARRAY_SIZE                  XSEM_NUM_OF_FRAMES*100

/************************** Function Prototypes ******************************/
u32 XSem_CfrStopScan(void);
u32 XSem_CfrStartScan(void);
u32 XSem_CfrWriteFrame(u32 CframeFar, u32 CfrWrDat[], u32 RowLoc);
u32 XSem_CfrReadFrame(u32 CframeFar, u32 CfrRdDat[], u32 RowLoc);
u32 XSem_CfrErrNjct(u32 CfrErrAddr, u32 WordLocation, u32 BitLocation,
    u32 CfrErrDat[], u32 RowLocation);
u32 XSem_CfrErrDet(void);

#ifdef __cplusplus
}
#endif

#endif  /* SEM_CFR_COMMON_H */
