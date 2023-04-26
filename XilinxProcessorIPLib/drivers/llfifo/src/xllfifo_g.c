/******************************************************************************
* Copyright (C) 2013 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xllfifo_g.c
* @addtogroup llfifo Overview
* @{
*
* Provide a template for user to define their own hardware settings.
*
* If using XPS, this file will be automatically generated.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 3.00a adk 9/10/2013 initial release
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xllfifo.h"

/************************** Constant Definitions *****************************/

XLlFifo_Config XLlFifo_ConfigTable[] =
{
	{
		XPAR_AXI_FIFO_MM_S_1_DEVICE_ID,
		XPAR_AXI_FIFO_MM_S_1_BASEADDR,
		XPAR_AXI_FIFO_MM_S_1_AXI4_BASEADDR,
		XPAR_AXI_FIFO_MM_S_1_DATA_INTERFACE_TYPE
	}
};
/** @} */
