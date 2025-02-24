/******************************************************************************
* (c) Copyright 2025 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/**
 * @file xsem_ebd_ssit_example.c
 *
 * This file has the example usage of XSem_EbdLookUp API used to find whether a
 * particular bit is essential or not.
 * This example is applicable for Versal SSIT devices and demonstrates the
 * usage of look-up API for two SLRs. If the device has more no.of SLRs,
 * You need to invoke the API by providing the corresponding XSem_EbdBufferX[]
 * as an argument, where 'X' refers to SLR ID.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----  ----  ----------  ---------------------------------------------------
 * 0.1   anv   02/24/2025  Initial Creation
 * </pre>
 *
 *****************************************************************************/
#include "xil_printf.h"
#include "xsem_ebd_search.h"

/* Essential data bit buffers created in xsem_ebdgoldendata*.c */
extern volatile int XSem_EbdBuffer0[];
extern volatile int XSem_EbdBuffer1[];

int main()
{
	u32 Status = 0U;

    xil_printf("=================================================\n\r");
    xil_printf("              EBD search start on SLR_0\n\r");
    xil_printf("=================================================\n\r");

    Status = XSem_EbdLookUp(0,0,0,0,98, &XSem_EbdBuffer0[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(4,2,5,0,87, &XSem_EbdBuffer0[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(5,3,1,24,127, &XSem_EbdBuffer0[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(0,0,0,1,42, &XSem_EbdBuffer0[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(5,1,0x1,24,127, &XSem_EbdBuffer0[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

    xil_printf("=================================================\n\r");
    xil_printf("              EBD search END on SLR_0\n\r");
    xil_printf("=================================================\n\r");

    xil_printf("=================================================\n\r");
    xil_printf("              EBD search start on SLR_1\n\r");
    xil_printf("=================================================\n\r");

    Status = XSem_EbdLookUp(0,0,1,0,63, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(0,0,1,0,68, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}

	xil_printf("=================================================\n\r");
    Status = XSem_EbdLookUp(0,0,1,0,69, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(0,0,0,1,42, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(3,0,0,0,0, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(3,0,0xA,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(3,1,0xB,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(3,2,0xB,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(3,3,0xB,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(4,0,0x4,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
	xil_printf("=================================================\n\r");
	Status = XSem_EbdLookUp(5,3,0x1,24,127, &XSem_EbdBuffer1[0]);
	if (Status == 1U)
	{
		xil_printf("Bit is essential\n\r");
	} else  if (Status == 0U){
		xil_printf("Bit is not essential\n\r");
	} else {
		xil_printf("Invalid Input\n");
	}
    xil_printf("=================================================\n\r");
    xil_printf("              EBD search END on SLR_1\n\r");
    xil_printf("=================================================\n\r");
	return 0;
}
