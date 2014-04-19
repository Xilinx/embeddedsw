/******************************************************************************
*
* (c) Copyright 2014 Xilinx, Inc. All rights reserved.
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
* or any type of loss or damage suffered as a Result of any action brought by
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
/*****************************************************************************/
/**
*
* @file xsrio.c
* This file contains the required functions for the XSrio driver. See the 
* xsrio.h header file for more details on this driver.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   adk  16/04/14 Initial release
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xsrio.h"

/************************** Constant Definitions *****************************/

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/****************************************************************************/
/**
* Initialize the XSrio instance provided by the caller based on the
* given Config structure.
*
* @param        InstancePtr is the XSrio instance to operate on.
* @param        Config is the device configuration structure containing
*               information about a specific SRIO Device.
* @param        EffectiveAddress is the Physical address of the hardware in a
*               Virtual Memory operating system environment.It is the Base
*               Address in a stand alone environment.
*
* @return
*               - XST_SUCCESS Initialization was successful.
*
* @note         None.
*****************************************************************************/
int XSrio_CfgInitialize(XSrio *InstancePtr,
			XSrio_Config *Config, u32 EffectiveAddress)
{
	u32 Portwidth;

	InstancePtr->IsReady = 0;

	/* Setup the instance */
	memset(InstancePtr, 0, sizeof(XSrio));
	InstancePtr->Config.BaseAddress = EffectiveAddress;
	InstancePtr->Config.DeviceId = Config->DeviceId;
		
	/* Initialization is successful */
	InstancePtr->IsReady = 1;

	/* Configuration of the Device */
	InstancePtr->Config.IsPEMemory = Config->IsPEMemory;
	InstancePtr->Config.IsPEProcessor = Config->IsPEProcessor;
	InstancePtr->Config.IsPEBridge = Config->IsPEBridge;

	/* Port width for the Device */
	Portwidth = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
		XSRIO_PORT_N_ERR_STS_CSR_OFFSET + XSRIO_PORT_N_CTL_CSR_OFFSET);
	InstancePtr->PortWidth = ((Portwidth & XSRIO_PORT_N_CTL_PW_CSR_MASK) >> 
					XSRIO_PORT_N_CTL_PW_CSR_SHIFT);

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
* XSrio_GetPortStatus will check the status of the port and returns the status 
* of the port to the user
*
* @param	InstancePtr is the XSrio instance to operate on.
* 
* @return 
* 		- XSRIO_PORT_OK Port is initialized with no errors.
*	    	- XSRIO_PORT_UNINITIALIZED Port is not intilized. 
*		  No Serial Rapidio link is present.
*	    	- XSRIO_PORT_HAS_ERRORS Port is initialized but has errors.
*
* @note:   None.
*
****************************************************************************/
int XSrio_GetPortStatus(XSrio *InstancePtr)
{
	u32 Result;
	
	Xil_AssertNonvoid(InstancePtr != NULL);
	
	Result = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
			XSRIO_PORT_N_ERR_STS_CSR_OFFSET);
	if(Result & XSRIO_PORT_N_ERR_STS_POK_CSR_MASK)
		Result = XSRIO_PORT_OK;
	else if(Result & XSRIO_PORT_N_ERR_STS_PUINT_CSR_MASK)
		Result = XSRIO_PORT_UNINITIALIZED;
	else if(Result & XSRIO_PORT_N_ERR_STS_PERR_CSR_MASK)
		Result = XSRIO_PORT_HAS_ERRORS;

	return Result;
} 
 
/*****************************************************************************/
/**
* XSrio_GetPEType API will check for the Processing Element type and 
* return the type of type of Processing Element
*
* @param	InstancePtr is the XSrio instance to operate on.
* 
* @return 
*		- XSRIO_IS_MEMORY if the core is configured as a memory
*	    	- XSRIO_IS_PROCESSOR if the core is configured as a processor
*	    	- XSRIO_IS_BRIDGE if the core is configured as a bridge.
*
* @note:   None.
*
*****************************************************************************/
int XSrio_GetPEType(XSrio *InstancePtr)
{
	int Result;
	
	Xil_AssertNonvoid(InstancePtr != NULL);
	
	Result = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
				XSRIO_PEF_CAR_OFFSET);
	if(Result & XSRIO_PEF_MEMORY_CAR_MASK)
		Result = XSRIO_IS_MEMORY;
	else if(Result & XSRIO_PEF_PROCESSOR_CAR_MASK)
		Result = XSRIO_IS_PROCESSOR;
	else if(Result & XSRIO_PEF_BRIDGE_CAR_MASK)
		Result = XSRIO_IS_BRIDGE;

	return Result;
} 

/*****************************************************************************/
/**
* XSrio_IsOperationSupported tells whether the operation is supported by the
* SRIO Gen2 core or not.
*
* 
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param        Operation type is the operation type of the SRIO Packet 
* @param	Direction type 
*
* @return       
*               - XST_SUCCESS if the operation is supported by the core.
* 		- XST_FAILURE if the operation is not supported by the core.
* 
* @note		None.
*
*****************************************************************************/
int XSrio_IsOperationSupported(XSrio *InstancePtr, u8 Operation, u8 Direction)
{
	u32 OperationCar;
	u32 Status = XST_FAILURE;
	
	Xil_AssertNonvoid(InstancePtr != NULL);
	
	if (Direction & XSRIO_DIR_TX) {
		OperationCar = XSrio_ReadSrcOps(InstancePtr);
	} else {
		OperationCar = XSrio_ReadDstOps(InstancePtr);
	}
	
	switch (Operation) {
		case XSRIO_OP_MODE_NREAD:
			if(OperationCar & XSRIO_SRCDST_OPS_READ_CAR_MASK) 
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_NWRITE:
			if(OperationCar & XSRIO_SRCDST_OPS_WRITE_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_SWRITE:
			if(OperationCar & XSRIO_SRCDST_OPS_SWRITE_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_NWRITE_R:
			if(OperationCar & XSRIO_SRCDST_OPS_WRITE_RESPONSE_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_DATA_MESSAGE:
			if(OperationCar & XSRIO_SRCDST_OPS_DATA_MSG_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_DOORBELL:
			if(OperationCar & XSRIO_SRCDST_OPS_DOORBELL_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_ATOMIC:
			if(OperationCar & XSRIO_SRCDST_OPS_ATOMIC_SET_CAR_MASK)
				Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
	} 
	return Status;
}
