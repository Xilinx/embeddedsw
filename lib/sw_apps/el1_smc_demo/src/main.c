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
/**
*
* This application demonstrate an example to call SMC which performs arithmetic
* operation at EL1 secure. It needs to be used along with apufw_demo
* application and atf to produce the complete flow of SMC. This Non-secure EL1
* application calls SMC with arithmetic arguments and ARITH_SMC_FID, exception
* level of processor changes to EL3 and control reaches to ATF. ATF checks for
* SMC ID and in this case calls xilspd which further calls apufw. Handler
* registered at apufw performs the operation and returns the value to ATF which
* further returns to this non-secure EL1 application.
*
******************************************************************************/

/***************************** Include Files *********************************/

#include <stdio.h>
#include "xstatus.h"
#include "xil_printf.h"
#include "xil_smc.h"

/************************** Constant Definitions *****************************/
#define SMC_UNK     0xFFFFFFFF
/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

/*****************************************************************************/
/**
*
* This is the main function for the A53 EL1 Non-secure SMC call demo.
*
* @param	None.
*
* @return	None
*
* @note		None.
*
****************************************************************************/
int main(void)
{
	u64 FID;
	XSmc_OutVar Result;
	u64 x1;
	u64 x2;

	xil_printf("Non-Secure EL1 Demo Application \n");
	FID = ARITH_SMC_FID;
	/* Specify arugemnts for SMC call to perform arithmetic operation */
	x1 = 0x02020202;
	x2 = 0x01010101;

	xil_printf("Calling SMC to perform arithmetic operation \n");
	Result = Xil_Smc(FID, x1, x2, 0, 0, 0, 0, 0);

	if (Result.Arg0 == SMC_UNK) {
		xil_printf("SMC operation is failed \n");
	} else {
		xil_printf("SMC operation is successful with Result : \n");

		xil_printf("%x * %x = %x \n",x1,x2,Result.Arg0);
		xil_printf("%x + %x = %x \n",x1,x2,Result.Arg1);
		xil_printf("%x - %x = %x \n",x1,x2,Result.Arg2);
		xil_printf("%x / %x = %x \n",x1,x2,Result.Arg3);
	}
	return 0;
}
