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
/****************************************************************************/
/**
*
* @file xilsp.c
*
* This file contains Xilinx secure payload related APIs.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who      Date     Changes
* ----- -------- -------- -----------------------------------------------
* 1.0 	pkp  	 08/15/16 First release
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xilsp.h"
#include "xdebug.h"
#include "xil_smc.h"
/************************** Constant Definitions ****************************/

/* Number of functions supported by XilSP handler */
#define MAX_FID	0x10
/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

/************************** Variable Definitions *****************************/

XilSP_HandlerEntry XilSPHandlerTable[MAX_FID];
u32 HandlerCount;
u64 _xilsp_vectors_table;
extern u64 _xilsp_vectors_table;

/*****************************************************************************/
/*****************************************************************************
*
* This function handles the request from non-secure application. It calls the
* function handler associates with a particular SMC identifier.
*
* @param	Arg0 is the SMC identifier for a particular request initiated by
*			non-secure application.
* @param	Arg1 to Arg7 is the arguements passed to EL3 secure monitor
*
* @return	None
* @note		FunctionID and Arg1-Arg7 is as per SMC calling convention.
*			Result received from the function handler for a particular SMC
*			identifier is returned by calling SMC. If no function handler
*			is registered for particular SMC identifier, arguments passed by
*			non-secure application is returned as it is via SMC calling.
*
******************************************************************************/
void XilSP_SmcHandler(u64 Arg0, u64 Arg1, u64 Arg2, u64 Arg3, u64 Arg4,
					 u64 Arg5, u64 Arg6, u64 Arg7)
{
	XSmc_OutVar OutVar;
	u32 Index;

	/* Look for Register handler with SMC ID Arg0 */
	for (Index = 0; Index < HandlerCount; Index++) {
		if ( XilSPHandlerTable[Index].SmcFID == Arg0) {
			OutVar = XilSPHandlerTable[Index].Handler(Arg0, Arg1, Arg2, Arg3,
									Arg4, Arg5, Arg6, Arg7);
			/*
			 * Call SMC with result arugments and go back to secure EL3 (ATF)
			 */
			Xil_Smc(Arg0, OutVar.Arg0, OutVar.Arg1, OutVar.Arg2, OutVar.Arg3,
					0, 0, 0);
		}
	}

	/*
	 * If not Handler is found then only control will reach here which will
	 * return to ATF with same arguments passed by non-secure application
	 */
	xdbg_printf(XDBG_DEBUG_ERROR, "No handler is registered for SMC ID %x \n",
				Arg0);
	Xil_Smc(Arg0, Arg1, Arg2, Arg3, Arg4, Arg5, Arg6, Arg7);

}
/*****************************************************************************
*
* This function registers Handler for particular SMC identifier.
*
* @param	SmcID is the identifier for a particular task as per SMC calling
*			convention.
* @param	Handler is a function which will be called when Non-secure
*			application initiates smc call with SmcID
*
* @return	XST_SUCCESS or XST_FAILURE
*
******************************************************************************/
s32 XilSP_HandlerConnect(u32 SmcID, XilSP_Handler Handler)
{
	u32 Status;
	if (HandlerCount < MAX_FID) {
		if ((SmcID < SMC_FID_END) && (SmcID > SMC_FID_START)) {
			XilSPHandlerTable[HandlerCount].SmcFID = SmcID;
			XilSPHandlerTable[HandlerCount].Handler = Handler;
			HandlerCount++;
			Status = XST_SUCCESS;
		}
		else
			Status = XST_FAILURE;
	} else
		Status = XST_FAILURE;

	return Status;
}
/*****************************************************************************
*
* This function initiates SMC call with XILSP_INIT_DONE ID informing
* arm-trusted-firmware that XILSP initialization is over and passes the address
* of XilSP vector table
*
* @param	None.
*
* @return	None.
*
******************************************************************************/
void XilSP_InitDone(){
	u64 VectorAddr;
	VectorAddr = (u64) &_xilsp_vectors_table;
	Xil_Smc(XILSP_INIT_DONE, VectorAddr, 0, 0, 0, 0, 0, 0);
}