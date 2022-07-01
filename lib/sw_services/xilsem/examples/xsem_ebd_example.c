/******************************************************************************
* (c) Copyright 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_ebd_example.c
 *
 * This file has the example usage of XSem_EbdLookUp API used to find whether a
 * particular bit is essential or not.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   hv    05/25/2022  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "xsem_ebd_search.h"

int main()
{
	u32 Status = 0U;

    xil_printf("=================================================\n\r");
    xil_printf("              EBD search start\n\r");
    xil_printf("=================================================\n\r");

    Status = XSem_EbdLookUp(0,0,0,0,98);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(3,1,10,0,63);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(4,2,5,0,87);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(5,3,1,24,127);
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
