/******************************************************************************
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
* @file xi3cpsx_slave_loopback.c
*
* This file contains a reference loopback example demonstrating the use of
* the Xilinx I3C controller operating in slave mode as part of a
* master-slave loopback configuration.
*
* In this setup:
*  - XPAR_XI3CPSX_0_BASEADDR is configured as the I3C slave
*  - XPAR_XI3CPSX_1_BASEADDR is configured as the I3C master
*
* The master transmits data to the slave, and the slave loops the received
* data back to the master. This validates basic I3C data transfer,
* reception, and response handling in slave mode.
*
* The example has been validated on the VEK385 evaluation board using
* an EMIO-based loopback connection between the master and slave
* instances.
*
* This application is intended to run in a bare-metal environment
* and does not rely on any operating system services.
*
* @note
*
* This is a reference example intended for functional validation.
* The data payload size must be less than or equal to the I3C FIFO size.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00a sd  06/10/22 First release
* 1.3   sd   11/17/23 Added support for system device-tree flow
* 1.4   gm   10/06/24 Added return statements, remove resetfifos and remove
*                     hard coded values.
* 1.7   vlt  12/18/25 Update Doxygen comments to include SDT flow details.
*       vlt  01/27/26 Fixed codespell issues
* </pre>
*
******************************************************************************/
/***************************** Include Files *********************************/

#include "xil_printf.h"
#include "xi3cpsx.h"
#include "xi3cpsx_hw.h"
#include "xi3cpsx_pr.h"

#ifndef SDT
#define I3C_MASTER_ID		XPAR_XI3CPSX_1_DEVICE_ID	/**< I3C Master Device Identifier */
#else
#define I3C_MASTER_ID		XPAR_XI3CPSX_1_BASEADDR		/**< I3C Master Device Identifier */
#endif

#ifndef SDT
#define I3C_SLAVE_ID		XPAR_XI3CPSX_0_DEVICE_ID
#else
#define I3C_SLAVE_ID		XPAR_XI3CPSX_0_BASEADDR
#endif
#define LEN 128
#define I3C_SLAVE_STATIC_ADDR	0x5d

u8 RxData[LEN];
u8 TxDataTest[LEN];

XI3cPsx Xi3cPs_InstanceMaster;
XI3cPsx *InstancePtr = &Xi3cPs_InstanceMaster;
XI3cPsx Xi3cPs_InstanceSlave;
XI3cPsx *InstancePtrSlave = &Xi3cPs_InstanceSlave;

#ifndef SDT
int I3cPsxSlaveLoopbackExample(u16 DeviceId);
#else
int I3cPsxSlaveLoopbackExample(UINTPTR BaseAddress);
#endif

/******************************************************************************/
/**
*
* Main function to call the polled example.
*
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
*******************************************************************************/
int main(void)
{
	int Status;

	xil_printf("I3C Slave Loopback Example Test \r\n");

	/*
	 * Run the I3c loopback example in slave mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
	Status = I3cPsxSlaveLoopbackExample(I3C_MASTER_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Slave Loopback Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Slave Loopback Test\r\n");
	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function does a slave loopback test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3cPsx driver.
*
* This function sends data and expects to receive the same data through the I3C
* slave loopback. It expects the controllers to be in loopback.
*
* This function uses slave driver mode of the I3C.
*
* @if SDT
* @param	BaseAddress contains the base address of the device
* @else
* @param	DeviceId is the Device ID of the I3cPsx Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
* @endif
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note	        In XSCT/classic flow, DeviceId is used to look up the device
*               configuration.
*
*******************************************************************************/
#ifndef SDT
int I3cPsxSlaveLoopbackExample(u16 DeviceId)
#else
int I3cPsxSlaveLoopbackExample(UINTPTR BaseAddress)
#endif
{
	s32 Ret;
	XI3cPsx_Config *CfgPtr;
	XI3cPsx_Cmd Cmd;
	int Index;

	/*
	 * Slave configuration
	 */
	CfgPtr = XI3cPsx_LookupConfig(I3C_SLAVE_ID);

	XI3cPsx_CfgInitialize(InstancePtrSlave, CfgPtr, CfgPtr->BaseAddress);

	XI3cPsx_SetupSlave(InstancePtrSlave, I3C_SLAVE_STATIC_ADDR);

	/*
	 * Master configuration
	 */
#ifndef SDT
	CfgPtr = XI3cPsx_LookupConfig(DeviceId);
#else
	CfgPtr = XI3cPsx_LookupConfig(BaseAddress);
#endif

	XI3cPsx_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	XI3cPsx_ResetFifos(InstancePtr);

	for (Index = 0; Index < LEN; Index++) {
		TxDataTest[Index] = Index;
		RxData[Index] = 0;
	}

	/*
	 * Transfer Argument
	 */
	Cmd.TransArg = COMMAND_PORT_ARG_DATA_LEN(LEN) | COMMAND_PORT_TRANSFER_ARG;

	/*
	 * Transfer Command
	 */
	Cmd.TransCmd = (COMMAND_PORT_SPEED(0) |
			COMMAND_PORT_DEV_INDEX(0) |
			COMMAND_PORT_TID(3) |
			COMMAND_PORT_TOC |
			COMMAND_PORT_ROC);
	Cmd.RxBuf = NULL;
	Ret = XI3cPsx_MasterSendPolled(InstancePtr,(u8 *) &TxDataTest, LEN, Cmd);
	if (Ret != XST_SUCCESS)
		return XST_FAILURE;

	Ret = XI3cPsx_SlaveRecvPolled(InstancePtrSlave, (u8 *)&RxData);
	if (Ret != XST_SUCCESS)
		return XST_FAILURE;

	for(Index = 0; Index < LEN; Index++)
	   xil_printf("Data from slave is  %d\n", RxData[Index]);

	for(Index = 0; Index < LEN; Index++){
		if(RxData[Index] != TxDataTest[Index]){
			xil_printf("Data mismatch at index 0x%x \r\n", Index);
			xil_printf("Expected 0x%x got 0x%x \r\n",TxDataTest[Index], RxData[Index]);
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}
