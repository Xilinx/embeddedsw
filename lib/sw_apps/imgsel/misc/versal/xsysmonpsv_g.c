/*******************************************************************
* Copyright (C) 2022 Xilinx, Inc. All Rights Reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
*
* Description: Driver configuration
*
*******************************************************************/

#include "xparameters.h"
#include "xsysmonpsv.h"

/*
* The configuration table for devices
*/

XSysMonPsv_Config XSysMonPsv_ConfigTable[XPAR_XSYSMONPSV_NUM_INSTANCES] =
{
	{
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_SYSMON_0_S_AXI_BASEADDR,
		XPAR_VERSAL_CIPS_0_PSPMC_0_PSV_PMC_SYSMON_0_NUMSUPPLIES,
			{
			}

	}

};
