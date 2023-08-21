/*******************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xdppsu.h"

/*
* The configuration table for devices
*/

XDpPsu_Config XDpPsu_ConfigTable[] =
{
	{
		XPAR_XDPPSU_0_COMPATIBLE,
		XPAR_XDPPSU_0_BASEADDR,
		XPAR_XDPPSU_0_INTERRUPTS,
		XPAR_XDPPSU_0_INTERRUPT_PARENT
	}
};
