/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
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
/*
* This is a APUFW main application which registers XilSP_Handlers for specified
* SMC IDs and return the xilsp_vector_table address to XILSPD.
*
******************************************************************************/
/***************************** Include Files *********************************/

#include <stdio.h>
#include "xilsp.h"
#include "xil_printf.h"
/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
/* Define in Secure application linker script */
extern XilSP_HandlerEntry __xilsp_start;
extern XilSP_HandlerEntry __xilsp_end;

XilSP_HandlerEntry *EL1Handler;
/*****************************************************************************/
/*
* The main function looks for XilSP_Handlers declared by various applications
* for different SMC IDs and registers these handlers to handler table with
* specified SMC IDs. Finally it calls XilSP_InitDone to indicate ATF that
* XilSP initialization is over and returns the xilsp_vector_table address
*
* @param	None.
*
* @return	0
*
* @note		None.
*
****************************************************************************/

int main()
{
	u32 Index;
	u64 XilSPHandlerNum;

	/* Find the number of handlers registered for different SMC IDs */
	XilSPHandlerNum = ((u64 )&__xilsp_end - (u64 )&__xilsp_start)/sizeof(XilSP_HandlerEntry);
	EL1Handler = (XilSP_HandlerEntry *)&__xilsp_start;

	/* Registers XilSP_Handler for specifies SMC IDs */
	for(Index = 0; Index < XilSPHandlerNum; Index++){
		XilSP_HandlerConnect(EL1Handler->SmcFID, EL1Handler->Handler);
		EL1Handler++;
	}

	xil_printf("Number of XilSP Handler Registered are : %x \n",XilSPHandlerNum);

	/* XilSP initialization is over and call smc to return to ATF*/
	XilSP_InitDone();

	/* Control will not reach here */
	return 0;
}
