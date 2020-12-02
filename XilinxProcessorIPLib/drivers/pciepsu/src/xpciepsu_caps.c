/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
*******************************************************************************/

/******************************************************************************/
/**
* @file xpciepsu_caps.c
*
* Implements all of supportive functions to expose PCIe capabilities.
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
#include "xpciepsu_common.h"

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
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
*
* @return  offset to the first of the Function's capability register
* if available.	0x0 if not available
*
*******************************************************************************/
static u32 XPciePsu_GetBaseCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
	u8 Function)
{
	u32 CapBase = 0x0;

	XPciePsu_ReadConfigSpace(InstancePtr, Bus, Device, Function,
			XPCIEPSU_CFG_P_CAP_PTR_T1_REG, &CapBase);
	return (CapBase & XPCIEPSU_CAP_PTR_LOC);
}

/******************************************************************************/
/**
* This function returns whether capability Id is available or not for the
* particular Function.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
* @param   cap id to check capability pointer availability
*
* @return  u32 0 if capability is not available
* 1 if capability is available
*
*******************************************************************************/
u8 XPciePsu_HasCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{
	u8 CapStatus = CAP_NOT_PRESENT;
	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	while (CapBase) {
		XPciePsu_ReadConfigSpace(InstancePtr, Bus, Device,
					       Function, XPCIEPSU_DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (CapBase & XPCIEPSU_CFG_CAP_ID_LOC)){
			CapStatus =  CAP_PRESENT;
			goto End;
		}


		CapBase = (CapBase >> XPCIEPSU_CAP_SHIFT) & XPCIEPSU_CAP_PTR_LOC;
	}

End:
	return CapStatus;

}

/******************************************************************************/
/**
* This function returns offset to the matching capability ID from the
* Function's Linked list of the capability registers.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
* @param   cap id to get capability pointer offset
*
* @return  u64 capability pointer if available
* 0 if not available.
*
*******************************************************************************/
u64 XPciePsu_GetCapability(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function, u8 CapId)
{
	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);
	u32 Adr = 0;
	u64 Location = CAP_NOT_PRESENT;
	u32 Offset;


	while (CapBase) {
		Adr = CapBase;
		XPciePsu_ReadConfigSpace(InstancePtr, Bus, Device,
				Function, XPCIEPSU_DOUBLEWORD(CapBase), &CapBase);
		if (CapId == (CapBase & XPCIEPSU_CFG_CAP_ID_LOC)) {
			Offset = XPciePsu_ComposeExternalConfigAddress(
					Bus, Device, Function, XPCIEPSU_DOUBLEWORD(Adr));
			Location = (InstancePtr->Config.Ecam) + (Offset);
			goto End;
		}
		CapBase = (CapBase >> XPCIEPSU_CAP_SHIFT) & XPCIEPSU_CAP_PTR_LOC;
	}
End:
	return Location;
}

/******************************************************************************/
/**
* This function prints all the available capabilities in the Function.
*
* @param   InstancePtr pointer to XPciePsu Instance Pointer
* @param   Bus is the number of the Bus
* @param   Device is the number of the Device
* @param   Function is number of the Function
*
* @return  XST_SUCCESS on success
* XST_FAILURE on failure.
*
*******************************************************************************/
u8 XPciePsu_PrintAllCapabilites(XPciePsu *InstancePtr, u8 Bus, u8 Device,
		u8 Function)
{

	u32 CapBase = XPciePsu_GetBaseCapability(InstancePtr, Bus, Device,
			Function);

	xil_printf("CAP-IDs:");
	while (CapBase) {
		XPciePsu_ReadConfigSpace(InstancePtr, Bus, Device,
				Function, XPCIEPSU_DOUBLEWORD(CapBase), &CapBase);
		xil_printf("0x%X ", CapBase & XPCIEPSU_CFG_CAP_ID_LOC);
		CapBase = (CapBase >> XPCIEPSU_CAP_SHIFT) & XPCIEPSU_CAP_PTR_LOC;
	}
	xil_printf("\r\n");
	return XST_SUCCESS;
}
