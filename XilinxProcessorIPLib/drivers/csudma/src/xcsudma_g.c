/*******************************************************************
* Copyright (C) 2010 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

#include "xparameters.h"
#include "xcsudma.h"

/*
* The configuration table for devices
*/

XCsuDma_Config XCsuDma_ConfigTable[] =
{
	{
		XPAR_PSU_CSUDMA_DEVICE_ID,
		XPAR_PSU_CSUDMA_BASEADDR,
		XPAR_PSU_CSUDMA_DMATYPE
	}
};