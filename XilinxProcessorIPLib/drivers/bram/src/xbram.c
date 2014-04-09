/******************************************************************************
*
* (c) Copyright 2010-2013 Xilinx, Inc. All rights reserved.
*
* This file contains confidential and proprietary information of Xilinx, Inc.
* and is protected under U.S. and international copyright and other
* intellectual property laws.
*
* DISCLAIMER
* This disclaimer is not a license and does not grant any rights to the
* materials distributed herewith. Except as otherwise provided in a valid
* license issued to you by Xilinx, and to the maximum extent permitted by
* applicable law: (1) THESE MATERIALS ARE MADE AVAILABLE "AS IS" AND WITH ALL
* FAULTS, AND XILINX HEREBY DISCLAIMS ALL WARRANTIES AND CONDITIONS, EXPRESS,
* IMPLIED, OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF
* MERCHANTABILITY, NON-INFRINGEMENT, OR FITNESS FOR ANY PARTICULAR PURPOSE;
* and (2) Xilinx shall not be liable (whether in contract or tort, including
* negligence, or under any other theory of liability) for any loss or damage
* of any kind or nature related to, arising under or in connection with these
* materials, including for any direct, or any indirect, special, incidental,
* or consequential loss or damage (including loss of data, profits, goodwill,
* or any type of loss or damage suffered as a result of any action brought by
* a third party) even if such damage or loss was reasonably foreseeable or
* Xilinx had been advised of the possibility of the same.
*
* CRITICAL APPLICATIONS
* Xilinx products are not designed or intended to be fail-safe, or for use in
* any application requiring fail-safe performance, such as life-support or
* safety devices or systems, Class III medical devices, nuclear facilities,
* applications related to the deployment of airbags, or any other applications
* that could lead to death, personal injury, or severe property or
* environmental damage (individually and collectively, "Critical
* Applications"). Customer assumes the sole risk and liability of any use of
* Xilinx products in Critical Applications, subject only to applicable laws
* and regulations governing limitations on product liability.
*
* THIS COPYRIGHT NOTICE AND DISCLAIMER MUST BE RETAINED AS PART OF THIS FILE
* AT ALL TIMES.
*
******************************************************************************/

/**
* @file xbram.c
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
			u32 EffectiveAddr)
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

