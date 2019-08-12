/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
*******************************************************************************/
/******************************************************************************/
/**
* @file xdmapcie_caps.c
*
* Implements all of supportive functions to expose PCIe capabilities.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------
* 1.0	tk	01/30/2019	First release
* </pre>
*
*******************************************************************************/

/******************************** Include Files *******************************/
#include "xdmapcie.h"
#include "xdmapcie_common.h"

/**************************** Constant Definitions ****************************/

/****************************** Type Definitions ******************************/

/******************** Macros (Inline Functions) Definitions *******************/

/**************************** Variable Definitions ****************************/

/***************************** Function Prototypes ****************************/


/******************************************************************************/
/**
* This function returns offset to the first of the Function's Linked
* list of the capability registers.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
*
* @return  offset to the first of the Function's capability register
* if available.	0x0 if not available
*
*******************************************************************************/
static u32 XDmaPcie_GetBaseCapability(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
	u8 Function)
{
	u32 CapBase = 0x0;

	XDmaPcie_ReadRemoteConfigSpace(InstancePtr, Bus, Device, Function,
			XDMAPCIE_CFG_P_CAP_PTR_T1_REG, &CapBase);
	return (CapBase & XDMAPCIE_CAP_PTR_LOC);
}

/******************************************************************************/
/**
* This function returns whether capability Id is available or not for the
* particular Function.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
* @param   cap id to check capability pointer availability
*
* @return  u32 0 if capability is not available
* 1 if capability is available
*
*******************************************************************************/
u8 XDmaPcie_HasCapability(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{
	u8 CapStatus = CAP_NOT_PRESENT;
	u32 CapBase = XDmaPcie_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	while (CapBase) {
		XDmaPcie_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
					       Function, XDMAPCIE_DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (CapBase & XDMAPCIE_CFG_CAP_ID_LOC)){
			CapStatus =  CAP_PRESENT;
			goto End;
		}


		CapBase = (CapBase >> XDMAPCIE_CAP_SHIFT) & XDMAPCIE_CAP_PTR_LOC;
	}

End:
	return CapStatus;

}

/******************************************************************************/
/**
* This function returns offset to the matching capability ID from the
* Function's Linked list of the capability registers.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
* @param   cap id to get capability pointer offset
*
* @return  u64 capability pointer if available
* 0 if not available.
*
*******************************************************************************/
u64 XDmaPcie_GetCapability(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{
	u32 CapBase = XDmaPcie_GetBaseCapability(InstancePtr, Bus, Device,
			Function);
	u32 Adr = 0;
	u64 Location = CAP_NOT_PRESENT;
	u32 Offset;


	while (CapBase) {
		Adr = CapBase;
		XDmaPcie_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
				Function, XDMAPCIE_DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (CapBase & XDMAPCIE_CFG_CAP_ID_LOC)) {
			Offset = XDmaPcie_ComposeExternalConfigAddress(
					Bus, Device, Function, XDMAPCIE_DOUBLEWORD(Adr));
			Location = (InstancePtr->Config.Ecam) + (Offset);
			goto End;
		}
		CapBase = (CapBase >> XDMAPCIE_CAP_SHIFT) & XDMAPCIE_CAP_PTR_LOC;
	}
End:
	return Location;
}

/******************************************************************************/
/**
* This function prints all the available capabilities in the Function.
*
* @param   InstancePtr pointer to XDmaPcie Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
*
* @return  XST_SUCCESS on success
* XST_FAILURE on failure.
*
*******************************************************************************/
u8 XDmaPcie_PrintAllCapabilites(XDmaPcie *InstancePtr, u8 Bus, u8 Device,
		u8 Function)
{

	u32 CapBase = XDmaPcie_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	xil_printf("CAP-IDs:");
	while (CapBase) {
		XDmaPcie_ReadRemoteConfigSpace(InstancePtr, Bus, Device,
				Function, XDMAPCIE_DOUBLEWORD(CapBase), &CapBase);
		xil_printf("0x%X ", CapBase & XDMAPCIE_CFG_CAP_ID_LOC);
		CapBase = (CapBase >> XDMAPCIE_CAP_SHIFT) & XDMAPCIE_CAP_PTR_LOC;
	}
	xil_printf("\r\n");
	return XST_SUCCESS;
}
