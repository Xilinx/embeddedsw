/******************************************************************************
*
* Copyright (C) 2018 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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

extern XPciePsu_Config XPciePsu_ConfigTable[];

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
	int Index;

	for (Index = 0; Index < 8; Index++) {
		if (XPciePsu_ConfigTable[Index].DeviceId == DeviceId) {
			if(XPciePsu_ConfigTable[Index].BrigReg == 0xff ||
					XPciePsu_ConfigTable[Index].Ecam == 0xff ||
					XPciePsu_ConfigTable[Index].NpMemBaseAddr == 0xff ||
					XPciePsu_ConfigTable[Index].NpMemMaxAddr == 0xff ||
					XPciePsu_ConfigTable[Index].PMemBaseAddr == 0xff ||
					XPciePsu_ConfigTable[Index].PMemMaxAddr == 0xff ||
					XPciePsu_ConfigTable[Index].PciReg == 0xff){
				CfgPtr = NULL;
				goto End;
			}
			CfgPtr = &XPciePsu_ConfigTable[Index];
			break;
		}
	}
End:
	return CfgPtr;
}
