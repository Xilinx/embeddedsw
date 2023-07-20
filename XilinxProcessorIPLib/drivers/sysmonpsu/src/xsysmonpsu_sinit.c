/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xsysmonpsu_sinit.c
* @addtogroup Overview
*
* This file contains the implementation of the XSysMonPsu driver's static
* initialization functionality.
*
* @note		None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date	    Changes
* ----- -----  -------- -----------------------------------------------
* 1.0   kvn    12/15/15 First release.
* 2.9   cog    07/20/23 Added support for SDT flow.
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsysmonpsu.h"
#ifndef SDT
#include "xparameters.h"
#endif

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/

#ifndef SDT
extern XSysMonPsu_Config XSysMonPsu_ConfigTable[XPAR_XSYSMONPSU_NUM_INSTANCES];
#else
extern XSysMonPsu_Config XSysMonPsu_ConfigTable[];
#endif

/*****************************************************************************/
/**
*
* This function looks for the device configuration based on the unique device
* ID. The table XSysmonPsu_ConfigTable[] contains the configuration information
* for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
* @note		None.
*
******************************************************************************/
#ifndef SDT
XSysMonPsu_Config *XSysMonPsu_LookupConfig(u16 DeviceId)
{
	XSysMonPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0U; Index < (u32)XPAR_XSYSMONPSU_NUM_INSTANCES; Index++) {
		if (XSysMonPsu_ConfigTable[Index].DeviceId == DeviceId) {
			CfgPtr = &XSysMonPsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
#else
XSysMonPsu_Config *XSysMonPsu_LookupConfig(u32 BaseAddress)
{
	XSysMonPsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = (u32)0x0; XSysMonPsu_ConfigTable[Index].Name != NULL; Index++) {
		if ((XSysMonPsu_ConfigTable[Index].BaseAddress == BaseAddress) ||
		    !BaseAddress) {
			CfgPtr = &XSysMonPsu_ConfigTable[Index];
			break;
		}
	}


	return CfgPtr;
}
#endif
