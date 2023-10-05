/******************************************************************************
* Copyright (C) 2014 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsrio_g.c
* @addtogroup srio Overview
* @{
*
* This file contains a configuration table that specifies the configuration of
* SRIO devices in the system. Each SRIO device in the system should have an entry
* in the table.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   adk  16/04/14 Initial release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsrio.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

XSrio_Config XSrio_ConfigTable[] = {
	{
		XPAR_SRIO_0_DEVICE_ID,
		XPAR_SRIO_0_BASEADDR,
		XPAR_SRIO_0_PE_MEMORY,
		XPAR_SRIO_0_PE_PROC,
		XPAR_SRIO_0_PE_BRIDGE
	}
};
/** @} */
