/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Interrupt Handler Table for MicroBlaze Processor
*
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
