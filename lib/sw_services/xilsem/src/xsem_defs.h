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
* @file xsem_defs.h
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
#ifndef XSEM_DEFS_H		/* prevent circular inclusions */
#define XSEM_DEFS_H		/* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#include "xil_types.h"
#include "xil_io.h"
#include "xil_cache.h"
#include "xil_assert.h"
#include "xstatus.h"

/***************************** Global variables ******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define XSEM_FAIL(infoptr, errcode, word0, word1, word2, word3, word4) \
  if (infoptr != NULL) { \
    infoptr->ErrCode  = (u32)errcode; \
    infoptr->ErrWord0 = (u32)word0; \
    infoptr->ErrWord1 = (u32)word1; \
    infoptr->ErrWord2 = (u32)word2; \
    infoptr->ErrWord3 = (u32)word3; \
    infoptr->ErrWord4 = (u32)word4; \
  }

#define XSEM_CLEAR(infoptr)	\
  if (infoptr != NULL) { \
    infoptr->ErrCode  = (u32)0; \
    infoptr->ErrWord0 = (u32)0; \
    infoptr->ErrWord1 = (u32)0; \
    infoptr->ErrWord2 = (u32)0; \
    infoptr->ErrWord3 = (u32)0; \
    infoptr->ErrWord4 = (u32)0; \
  }

/************************** Constant Definitions *****************************/
/** @name Macro to limit value between min and max
 * @{
 */
#define XSem_limit(x,max,min)	(((x)<(min))?(min):(((x)>(max))?(max):(x)))

/**************************** Type Definitions *******************************/
/**
* This typedef contains....
* ErrCode	: Error code indicating which SEM feature failed
* ErrWord0	: Word-0 of the error information
* ErrWord1	: Word-1 of the error information
* ErrWord2	: Word-2 of the error information
* ErrWord3	: Word-3 of the error information
* ErrWord4	: Word-4 of the error information
*/
typedef struct {
	u32 ErrCode;	/**< Error code indicating which SEM feature failed */
	u32 ErrWord0;	/**< Word-0 of the error information */
	u32 ErrWord1;	/**< Word-1 of the error information */
	u32 ErrWord2;	/**< Word-2 of the error information */
	u32 ErrWord3;	/**< Word-3 of the error information */
	u32 ErrWord4;	/**< Word-4 of the error information */
} XSem_ErrReport;

/************************** Function Prototypes ******************************/

#ifdef __cplusplus
}
#endif

#endif /* XSEM_DEFS_H */
