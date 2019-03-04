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
* @file xsem_glbl_regs.h
*  This file contains structures, global variables and Macro definitions
*  required for the common routines
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	  Who      Date		     Changes
* ----  ----     --------    --------------------------------------------------
* 0.1	  mw	 06/26/2018  Initial creation
* </pre>
*
******************************************************************************/
#ifndef XSEM_GLBL_REGS_H		/* prevent circular inclusions */
#define XSEM_GLBL_REGS_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/

/***************************** Global variables ******************************/
/* TODO: temporary. not in ppu1_iomodule.h */
#define PPU1_IOMODULE_IRQ_ACK_PMC_GIC_IRQ_MASK	     ((u32)0x00010000U)

/* TODO: remove once xregdb is updated */
#define PMC_GLOBAL_SEM_CRAM_ATTRIB_EN_INITIALIZATION_SHIFT   5
#define PMC_GLOBAL_SEM_CRAM_ATTRIB_EN_INITIALIZATION_WIDTH   1
#define PMC_GLOBAL_SEM_CRAM_ATTRIB_EN_INITIALIZATION_MASK    0X00000020

#define PMC_GLOBAL_SEM_STATUS_CRAM_INIT_DONE_SHIFT   11
#define PMC_GLOBAL_SEM_STATUS_CRAM_INIT_DONE_WIDTH   1
#define PMC_GLOBAL_SEM_STATUS_CRAM_INIT_DONE_MASK    0X00080000

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/************************** Function Prototypes ******************************/


#ifdef __cplusplus
}
#endif

#endif	/* XSEM_GLBL_REGS_H */
