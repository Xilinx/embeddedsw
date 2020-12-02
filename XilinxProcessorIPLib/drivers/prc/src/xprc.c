/******************************************************************************
* Copyright (C) 2016 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xprc.c
* @addtogroup prc_v2_1
* @{
*
* This file contains the required functions for the XPrc driver. Refer xprc.h
* for a detailed description of the driver.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who     Date         Changes
* ---- ----- ------------  -----------------------------------------------
* 1.0   ms    07/18/2016    First release
* 1.1   ms    08/01/17      Added a new parameter "Cp_Compression" in
*                           "XPrc_CfgInitialize" function.
*                           Added new status error macros in function
*                           "XPrc_PrintVsmStatus".
* 1.2  Nava   29/03/19      Updated the tcl logic to generated the
*                           XPrc_ConfigTable properly.
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xprc.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Variable Definitions *****************************/

/************************** Function Prototypes ******************************/

static u32 XPrc_GetRegisterOffset(XPrc *InstancePtr, u32 VsmId, u8 BankId,
			u8 TableInBank, u8 TableRow);
static void XPrc_SendCommand(XPrc *InstancePtr, u16 VsmId, u8 Cmd, u8 Byte,
			u16 Halfword);
static u16 XPrc_GetOffsetForTableType(u8 TableId);

/****************************** Functions Definitions ************************/

/*****************************************************************************/
/**
*
* This function initializes a PRC instance.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	ConfigPtr points to the XPrc device configuration structure.
* @param	EffectiveAddr is the device base address in the virtual memory
*		address space. If the address translation is not used then the
*		physical address should be passed.
*
* @return
*		- XST_SUCCESS if initialisation was successful.
*
* @note		None.
*
******************************************************************************/
s32 XPrc_CfgInitialize(XPrc *InstancePtr, XPrc_Config *ConfigPtr,
			u32 EffectiveAddr)
{
	u8 Index;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(ConfigPtr != NULL);

	/**
	 * Set some default values for instance data, don't indicate the device
	 * is ready to use until everything has been initialized successfully.
	 */
	InstancePtr->Config.BaseAddress = EffectiveAddr;
	InstancePtr->Config.DeviceId = ConfigPtr->DeviceId;

	InstancePtr->Config.NumberOfVsms = ConfigPtr->NumberOfVsms;
	InstancePtr->Config.RequiresClearBitstreams =
				ConfigPtr->RequiresClearBitstreams;
	InstancePtr->Config.Cp_Arbitration_Protocol =
				ConfigPtr->Cp_Arbitration_Protocol;
	InstancePtr->Config.Has_Axi_Lite_If =
				ConfigPtr->Has_Axi_Lite_If;
	InstancePtr->Config.Reset_Active_Level =
				ConfigPtr->Reset_Active_Level;
	InstancePtr->Config.Cp_Fifo_Depth = ConfigPtr->Cp_Fifo_Depth;
	InstancePtr->Config.Cp_Fifo_Type = ConfigPtr->Cp_Fifo_Type;
	InstancePtr->Config.Cp_Family = ConfigPtr->Cp_Family;
	InstancePtr->Config.Cdc_Stages = ConfigPtr->Cdc_Stages;
	InstancePtr->Config.Cp_Compression = ConfigPtr->Cp_Compression;

	for(Index = 0; Index < InstancePtr->Config.NumberOfVsms; Index++) {
		InstancePtr->Config.NumberOfRms[Index] =
				ConfigPtr->NumberOfRms[Index];
		InstancePtr->Config.NumberOfRmsAllocated[Index] =
				ConfigPtr->NumberOfRmsAllocated[Index];
		InstancePtr->Config.Start_In_Shutdown[Index] =
				ConfigPtr->Start_In_Shutdown[Index];
		InstancePtr->Config.No_Of_Triggers_Allocated[Index] =
				ConfigPtr->No_Of_Triggers_Allocated[Index];
		InstancePtr->Config.Shutdown_On_Error[Index] =
				ConfigPtr->Shutdown_On_Error[Index];
		InstancePtr->Config.Has_Por_Rm[Index] =
				ConfigPtr->Has_Por_Rm[Index];
		InstancePtr->Config.Por_Rm[Index] = ConfigPtr->Por_Rm[Index];
		InstancePtr->Config.Has_Axis_Status[Index] =
				ConfigPtr->Has_Axis_Status[Index];
		InstancePtr->Config.Has_Axis_Control[Index] =
				ConfigPtr->Has_Axis_Control[Index];
		InstancePtr->Config.Skip_Rm_Startup_After_Reset[Index] =
				ConfigPtr->Skip_Rm_Startup_After_Reset[Index];
		InstancePtr->Config.Num_Hw_Triggers[Index] =
				ConfigPtr->Num_Hw_Triggers[Index];
	}

	InstancePtr->Config.RegVsmMsb = ConfigPtr->RegVsmMsb;
	InstancePtr->Config.RegVsmLsb = ConfigPtr->RegVsmLsb;
	InstancePtr->Config.RegBankMsb = ConfigPtr->RegBankMsb;
	InstancePtr->Config.RegBankLsb = ConfigPtr->RegBankLsb;
	InstancePtr->Config.RegSelectMsb = ConfigPtr->RegSelectMsb;
	InstancePtr->Config.RegSelectLsb = ConfigPtr->RegSelectLsb;

	/* Indicate the component is now ready to use */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function instructs the Virtual Socket Manager to enter the  shutdown
* state.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendShutdownCommand(XPrc *InstancePtr, u16 VsmId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	XPrc_SendCommand(InstancePtr, VsmId, XPRC_CR_SHUTDOWN_CMD,
			XPRC_CR_DEFAULT_BYTE, XPRC_CR_DEFAULT_HALFWORD);
}

/*****************************************************************************/
/**
*
* This function is used to restart a Virtual Socket Manager in shutdown if the
* Virtual Socket has not been modified during shutdown.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendRestartWithNoStatusCommand(XPrc *InstancePtr, u16 VsmId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	XPrc_SendCommand(InstancePtr, VsmId, XPRC_CR_RESTART_NO_STATUS_CMD,
			XPRC_CR_DEFAULT_BYTE, XPRC_CR_DEFAULT_HALFWORD);
}

/*****************************************************************************/
/**
*
* This function is used to restart a Virtual Socket Manager in shutdown if the
* Virtual Socket has been modified during shutdown.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	Full = 0, if Virtual Socket is empty,
*		Full = 1, if Virtual Socket is full.
* @param	RmId is the identifier of the Reconfigurable Module loaded
*		while the VSM was in shutdown.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendRestartWithStatusCommand(XPrc *InstancePtr, u16 VsmId, u8 Full,
			u16 RmId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);
	Xil_AssertVoid((Full == XPRC_CR_VS_FULL) ||
			(Full == XPRC_CR_VS_EMPTY));

	XPrc_SendCommand(InstancePtr, VsmId, XPRC_CR_RESTART_WITH_STATUS_CMD,
			Full, RmId);
}

/*****************************************************************************/
/**
*
* This function instructs the Virtual Socket Manager to proceed with
* processing the Reconfigurable Module.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendProceedCommand(XPrc *InstancePtr, u16 VsmId)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	XPrc_SendCommand(InstancePtr, VsmId, XPRC_CR_OK_TO_PROCEED_CMD,
			XPRC_CR_DEFAULT_BYTE, XPRC_CR_DEFAULT_HALFWORD);
}

/*****************************************************************************/
/**
*
* This function is used to set the values of Rm_Shutdown_Req, Rm_Decouple,
* Sw_Shutdown_Req, Sw_Startup_Req, Rm_Reset signals.
* It can only be used when the VSM is in the shutdown state.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	Rm_Shutdown_Req => Set to 1 to inform the RM Reconfigurable
*		Module that it is to be removed.
* @param	Rm_Decouple => Set to 1 to enable any external Virtual Socket
*		decoupling logic.
* @param	Sw_Shutdown_Req => Set to 1 to inform software that the active
*		Reconfigurable Module is to be removed.
* @param	Sw_Startup_Req => Set to 1 to inform software that the new
*		Reconfigurable Module has been loaded
* @param	Rm_Reset => Reset signal from the IP to the RM.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendUserControlCommand(XPrc *InstancePtr, u16 VsmId,
			u8 Rm_Shutdown_Req, u8 Rm_Decouple, u8 Sw_Shutdown_Req,
			u8 Sw_Startup_Req, u8 Rm_Reset)
{
	u8 Byte = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Byte = Rm_Shutdown_Req;	/* Bit 0 so I can start by just assigning */
	Byte |= (Rm_Decouple << XPRC_CR_USER_CONTROL_RM_DECOUPLE_BIT);
				/* Bit 1 */
	Byte |= (Sw_Shutdown_Req << XPRC_CR_USER_CONTROL_SW_SHUTDOWN_REQ_BIT);
				/* Bit 2 */
	Byte |= (Sw_Startup_Req << XPRC_CR_USER_CONTROL_SW_STARTUP_REQ_BIT);
				/* Bit 3 */
	Byte |= (Rm_Reset << XPRC_CR_USER_CONTROL_RM_RESET_BIT);
				/* Bit 4 */

	XPrc_SendCommand(InstancePtr, VsmId, XPRC_CR_USER_CTRL_CMD, Byte,
				XPRC_CR_DEFAULT_HALFWORD);
}

/*****************************************************************************/
/**
*
* This function is used to set a Trigger to Reconfigurable Module mapping in
* a Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	TriggerId specifies the trigger to modify.
* @param	RmId specifies Reconfigurable Module to load when trigger
*		TriggerId occurs.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SetTriggerToRmMapping(XPrc *InstancePtr, u16 VsmId, u16 TriggerId,
			u16 RmId)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_TRIGGER_REG, TriggerId);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetTriggerToRmMapping :: Instance"
		"%d, VSMID = %x, Address = %x, Trigger = %x, Rm = %x \n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, TriggerId,
		RmId);

	XPrc_WriteReg(Address, RmId);
}

/*****************************************************************************/
/**
*
* This function gets a Trigger to Reconfigurable Module mapping in a Virtual
* Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	TriggerId specifies the trigger to query.
*
* @return	The identifier of the Reconfigurable Module that will be loaded
*		when trigger TriggerId occurs.
*
* @note		None.
*
******************************************************************************/
u32 XPrc_GetTriggerToRmMapping(XPrc *InstancePtr, u16 VsmId, u16 TriggerId)
{
	u32 Address;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_TRIGGER_REG, TriggerId);

	return XPrc_ReadReg(Address);
}

/*****************************************************************************/
/**
*
* This function sets the BS_INDEX field in the RM_BS_INDEX register to specify
* which row of the Bitstream Information tables holds the partial bitstream for
* the Reconfigurable Module in the Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the partial bitstream for the
*		Reconfigurable Module.
*
* @return	None.
*
* @note		This overwrites the entire BS Index register so care is
*		required in Ultrascale parts where clearing bitstreams are
*		required. In those parts, use this first and then call
*		XPrc_SetRmClearingBsIndex.
*
******************************************************************************/
void XPrc_SetRmBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId, u16 BsIndex)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_RM_BS_INDEX_REG, RmId);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetRmBsIndex :: Instance %d,"
		"VSMID = %x, Address = %x, Rm = %x, Bs Index = %x \n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, RmId,
		BsIndex);

	XPrc_WriteReg(Address, BsIndex);
}

/*****************************************************************************/
/**
*
* This function set the CLEAR_BS_INDEX field in the RM_BS_INDEX register to
* specify which row of the BS Information tables holds the clearing bitstream
* for the Reconfigurable Module in the Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
*		bitstream.
* @param	ClearingBsIndex is the row number in the Bitstream Information
*		register bank that holds information about the clearing
*		bitstream for the Reconfigurable Module.
*
* @return	None.
*
* @note		This is only needed when the device being managed is an
*		UltraScale device. This must be called after XPrc_SetRmBsIndex
*
******************************************************************************/
void XPrc_SetRmClearingBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId,
			u16 ClearingBsIndex)
{
	u32 Address;
	u32 BsIndex;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_RM_BS_INDEX_REG, RmId);

	/* Read the register and merge in the clearing BS index */
	BsIndex = XPrc_ReadReg(Address);
	Data = BsIndex;
	Data |= (ClearingBsIndex << XPRC_RM_CLEARING_BS_INDEX_SHIFT);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetRmClearingBsIndex :: Instance"
		"%d, VSMID = %x, Address = %x, Rm = %x, Data = %x,"
		"Clearing Bs Index = %x, Bs Index = %x\n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, RmId, Data,
		ClearingBsIndex, BsIndex);

	XPrc_WriteReg(Address, Data);
}

/*****************************************************************************/
/**
*
* This function get the BS_INDEX field from the RM_BS_INDEX register for the
* Reconfigurable Module in the Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
*
* @return	Returns BS_INDEX.  This is the row number in the Bitstream
*		Information register bank that holds information about the
*		partial bitstream for the Reconfigurable Module.
*
* @note		None.
*
******************************************************************************/
u32 XPrc_GetRmBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId)
{
	u32 Address;
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_RM_BS_INDEX_REG, RmId);

	/* Read the register and merge in the clearing BS index */
	Data = XPrc_ReadReg(Address);
	/* Mask out any clearing BS index (only needed in Ultrascale) */
	Data &= XPRC_RM_BS_INDEX_MASK;

	return Data;
}

/*****************************************************************************/
/**
*
* This function get the CLEAR_BS_INDEX field from the RM_BS_INDEX register for
* the Reconfigurable Module in the Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
*
* @return	Returns CLEAR_BS_INDEX.  This is the row number in the
*		Bitstream Information register bank that holds information
*		about the clearing bitstream for the Reconfigurable Module.
*
* @note		This is only needed when the device being managed is an
*		UltraScale device.
*
******************************************************************************/
u16 XPrc_GetRmClearingBsIndex(XPrc *InstancePtr, u16 VsmId, u16 RmId)
{
	u32 Address;
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
				XPRC_RM_BS_INDEX_REG, RmId);

	/* Read the register and merge in the clearing BS index */
	Data = XPrc_ReadReg(Address);

	/* Remove the lower 16 bits which contain the BS Index */
	return (u16)(Data >> XPRC_RM_CLEARING_BS_INDEX_SHIFT);
}

/*****************************************************************************/
/**
*
* This function set the control information for Reconfigurable Module.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
* @param	ShutdownRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_NO_SHUTDOWN_REQUIRED(00): No Reconfigurable Module
*		  shutdown is required.
*		- XPRC_RM_CR_HW_SHUTDOWN_REQUIRED(01) : Hardware Reconfigurable
*		  Module shutdown is required.
*		- XPRC_RM_CR_HW_SW_SHUTDOWN_REQUIRED(10) : Hardware then
*		  software shutdown is required.
*		- XPRC_RM_CR_SW_HW_SHUTDOWN_REQUIRED(11) : Software then
*		  hardware shutdown is required.
* @param	StartupRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_STARTUP_NOT_REQUIRED (0) : No start-up is required
*		- XPRC_RM_CR_SW_STARTUP_REQUIRED (1) : Software start-up is
*		  required.
* @param	ResetRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_NO_RESET_REQUIRED(00): No Reconfigurable Module
*		  Reset is required.
*		- 01: RESERVED.
*		- XPRC_RM_CR_LOW_RESET_REQUIRED(10) : Active-Low Reconfigurable
*		  Module reset is required.
*		- XPRC_RM_CR_HIGH_RESET_REQUIRED(11) : Active-High
*		  Reconfigurable Module reset is required.
* @param	ResetDuration value for the Reconfigurable Module.
*		The maximum reset duration is 256 clock cycles.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SetRmControl(XPrc *InstancePtr, u16 VsmId, u16 RmId,
	u8 ShutdownRequired, u8 StartupRequired, u8 ResetRequired,
	u8 ResetDuration)
{
	u32 Address;
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);
	Xil_AssertVoid(ShutdownRequired <= XPRC_RM_CR_SW_HW_SHUTDOWN_REQUIRED);
	Xil_AssertVoid((StartupRequired == XPRC_RM_CR_STARTUP_NOT_REQUIRED) ||
			(StartupRequired == XPRC_RM_CR_SW_STARTUP_REQUIRED));
	Xil_AssertVoid(ResetRequired <= XPRC_RM_CR_HIGH_RESET_REQUIRED);
	Xil_AssertVoid(ResetDuration < XPRC_RM_CR_MAX_RESETDURATION);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_RM_CONTROL_REG, RmId);

	Data |= ((ShutdownRequired << XPRC_RM_CR_SHUTDOWN_REQUIRED_SHIFT) &
			XPRC_RM_CR_SHUTDOWN_REQUIRED_MASK);
	Data |= ((StartupRequired << XPRC_RM_CR_STARTUP_REQUIRED_SHIFT) &
			XPRC_RM_CR_STARTUP_REQUIRED_MASK);
	Data |= ((ResetRequired << XPRC_RM_CR_RESET_REQUIRED_SHIFT) &
			XPRC_RM_CR_RESET_REQUIRED_MASK);
	Data |= ((ResetDuration << XPRC_RM_CR_RESET_DURATION_SHIFT) &
			XPRC_RM_CR_RESET_DURATION_MASK);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetRmControl :: Instance %d,"
		"VSMID = %x, Address = %x, Data = %x (| Reset Duration %0d |"
		"Reset Required %0d | Startup Required = %0d | Shutdown"
		"Required = %0d |)\n\r", InstancePtr->Config.DeviceId,
		VsmId, Address, Data, ResetDuration, ResetRequired,
		StartupRequired, ShutdownRequired);

	XPrc_WriteReg(Address, Data);
}


/*****************************************************************************/
/**
*
* This function get the control information for Reconfigurable Module.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RmId is the identifier of the Reconfigurable Module.
* @param	ShutdownRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_NO_SHUTDOWN_REQUIRED(00): No Reconfigurable Module
*		  shutdown is required.
*		- XPRC_RM_CR_HW_SHUTDOWN_REQUIRED(01) : Hardware Reconfigurable
*		  Module shutdown is required.
*		- XPRC_RM_CR_HW_SW_SHUTDOWN_REQUIRED(10) : Hardware then
*		  software shutdown is required.
*		- XPRC_RM_CR_SW_HW_SHUTDOWN_REQUIRED(11) : Software then
*		  hardware shutdown is required.
* @param	StartupRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_STARTUP_NOT_REQUIRED (0) : No start-up is required
*		- XPRC_RM_CR_SW_STARTUP_REQUIRED (1) : Software start-up is
*		  required.
* @param	ResetRequired value for the Reconfigurable Module.
*		- XPRC_RM_CR_NO_RESET_REQUIRED(00): No Reconfigurable Module
*		  Reset is required.
*		- 01: RESERVED.
*		- XPRC_RM_CR_LOW_RESET_REQUIRED(10) : Active-Low Reconfigurable
*		  Module reset is required.
*		- XPRC_RM_CR_HIGH_RESET_REQUIRED(11) : Active-High
*		  Reconfigurable Module reset is required.
* @param	ResetDuration value for the Reconfigurable Module.
*		The maximum reset duration is 256 clock cycles.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_GetRmControl(XPrc *InstancePtr, u16 VsmId, u16 RmId,
			u8 *ShutdownRequired, u8 *StartupRequired,
			u8 *ResetRequired, u8 *ResetDuration)
{
	u32 Address;
	u32 Data;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_RM_CONTROL_REG, RmId);

	Data = XPrc_ReadReg(Address);

	if (ShutdownRequired != NULL) {
		*ShutdownRequired = (Data & XPRC_RM_CR_SHUTDOWN_REQUIRED_MASK)
				>> XPRC_RM_CR_SHUTDOWN_REQUIRED_SHIFT;
	}

	if (StartupRequired != NULL) {
		*StartupRequired = (Data & XPRC_RM_CR_STARTUP_REQUIRED_MASK)
				>> XPRC_RM_CR_STARTUP_REQUIRED_SHIFT;
	}

	if (ResetRequired != NULL) {
		*ResetRequired = (Data & XPRC_RM_CR_RESET_REQUIRED_MASK) >>
				XPRC_RM_CR_RESET_REQUIRED_SHIFT;
	}

	if (ResetDuration != NULL) {
		*ResetDuration = (Data & XPRC_RM_CR_RESET_DURATION_MASK) >>
				XPRC_RM_CR_RESET_DURATION_SHIFT;
	}

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_GetRmControl :: Instance %d,"
		"VSMID = %x, Address = %x, Data = %x (| Reset Duration %0d |"
		"Reset Required %0d | Startup Required = %0d | Shutdown"
		"Required = %0d |)\n\r", InstancePtr->Config.DeviceId,
		VsmId, Address, Data, *ResetDuration, *ResetRequired,
		*StartupRequired, *ShutdownRequired);

}

/*****************************************************************************/
/**
*
* This function sets the Bitstream Identifier.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
* @param	BsId is the identifier of the bitstream referenced by BsIdex.
*
* @return	None.
*
* @note		When the device being managed is 7 Series or UltraScale+,
*		BsID can only be 0.
*		When the device being managed is UltraScale, BsID can be 0 or 1
*
******************************************************************************/
void XPrc_SetBsId(XPrc *InstancePtr, u16 VsmId, u16 BsIndex, u16 BsId)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_ID_REG, BsIndex);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetBsId :: Instance %d, VSMID ="
		"%x, Address = %x, Bs Index = %x, Bs Id = %x \n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, BsIndex,
		BsId);

	XPrc_WriteReg(Address, BsId);
}

/*****************************************************************************/
/**
*
* This function get the Bitstream Identifier.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
*
* @return	BsId, which is the identifier of the bitstream referenced by
*		BsIdex.
*
* @note		When the device being managed is 7 Series or UltraScale+,
*		BsID can only be 0.
*		When the device being managed is UltraScale, BsID can be 0 or 1
*
******************************************************************************/
u32 XPrc_GetBsId(XPrc *InstancePtr, u16 VsmId, u16 BsIndex)
{
	u32 Address;
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_ID_REG, BsIndex);

	/* Read the register and merge in the clearing BS index */
	Data = XPrc_ReadReg(Address);

	return Data;
}

/*****************************************************************************/
/**
*
* This function sets the Bitstream size in bytes.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
* @param	BsSize is the bitstream size in bytes.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SetBsSize(XPrc *InstancePtr, u16 VsmId, u16 BsIndex, u32 BsSize)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_SIZE_REG, BsIndex);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetBsSize :: Instance %d, VSMID ="
		"%x, Address = %x, Bs Index = %x, Bs Size = %x \n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, BsIndex,
		BsSize);

	XPrc_WriteReg(Address, BsSize);
}

/*****************************************************************************/
/**
*
* This function get the Bitstream size in bytes.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
*
* @return	The bitstream size in bytes.
*
* @note		None.
*
******************************************************************************/
u32 XPrc_GetBsSize(XPrc *InstancePtr, u16 VsmId, u16 BsIndex)
{
	u32 Address;
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_SIZE_REG, BsIndex);


	Data = XPrc_ReadReg(Address);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_getBsSize :: Instance %d, VSMID ="
		"%x, Address = %x, Bs Index = %x, Bs Size = %x \n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, BsIndex,
		Data);

	return Data;
}

/*****************************************************************************/
/**
*
* This function sets the Address of the Bitstream.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
* @param	BsAddress is the address of the bitstream in memory.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SetBsAddress(XPrc *InstancePtr, u16 VsmId, u16 BsIndex,
		u32 BsAddress)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_ADDRESS_REG, BsIndex);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SetBsAddress :: Instance %d,"
		"VSMID = %x, Address = %x, Bs Index = %x, Bs Address ="
		"%x \n\r", InstancePtr->Config.DeviceId, VsmId, Address,
		BsIndex, BsAddress);

	XPrc_WriteReg(Address, BsAddress);
}

/*****************************************************************************/
/**
*
* This function get the Address of the Bitstream.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BsIndex is the row number in the Bitstream Information register
*		bank that holds information about the bitstream.
*
* @return	The address of the bitstream in memory
*
* @note		None.
*
******************************************************************************/
u32 XPrc_GetBsAddress(XPrc *InstancePtr, u16 VsmId, u16 BsIndex)
{
	u32 Address;
	u32 Data;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_BS_ADDRESS_REG, BsIndex);


	Data = XPrc_ReadReg(Address);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_getBsAddress:: Instance %d,"
		"VSMID = %x, Address = %x, Bs Index = %x, Bs Address ="
		"%x \n\r", InstancePtr->Config.DeviceId, VsmId, Address,
		BsIndex, Data);

	return Data;
}

/*****************************************************************************/
/**
*
* This function is used to read the VSM's Status Register.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
*
* @return	Returns the contents of the VSM's status register.
*
* @note		None.
*
******************************************************************************/
u32 XPrc_ReadStatusReg(XPrc *InstancePtr, u16 VsmId)
{
	u32 Address;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_STATUS_REG, XPRC_REG_TABLE_ROW);

	return XPrc_ReadReg(Address);
}

/*****************************************************************************/
/**
*
* This function is used to identify whether a VSM is in the Shutdown state or
* not.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is an identifier of the VSM to access or
*		a pre-read status word.
*
* @return
*		- XPRC_SR_SHUTDOWN_ON when VSM is in Shutdown State.
*		- XPRC_SR_SHUTDOWN_OFF when VSM is not in Shutdown State.
*
* @note		There are two modes of operation.  If InstancePtr is NULL then
*		VsmIdOrStatus contains a previously read status value.  Just
*		decode it. If InstancePtr is not NULL then read a fresh status
*		value.
*
******************************************************************************/
u8 XPrc_IsVsmInShutdown(XPrc *InstancePtr, u32 VsmIdOrStatus)
{
	u32 Status = VsmIdOrStatus;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	return (Status & XPRC_SR_SHUTDOWN_MASK) ? XPRC_SR_SHUTDOWN_ON :
		XPRC_SR_SHUTDOWN_OFF;
}

/*****************************************************************************/
/**
*
* This function is used to get a VSM's state.
* Each VSM exist in two states  1. Active State
*				2. Shutdown State
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is an identifier of the VSM to access or
*		a pre-read status word.
*
* @return	State of VSM.
*
* @note		There are two modes of operation.  If InstancePtr is NULL then
*		VsmIdOrStatus contains a previously read status value.  Just
*		decode it. If InstancePtr is not NULL then read a fresh status
*		value.
*
******************************************************************************/
u32 XPrc_GetVsmState(XPrc *InstancePtr, u32 VsmIdOrStatus)
{
	u32 Status = VsmIdOrStatus;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	return (Status & XPRC_SR_STATE_MASK);
}

/*****************************************************************************/
/**
*
* This function is used to get a VSM's Error Status.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is an identifier of the VSM to access or
*		a pre-read status word.
*
* @return
*		- 0 if there's no error.
*		- Error code if there is an error.
*
* @note		There are two modes of operation.  If InstancePtr is NULL then
*		VsmIdOrStatus contains a previously read status value. Just
*		decode it. If InstancePtr is not NULL then read a fresh status
*		value.
*
******************************************************************************/
u32 XPrc_GetVsmErrorStatus(XPrc *InstancePtr, u32 VsmIdOrStatus)
{
	u32 Status = VsmIdOrStatus;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	return (Status & XPRC_SR_ERROR_MASK) >> XPRC_SR_ERROR_SHIFT;
}

/*****************************************************************************/
/**
*
* This function is used to extract the identifier of a Reconfigurable Module
* from a VSM's status register.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is the identifier of the VSM to access or
* 		a pre-read status word.
*
* @return
* 		 - RM Identifier
*
* @note		There are two modes of operation.  If InstancePtr is NULL then
*		VsmIdOrStatus contains a previously read status value.  Just
*		decode it. If InstancePtr is not NULL then read a fresh status
*		value.
*
******************************************************************************/
u32 XPrc_GetRmIdFromStatus(XPrc *InstancePtr, u32 VsmIdOrStatus)
{
	u32 Status = VsmIdOrStatus;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	return (Status & XPRC_SR_RMID_MASK) >> XPRC_SR_RMID_SHIFT;
}

/*****************************************************************************/
/**
*
* This function is used to extract the identifier of a Bitstream from
* a VSM's status register.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is the identifier of the VSM to access or
* 		a pre-read status word.
*
* @return
* 		- BS Identifier
*
* @note		There are two modes of operation.  If InstancePtr is NULL then
*		VsmIdOrStatus contains a previously read status value.  Just
*		decode it. If InstancePtr is not NULL then read a fresh status
*		value.
*
******************************************************************************/
u32 XPrc_GetBsIdFromStatus(XPrc *InstancePtr, u32 VsmIdOrStatus)
{
	u32 Status = VsmIdOrStatus;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	return (Status & XPRC_SR_BSID_MASK) >> XPRC_SR_BSID_SHIFT;
}

/*****************************************************************************/
/**
*
* This function is used to Print the VSM Status.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmIdOrStatus is an identifier of the VSM to access or
* 		a pre-read status word.
* @param	Prefix is a text string that will prefix each line of output.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_PrintVsmStatus(XPrc *InstancePtr, u32 VsmIdOrStatus, char * Prefix)
{
	u32 Status = VsmIdOrStatus;
	u8 Is_shutdown;
	u8 State;
	u8 Error;

	if (InstancePtr != NULL) {
		Status = XPrc_ReadStatusReg(InstancePtr, VsmIdOrStatus);
	}

	Is_shutdown = XPrc_IsVsmInShutdown(NULL, Status);
	State = XPrc_GetVsmState(NULL, Status);
	Error = XPrc_GetVsmErrorStatus(NULL, Status);

	if (Is_shutdown != 0) {
		/* The STATE field doesn't represent the state. Instead, bit 0
		 * contains the value of RM_SHUTDOWN_ACK. That means we don't
		 * tell RM_ID or BS_ID either because they are only valid if
		 * the VSM isn'tempty, and we can't tell that in the shutdown
		 * mode.
		 */

		/* Shutdown mode */
		xprc_core_printf(XPRC_DEBUG_GENERAL,"%sMode : SHUTDOWN\n\r",
				Prefix);
		xprc_core_printf(XPRC_DEBUG_GENERAL,"%sRM_SHUTDOWN_ACK : %x\n\r",
				Prefix, State);
	}
	else {
		/* Active Mode */
		xprc_printf(XPRC_DEBUG_GENERAL,"%sMode : ACTIVE\n\r", Prefix);
		switch (State) {
			case XPRC_SR_STATE_EMPTY:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE :"
					"EMPTY (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_HW_SHUTDOWN:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : HW"
					"SHUTDOWN (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_SW_SHUTDOWN:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : SW"
					"SHUTDOWN (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_RM_UNLOAD:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : RM"
					"UNLOAD (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_RM_LOAD:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : RM"
					"LOAD (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_SW_STARTUP:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : SW"
					"STARTUP (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_RM_RESET:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : RM"
					"RESET (%x)\n\r", Prefix, State);
				break;
			case XPRC_SR_STATE_FULL:
				xprc_core_printf(XPRC_DEBUG_GENERAL,"%sSTATE : FULL"
					"(%x)\n\r", Prefix, State);
				break;
			default:
				break;
		}
		if (State != XPRC_SR_STATE_EMPTY) {
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sRM_ID : %d\n\r",
				Prefix, XPrc_GetRmIdFromStatus(NULL, Status));
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sBS_ID : %d\n\r",
				Prefix, XPrc_GetBsIdFromStatus(NULL, Status));
		}
	}
	switch (Error) {
		case XPRC_SR_BS_COMPATIBLE_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : BS"
				"COMPATIBLE ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_DECOMPRESS_BAD_FORMAT_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : DECOMPRESS"
				"BAD FORMAT ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_DECOMPRESS_BAD_SIZE_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : DECOMPRESS"
				"BAD SIZE ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_FETCH_AND_CP_LOST_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : FETCH AND"
				"CP LOST_ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_FETCH_AND_BS_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : FETCH AND BS"
				"ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_FETCH_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : FETCH ERROR"
				"(%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_CP_LOST_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : CP LOST"
				"ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_BS_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : BS ERROR"
				"(%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_BAD_CONFIG_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : BAD CONFIG"
				"ERROR (%x)\n\r", Prefix, Error);
			break;
		case XPRC_SR_NO_ERROR:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : NO ERROR"
				"(%x)\n\r", Prefix, Error);
			break;
		default:
			xprc_core_printf(XPRC_DEBUG_GENERAL,"%sERROR : UNKNOWN"
				"(%x)\n\r", Prefix, Error);
			break;
	}
}

/*****************************************************************************/
/**
*
* This function sends a software trigger to a Virtual Socket Manager.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	TriggerId is the trigger to send.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void XPrc_SendSwTrigger(XPrc *InstancePtr, u16 VsmId, u16 TriggerId)
{
	u32 Address;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_SW_TRIGGER_REG, XPRC_REG_TABLE_ROW);

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SendTrigger :: Instance %d,"
		"VSM_ID = %x, Address = %x, TriggerId = %x\n\r",
		InstancePtr->Config.DeviceId, VsmId, Address, TriggerId);

	XPrc_WriteReg(Address, TriggerId);
}

/*****************************************************************************/
/**
*
* This function is used to find out if there is a Software Trigger pending
* in a VSM.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	TriggerId is a pointer to a variable that will hold the trigger
*		ID that's in the register. This is the last trigger written
*		to the register and may or may not still be pending.
*
* @return
*		- XPRC_SW_TRIGGER_PENDING(1) if software trigger is pending
*		- XPRC_NO_SW_TRIGGER_PENDING(0) if software trigger is not
*		  pending.
*
* @note		None.
*
******************************************************************************/
u8 XPrc_IsSwTriggerPending(XPrc *InstancePtr, u16 VsmId, u16 *TriggerId)
{
	u32 Address;
	u32 Data = 0;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertNonvoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_SW_TRIGGER_REG, XPRC_REG_TABLE_ROW);
	Data = XPrc_ReadReg(Address);

	if (TriggerId != NULL) {
		*TriggerId = Data & XPRC_SW_TRIGGER_ID_MASK;
	}

	return (Data & XPRC_SW_TRIGGER_PENDING_MASK) ? XPRC_SW_TRIGGER_PENDING
			: XPRC_NO_SW_TRIGGER_PENDING;
}

/*****************************************************************************/
/**
*
* This function used to build the offset address for a register in a VSM.
* The register addresses are encoded as follows:
* [VSM Selector] [Bank Selector] [Row in the table][table][00]
*
* VSM Selector : The ID of the VSM.  0 = VSM0, 1 = VSM1, etc
* Table Selector : There are 4 tables of registers in each VSM. 3 are
*		   implemented as LUTRAM and 1 is just implemented using flops.
*			1. General Registers (Flops)
*			2. Trigger to RM Mapping Table (LUTRAM)
*			3. RM Information Table (LUTRAM)
*			4. BS Information Table (LUTRAM)
*
* Row in the table : This is the address of the register within the table.
*
* Table : The RM and BS banks implement several types of registers, each
*	  using its own LUTRAM table. This field specifies which LUTRAM
*	  to access.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	BankId is an identifier to represent the bank of registers to
*		access.
* @param	TableInBank is the table within the bank register.
* @param	TableRow is the address of the register within the table.
*
* @return	Offset of the register.
*
* @note		None.
*
******************************************************************************/
static u32 XPrc_GetRegisterOffset(XPrc *InstancePtr, u32 VsmId, u8 BankId,
			u8 TableInBank, u8 TableRow)
{
	u32 Offset = 0;

	if (InstancePtr != NULL) {
		Offset = VsmId << XPrc_GetRegVsmLsb(InstancePtr);
		Offset += BankId << XPrc_GetRegBankLsb(InstancePtr);
		Offset += TableRow << (XPrc_GetRegSelectLsb(InstancePtr) +
			XPrc_GetOffsetForTableType(BankId));
		Offset += TableInBank << XPrc_GetRegSelectLsb(InstancePtr);
	}

	return Offset;
}

/*****************************************************************************/
/**
*
* The register offsets for the PRC are dynamic. These could become awkward to
* use as each Trigger, VS, RM and BS require their own registers so looking up
* hard wired names would be clumsy.  For example a reference to a constant
* PRC_VS3_TRIGGER7 would be difficult to generate in a loop that iterated
* over each VS and Trigger.
*
* This function gets the address offset for a register in the PRC.
* As this is a Direct Hardware Interface, it cannot use device configuration
* parameters, and multiple instances are supported through the base address.
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	RegisterType is the type of register(XPRC_STATUS_REG,
*		XPRC_CONTROL_REG, XPRC_SW_TRIGGER_REG, XPRC_TRIGGER_REG, etc)
* @param	TableRow is the row of the table(trigger_id, rm_id,
*		rm_bs_index).
*
* @return	Address of the register.
*
* @note		None
*
******************************************************************************/
u32 XPrc_GetRegisterAddress(XPrc *InstancePtr, u32 VsmId,
			u8 RegisterType, u16 TableRow)
{
	/* XPRC_VSM_GENERAL_REG_BANK, XPRC_VSM_TRIGGER_REG_BANK,
	 * XPRC_VSM_RM_REG_BANK, XPRC_VSM_BS_REG_BANK
	 */
	u8 BankId;
	u8 TableInBank;

	switch (RegisterType) {
		/* The bank the reg is in; The table within the bank */
		case XPRC_STATUS_REG:
			BankId = XPRC_VSM_GENERAL_REG_BANK;
			TableInBank = XPRC_STATUS_REG_TABLE_ID;
			break;
		case XPRC_CONTROL_REG:
			BankId = XPRC_VSM_GENERAL_REG_BANK;
			TableInBank = XPRC_CONTROL_REG_TABLE_ID;
			break;
		case XPRC_SW_TRIGGER_REG:
			BankId = XPRC_VSM_GENERAL_REG_BANK;
			TableInBank = XPRC_SW_TRIGGER_REG_TABLE_ID;
			break;
		case XPRC_TRIGGER_REG:
			BankId = XPRC_VSM_TRIGGER_REG_BANK;
			TableInBank = XPRC_TRIGGER_REG_TABLE_ID;
			break;
		case XPRC_RM_BS_INDEX_REG:
			BankId = XPRC_VSM_RM_REG_BANK;
			TableInBank = XPRC_RM_BS_INDEX_REG_TABLE_ID;
			break;
		case XPRC_RM_CONTROL_REG:
			BankId = XPRC_VSM_RM_REG_BANK;
			TableInBank = XPRC_RM_CONTROL_REG_TABLE_ID;
			break;
		case XPRC_BS_ID_REG:
			BankId = XPRC_VSM_BS_REG_BANK;
			TableInBank = XPRC_BS_ID_REG_TABLE_ID;
			break;
		case XPRC_BS_ADDRESS_REG:
			BankId = XPRC_VSM_BS_REG_BANK;
			TableInBank = XPRC_BS_ADDRESS_REG_TABLE_ID;
			break;
		case XPRC_BS_SIZE_REG:
			BankId = XPRC_VSM_BS_REG_BANK;
			TableInBank = XPRC_BS_SIZE_REG_TABLE_ID;
			break;
		default:
			BankId = XPRC_DEFAULT_BANKID;
			TableInBank = XPRC_DEFAULT_TABLEID;
			break;
	}

	return (InstancePtr->Config.BaseAddress) + XPrc_GetRegisterOffset(
			InstancePtr, VsmId, BankId, TableInBank, TableRow);
}

/*****************************************************************************/
/**
*
* This function sends a command(Shutdown, Restart With No Status, Restart With
* Status, Proceed, User Control) to a Virtual Socket Manager
*
* @param	InstancePtr is a pointer to the PRC instance.
* @param	VsmId is the identifier of the VSM to access.
* @param	Cmd is a Control Register command (Shutdown, Restart with no
*		Status, Restart with Status, Proceed, User control).
* @param	Byte is an 8-bit field containing extra information for the
*		selected command.
* @param	Halfword is a 16-bit field containing extra information for
*		the selected command.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
static void XPrc_SendCommand(XPrc *InstancePtr, u16 VsmId, u8 Cmd, u8 Byte,
			u16 Halfword)
{
	u32 Address;
	u32 Data = 0;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);
	Xil_AssertVoid(Cmd <= XPRC_CR_USER_CTRL_CMD);
	Xil_AssertVoid(VsmId < XPRC_MAX_NUMBER_OF_VSMS);

	Address = XPrc_GetRegisterAddress(InstancePtr, VsmId,
			XPRC_CONTROL_REG, XPRC_REG_TABLE_ROW);

	/* Register format is
	 * 31       16 15   8 7        0
	 * | Halfword | Byte | Command |
	 */
	Data = (Halfword << XPRC_CR_HALFWORD_FIELD_LSB);
	Data |= (Byte << XPRC_CR_BYTE_FIELD_LSB);
	Data |= Cmd;

	xprc_printf(XPRC_DEBUG_GENERAL,"XPrc_SendCommand :: Instance %d,"
		"VSMID = %x, Address = %x, Data = %x (| Halfword %x | Byte"
		"%x | Cmd = %x |)\n\r", InstancePtr->Config.DeviceId,
		VsmId, Address, Data, Halfword, Byte, Cmd);

	XPrc_WriteReg(Address, Data);
}

/*****************************************************************************/
/**
*
* This function works out the register offset within a bank for a given
* register table. Registers in the PRC are organised into banks. Some banks
* only have one type of register. Some banks have multiple types of registers.
*
* @param	TableId is the register table.
*
* @return	Offset for the table within a bank.
*
* @note		None.
*
******************************************************************************/
static u16 XPrc_GetOffsetForTableType(u8 TableId)
{
	u16 Offset;

	/**
	 * For each given table type, work out how many LSBs are used to select
	 * a register type within a bank.
	 */
	switch (TableId) {
		case XPRC_VSM_RM_REG_BANK:
		/* RM Registers */
		/* The equation is ceil(log2(XPRC_VSM_REGISTERS_PER_RM)) */
			Offset = 1;
			break;
		case XPRC_VSM_BS_REG_BANK:
		/* BS Registers */
		/* The equation is * ceil(log2(XPRC_VSM_REGISTERS_PER_BS)) */
			Offset = 2;
			break;
		default:
			Offset = 0;
			break;
	}

	return Offset;
}

/** @} */
