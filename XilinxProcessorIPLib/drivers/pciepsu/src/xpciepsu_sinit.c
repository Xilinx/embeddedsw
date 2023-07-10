/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_sinit.c
*
* This file contains PSU PCIe driver's static initialization.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/
/******************************** Include Files *******************************/
#include "xpciepsu.h"

/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/

/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/

/******************************************************************************/
/**
* This function looks for the configuration of PCIe from the configTable based
* on the unique device ID. The table XPciePsu_ConfigTable[] contains the
* configuration information for each device in the system.
*
* @param	DeviceId is the unique device ID of the device being looked up.
*
* @return	A pointer to the configuration table entry corresponding to the
*		given device ID, or NULL if no match is found.
*
*******************************************************************************/
XPciePsu_Config *XPciePsu_LookupConfig(u16 DeviceId)
{
	XPciePsu_Config *CfgPtr = NULL;
	u32 Index;

	for (Index = 0; Index < XPciePsu_ConfigTableSize; Index++) {
		if (XPciePsu_ConfigTable[Index].DeviceId == DeviceId) {
			if((XPciePsu_ConfigTable[Index].BrigReg == 0xffU) ||
					(XPciePsu_ConfigTable[Index].Ecam == 0xffU) ||
					(XPciePsu_ConfigTable[Index].NpMemBaseAddr == 0xffU) ||
					(XPciePsu_ConfigTable[Index].NpMemMaxAddr == 0xffU) ||
#if defined(__aarch64__) || defined(__arch64__)
					(XPciePsu_ConfigTable[Index].PMemBaseAddr == 0xffU) ||
					(XPciePsu_ConfigTable[Index].PMemMaxAddr == 0xffU) ||
#endif
					(XPciePsu_ConfigTable[Index].PciReg == 0xffU)){
				CfgPtr = NULL;
				break;
			}
			CfgPtr = &XPciePsu_ConfigTable[Index];
			break;
		}
	}

	return CfgPtr;
}
