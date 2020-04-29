/******************************************************************************
* Copyright (C) 2010 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/**
* @file xbram.c
* @addtogroup bram_v4_4
* @{
*
* The implementation of the XBram driver's basic functionality.
* See xbram.h for more information about the driver.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sa   05/11/10 First release
* 3.01a sa   13/01/12 Added CorrectableFailingDataRegs and
*                     UncorrectableFailingDataRegs in
*					  XBram_CfgInitialize API.
* 4.1   sk   11/10/15 Used UINTPTR instead of u32 for Baseaddress CR# 867425.
*                     Changed the prototype of XBram_CfgInitialize API.
*</pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xbram.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/

/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/


/************************** Function Prototypes *****************************/


/****************************************************************************/
/**
* Initialize the XBram instance provided by the caller based on the given
* configuration data.
*
* Nothing is done except to initialize the InstancePtr.
*
* @param	InstancePtr is a pointer to an XBram instance.
*		The memory the pointer references must be pre-allocated by
*		the caller. Further calls to manipulate the driver through
*		the XBram API must be made with this pointer.
* @param	Config is a reference to a structure containing information
*		about a specific BRAM device. This function
*		initializes an InstancePtr object for a specific device
*		specified by the contents of Config. This function can
*		initialize multiple instance objects with the use of multiple
*		calls giving different Config information on each call.
* @param 	EffectiveAddr is the device base address in the virtual memory
*		address space. The caller is responsible for keeping the
*		address	mapping from EffectiveAddr to the device physical base
*		address	unchanged once this function is invoked. Unexpected
*		errors may occur if the address mapping changes after this
*		function is called. If address translation is not used, use
*		Config->BaseAddress for this parameters, passing the physical
*		address instead.
*
* @return
* 		- XST_SUCCESS	Initialization was successful.
*
* @note		None.
*
*****************************************************************************/
int XBram_CfgInitialize(XBram *InstancePtr,
			XBram_Config *Config,
			UINTPTR EffectiveAddr)
{
	/*
	 * Assert arguments
	 */
	Xil_AssertNonvoid(InstancePtr != NULL);

	/*
	 * Set some default values.
	 */
	InstancePtr->Config.CtrlBaseAddress = EffectiveAddr;
	InstancePtr->Config.MemBaseAddress = Config->MemBaseAddress;
	InstancePtr->Config.MemHighAddress = Config->MemHighAddress;
	InstancePtr->Config.DataWidth = Config->DataWidth;
	InstancePtr->Config.EccPresent = Config->EccPresent;
	InstancePtr->Config.FaultInjectionPresent =
					Config->FaultInjectionPresent;
	InstancePtr->Config.CorrectableFailingRegisters =
					Config->CorrectableFailingRegisters;
	InstancePtr->Config.CorrectableFailingDataRegs =
					Config->CorrectableFailingDataRegs;
	InstancePtr->Config.UncorrectableFailingRegisters =
					Config->UncorrectableFailingRegisters;
	InstancePtr->Config.UncorrectableFailingDataRegs =
					Config->UncorrectableFailingDataRegs;
	InstancePtr->Config.EccStatusInterruptPresent =
					Config->EccStatusInterruptPresent;
	InstancePtr->Config.CorrectableCounterBits =
					Config->CorrectableCounterBits;
	InstancePtr->Config.WriteAccess = Config->WriteAccess;

	/*
	 * Indicate the instance is now ready to use, initialized without error
	 */
	InstancePtr->IsReady = XIL_COMPONENT_IS_READY;
	return (XST_SUCCESS);
}

/** @} */
