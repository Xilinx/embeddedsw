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
* @file xpciepsu_caps.c
*
* Implements all of supportive functions to expose PCIe capablities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 0.1	bs	08/21/2018	First release
* </pre>
*
*******************************************************************************/

/******************************** Include Files *******************************/
#include "xpciepsu.h"
#include "xpciepsu_common.h"

/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/
#define BASE_CAPPTR_DW					0xD

#define LAST_TWO_NIBBLES(x)				(x & 0xFF)
#define SHIFT_RIGHT_TWO_NIBBLES(x)		(x >> 8)
#define NEXT_CAPPTR(x)					((x >> 8) & 0xFF)
#define DOUBLEWORD(x)					(x / 4)
#define CAP_PRESENT						(1)
#define CAP_NOT_PRESENT					(0)
/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/


/******************************************************************************/
/**
* This function returns capability pointer at doubleword 0xD of bdf.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
*
* @return  u32 address available in capability pointer location
*
*******************************************************************************/
static u32 XPciePsu_GetBaseCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
	u8 Function)
{
	u32 CapBase = 0x0;

	XPciePsu_ReadRemoteConfigSpace(InstancePtr, Bus, Device, Function,
			BASE_CAPPTR_DW, &CapBase);
	return (LAST_TWO_NIBBLES(CapBase));
}

/******************************************************************************/
/**
* This function returns whether capability is available with cap id in bdf.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
* @param   cap id to check capability pointer availability
*
* @return  u32 0 if capability is not available
* 1 if capability is available
*
*******************************************************************************/
u32 XPciePsu_HasCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{

	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	while (CapBase) {
		XPciePsu_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
					       Function, DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (LAST_TWO_NIBBLES(CapBase)))
			return CAP_PRESENT;

		CapBase = NEXT_CAPPTR(CapBase);
	}

	return CAP_NOT_PRESENT;
}

/******************************************************************************/
/**
* This function returns address of the capability pointer with the cap id.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
* @param   cap id to get capability pointer
*
* @return  u32 capability pointer if available
* 0 if not available.
*
*******************************************************************************/
u32 XPciePsu_GetCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{
	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);
	u32 adr = 0;
	u32 Location;

	while (CapBase) {
		adr = CapBase;
		XPciePsu_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
				Function, DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (LAST_TWO_NIBBLES(CapBase))) {
			Location = XPciePsu_ComposeExternalConfigAddress(
					Bus, Device, Function, DOUBLEWORD(adr));
			return Location;
		}
		CapBase = NEXT_CAPPTR(CapBase);
	}
	return CAP_NOT_PRESENT;
}

/******************************************************************************/
/**
* This function prints all the available capabilities in the bdf.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus
* @param   Device
* @param   Function
*
* @return  none
*
*******************************************************************************/
void XPciePsu_ListAllCapabilites(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function)
{

	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	XPciePsu_Dbg("CAP-IDs:");
	while (CapBase) {
		XPciePsu_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
				Function, DOUBLEWORD(CapBase), &CapBase);
		XPciePsu_Dbg("0x%X ", LAST_TWO_NIBBLES(CapBase));
		CapBase = NEXT_CAPPTR(CapBase);
	}
	XPciePsu_Dbg("\r\n");
}
