/*******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/
#include "xdpdma.h"
#include "xparameters.h"

XDpDma_Config XDpDma_ConfigTable[]
{
	{
		XPAR_XDPDMA_0_COMPATIBLE,
		XPAR_XDPDMA_0_BASEADDR,
		XPAR_XDPDMA_0_INTERRUPTS,
		XPAR_XDPDMA_0_INTERRUPT_PARENT
	}
};
