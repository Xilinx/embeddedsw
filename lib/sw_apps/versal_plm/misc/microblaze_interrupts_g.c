/*******************************************************************
* Copyright (c) 2010-2020 Xilinx, Inc. All Rights Reserved.*
* SPDX-License-Identifier: MIT
*******************************************************************/

#include "microblaze_interrupts_i.h"
#include "xparameters.h"


extern void XNullHandler (void *);

/*
* The interrupt handler table for microblaze processor
*/

MB_InterruptVectorTableEntry MB_InterruptVectorTable[] =
{
{	XNullHandler,
	(void*) XNULL}
};

