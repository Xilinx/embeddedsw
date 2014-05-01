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
		
	/* Port width for the Device */
	Portwidth = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
		XSRIO_PORT_N_ERR_STS_CSR_OFFSET + XSRIO_PORT_N_CTL_CSR_OFFSET);
	InstancePtr->PortWidth = ((Portwidth & XSRIO_PORT_N_CTL_CSR_PW_MASK) >> 
					XSRIO_PORT_N_CTL_CSR_PW_SHIFT);

	/* Initialization is successful */
	InstancePtr->IsReady = 1;

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
*	    	- XSRIO_PORT_UNINITIALIZED Port is not initialized. 
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
	if(Result & XSRIO_PORT_N_ERR_STS_CSR_POK_MASK)
		Result = XSRIO_PORT_OK;
	else if(Result & XSRIO_PORT_N_ERR_STS_CSR_PUINT_MASK)
		Result = XSRIO_PORT_UNINITIALIZED;
	else if(Result & XSRIO_PORT_N_ERR_STS_CSR_PERR_MASK)
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
	if(Result & XSRIO_PEF_CAR_MEMORY_MASK)
		Result = XSRIO_IS_MEMORY;
	else if(Result & XSRIO_PEF_CAR_PROCESSOR_MASK)
		Result = XSRIO_IS_PROCESSOR;
	else if(Result & XSRIO_PEF_CAR_BRIDGE_MASK)
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
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_READ_MASK) 
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_NWRITE:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_WRITE_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_SWRITE:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_SWRITE_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_NWRITE_R:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_WRITE_RESPONSE_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_DATA_MESSAGE:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_DATA_MSG_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_DOORBELL:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_DOORBELL_MASK)
				Status = XST_SUCCESS;
			break;
		case XSRIO_OP_MODE_ATOMIC:
			if(OperationCar & XSRIO_SRCDST_OPS_CAR_ATOMIC_SET_MASK)
				Status = XST_SUCCESS;
			break;
		default:
			Status = XST_FAILURE;
	} 
	return Status;
}

/*****************************************************************************/
/**
* XSrio_SetWaterMark Configures the watermark to transfer a priority packet.
* 
* @param        InstancePtr is a pointer to the SRIO Gen2 instance to be
*               worked on.
* @param        WaterMark0 is the water mark value to transfer a priority 0
*		packet.
* @param	WaterMark1 is the water mark value to transfer a priority 1
*		packet.
* @param 	WaterMark2 is the water mark value to transfer a priority 2
*		packet.
*
* @return       None.              
* 
* @note		None.
*
*****************************************************************************/
void XSrio_SetWaterMark(XSrio *InstancePtr, u8 WaterMark0, u8 WaterMark1,
					u8 WaterMark2)
{
	int Regval;
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(WaterMark0 > WaterMark1);
	Xil_AssertVoid(WaterMark1 > WaterMark2);
	Xil_AssertVoid(WaterMark2 > 0);
	
	Regval = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
				XSRIO_IMP_WCSR_OFFSET);
				
	if (WaterMark0 > WaterMark1) {
		Regval = ((Regval & ~XSRIO_IMP_WCSR_WM0_MASK) | 
				(WaterMark0 & XSRIO_IMP_WCSR_WM0_MASK));
		XSrio_WriteReg((InstancePtr)->Config.BaseAddress, 
				XSRIO_IMP_WCSR_OFFSET, Regval);
	} 
	
	if(WaterMark1 > WaterMark2) {
		Regval = ((Regval & ~XSRIO_IMP_WCSR_WM1_MASK) | 
			((WaterMark1 << XSRIO_IMP_WCSR_WM1_SHIFT) & 
				XSRIO_IMP_WCSR_WM1_MASK));
		XSrio_WriteReg((InstancePtr)->Config.BaseAddress, 
				XSRIO_IMP_WCSR_OFFSET, Regval);
	}

	if(WaterMark2 > 0) {
		Regval = ((Regval & ~XSRIO_IMP_WCSR_WM2_MASK) | 
			((WaterMark2 << XSRIO_IMP_WCSR_WM2_SHIFT) & 
				 XSRIO_IMP_WCSR_WM2_MASK));
		XSrio_WriteReg((InstancePtr)->Config.BaseAddress, 
				XSRIO_IMP_WCSR_OFFSET, Regval);
	}
}

/*****************************************************************************/
/**
* XSrio_GetWaterMark API reads the water mark values.
*
* @param	InstancePtr is the XSrio instance to operate on.
* @param	WaterMark0 is a pointer to a variable where the driver will pass
*		back the water mark 0 value.
* @param	WaterMark1 is a pointer to a variable where the driver will pass
*		back the water mark 1 value.
* @param	WaterMark2 is a pointer to a variable where the driver will pass
*		back the water mark 2 value.
* 
* @return 	None.
*
* @note:        None.
*
*****************************************************************************/
void XSrio_GetWaterMark(XSrio *InstancePtr, u8 *WaterMark0, u8 *WaterMark1,
					u8 *WaterMark2)
{
	int Regval;
	Xil_AssertVoid(InstancePtr != NULL);
	
	Regval = XSrio_ReadReg(InstancePtr->Config.BaseAddress,
				XSRIO_IMP_WCSR_OFFSET);
				
	*WaterMark0 = (Regval & XSRIO_IMP_WCSR_WM0_MASK);
	*WaterMark1 = ((Regval & XSRIO_IMP_WCSR_WM1_MASK) >> 
			XSRIO_IMP_WCSR_WM1_SHIFT);
	*WaterMark2 = ((Regval & XSRIO_IMP_WCSR_WM2_MASK) >> 
			XSRIO_IMP_WCSR_WM2_SHIFT);

}
