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
* This is an example application which demonstrate how different applications
* can be developed with APUFW library for secure EL1.
* xsecure_arithmetic.c contains function Xil_Arithmetic which performs
* different arithmetic operation returns the result. It registers the
* Xil_Arithmetic function as a xilsp handler for ARITH_SMC_FID which is being
* called by APUFW when non-secure application request the service with
* ARITH_SMC_FID.
*
******************************************************************************/
/***************************** Include Files *********************************/
#include "xilsp.h"
#include "xil_smc.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
XSmc_OutVar Xil_Arithmetic(u64 arg0,u64 arg1, u64 arg2, u64 arg3, u64 arg4,
					u64 arg5, u64 arg6, u64 arg7);
/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This function performs arithmetic operations using Arg1 and Arg2 provided by
* APUFW (which are given by non-secure EL1).
*
* @param	None.
*
* @return	Result which contains results of different arithemetic operation
*
* @note		None.
*
****************************************************************************/
XSmc_OutVar Xil_Arithmetic(u64 Arg0,u64 Arg1, u64 Arg2, u64 Arg3, u64 Arg4,
					u64 Arg5, u64 Arg6, u64 Arg7){
	XSmc_OutVar Result;
	Result.Arg0 = Arg1 * Arg2;
	Result.Arg1 = Arg1 + Arg2;
	Result.Arg2 = Arg1 - Arg2;
	Result.Arg3 = Arg1 / Arg2;

	return Result;
}

/*
 * Declares Xil_Arithmetic as a XilSP_Handler for ARITH_SMC_FID which is used
 * by APUFW main application to register handler for ARITH_SMC_FID
 */
DECLARE_XILSP_HANDLER(
		ARITH_SMC_FID,
		Xil_Arithmetic);
