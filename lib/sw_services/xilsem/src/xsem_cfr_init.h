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
* @file xsem_cfr_init.h
*  This file contains data declarations and definitions required for XilSem
* CFRAME Initialization
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	  Who      Date		     Changes
* ----  ----     --------    --------------------------------------------------
* 0.1	  mw	 06/26/2018  Initial creation
* 0.2     pc     02/13/2019  Remove unneeded error codes. Update include paths
*
* </pre>
*
******************************************************************************/

#ifndef XSEM_CFRINIT_H		/* prevent circular inclusions */
#define XSEM_CFRINIT_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xsem_glbl_regs.h"
#include "xsem_common.h"
#include "xsem_cfr_common.h"
#include "xsem_defs.h"

/***************************** Debug variables ******************************/

/***************************** Global variables ******************************/

/************************** Constant Definitions *****************************/
/* Error Codes
 *    3210
 *  0x0000; 0x<CFR ID><PARENT FUNCTION><ID Code High><ID Code Low>
 *  CFR_ID
 *    1 = CFR INIT
 *  Parent Function
 *    1 = SEM
 *    2 = CFRAME
 *    3 = CFU
 *    4 = DMA
 *    5 = SSS
 *    6 = SHA
 *  ID Code High
 *    TBD
 *  ID Code Low
 *    TBD
 */
#define XSEM_CFR_INIT_SEM_CRAM_SUCCESS      0x0000
#define XSEM_CFR_INIT_SEM_ENDOFCAL_WDT_EXP  0x1100

/***************************** Function prototypes ***************************/

u32 XSem_CfrInit(void);

#ifdef __cplusplus
}
#endif

#endif /* XSEM_CFRINIT_H */
