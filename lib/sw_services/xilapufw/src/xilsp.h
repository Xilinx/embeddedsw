/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc. All rights reserved.
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
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xilsp.h
*
* This header file contains Xilinx secure payload related APIs definitions.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 1.0 	pkp  	 08/15/16 First release
* </pre>
*
******************************************************************************/

#ifndef XILSP_H /* prevent circular inclusions */
#define XILSP_H /* by using protection macros */

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files ********************************/

#include "xil_types.h"
#include "xil_smc.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

typedef XSmc_OutVar (*XilSP_Handler) (u64 Arg0,u64 Arg1, u64 Arg2,
					u64 Arg3, u64 Arg4, u64 Arg5, u64 Arg6, u64 Arg7);

typedef struct {
	u64 SmcFID;
	XilSP_Handler Handler;
} XilSP_HandlerEntry;

/***************** Macros (Inline Functions) Definitions ********************/
#define DECLARE_XILSP_HANDLER( _smcfid, _handler) \
	static const XilSP_HandlerEntry __xilsp_handlers \
		__section(".xilsp_handlers") __used = { \
			.SmcFID = _smcfid, \
			.Handler = _handler}
/************************** Variable Definitions ****************************/

/************************** Function Prototypes *****************************/
void XilSP_SmcHandler(u64 Arg0, u64 Arg1, u64 Arg2, u64 Arg3, u64 Arg4, u64 Arg5,
					u64 Arg6, u64 Arg7);
s32 XilSP_HandlerConnect(u32 FunctionID, XilSP_Handler Handler);
void XilSP_InitDone();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* XILSP_H */
