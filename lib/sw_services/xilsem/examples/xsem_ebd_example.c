/******************************************************************************
* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.
* (c) Copyright 2022-2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_ebd_example.c
 *
 * This file has the example usage of XSem_EbdLookUp API used to find whether a
 * particular bit is essential or not.
 * This example is applicable for Versal mono devices
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hv    05/25/2022  Initial Creation
 * 0.2   anv   02/24/2025  Updated XSem_EbdLookUp invocation as per the change
 *                         in API arguments.
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "xsem_ebd_search.h"
extern volatile int XSem_EbdBuffer[];
int main()
{
	u32 Status = 0U;

    xil_printf("=================================================\n\r");
    xil_printf("              EBD search start\n\r");
    xil_printf("=================================================\n\r");

    Status = XSem_EbdLookUp(0,0,0,0,98,&XSem_EbdBuffer[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(3,1,10,0,63,&XSem_EbdBuffer[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(4,2,5,0,87,&XSem_EbdBuffer[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(5,3,1,24,127,&XSem_EbdBuffer[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	return 0;
}
