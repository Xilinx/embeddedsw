
/******************************************************************************
* Copyright (C) 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*****************************************************************************/
/**
 * @file xi3c_daa_example.c
 *
 * Design example to use the I3C device as master in polled mode.
 *
 * It assigns the dynamic addressed to slave addresses.
 * It sends and receives data from slave.
 *
 *
 * Note: Max 108 dynamic addresses are available unconditionally.
 * User need to provide dynamic addresses from the valid dynamic addresses list.
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver   Who Date     Changes
 * ----- --- -------- -----------------------------------------------
 * 1.00 gm   02/09/24 First release
 *
 * </pre>
 *
 ****************************************************************************/

#include "xil_printf.h"
#include "xi3c.h"
#include "xi3c_hw.h"

#ifndef SDT
#define XI3C_DEVICE_ID		XPAR_XI3C_0_DEVICE_ID
#else
#define XI3C_BASEADDRESS	XPAR_XI3C_0_BASEADDR
#endif

#define I3C_DATALEN		10
#define I3C_SLAVES_COUNT	1

XI3c Xi3c_Instance;
XI3c *InstancePtr = &Xi3c_Instance;

u8 Dynamic_Addr[] = {0x08};	/**< 0x08 to 0x3D are the 1st 54 valid dynamic addresses */

/************************** Function Prototypes *******************************/

#ifndef SDT
int I3cMasterDaaExample(u16 DeviceId);
#else
int I3cMasterDaaExample(UINTPTR BaseAddress);
#endif
/******************************************************************************/
/******************************************************************************/
/**
*
* Main function to call the daa example.
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

	xil_printf("I3C Master Dynamic address assignment Example Test \r\n");

	/*
	 * Run the I3c polled example in master mode, specify the Device
	 * ID that is specified in xparameters.h.
	 */
#ifndef SDT
	Status = I3cMasterDaaExample(XI3C_DEVICE_ID);
#else
	Status = I3cMasterDaaExample(XI3C_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("I3C Master Dynamic address assignment Example Test Failed, Status = 0x%x\r\n", Status);
		return XST_FAILURE;
	}

	xil_printf("Successfully ran I3C Master Dynamic address assignment Example Test\r\n");
	return XST_SUCCESS;
}


/*****************************************************************************/
/**
*
* This function does a minimal test on the I3c device and driver as a
* design example. The purpose of this function is to illustrate
* how to use the XI3c driver.
*
* This function assigns dynamic addresses to slave devices.
* It sends data and receives the data through the I3C
*
* This function uses polled driver mode of the I3C.
*
* @param	DeviceId is the Device ID of the I3c Device and is the
*		XPAR_<I3C_instance>_DEVICE_ID value from xparameters.h
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note
*
*******************************************************************************/
#ifndef SDT
int I3cMasterDaaExample(u16 DeviceId)
#else
int I3cMasterDaaExample(UINTPTR BaseAddress)
#endif
{
	int Status;
	XI3c_Config *CfgPtr;
	u8 TxData[I3C_DATALEN];
	u8 RxData[I3C_DATALEN];
	XI3c_Cmd Cmd;
	u16 Index;

#ifndef SDT
	CfgPtr = XI3c_LookupConfig(DeviceId);
#else
	CfgPtr = XI3c_LookupConfig(BaseAddress);
#endif
	if (NULL == CfgPtr) {
		return XST_FAILURE;
	}
	XI3c_CfgInitialize(InstancePtr, CfgPtr, CfgPtr->BaseAddress);

	XI3c_ResetFifos(InstancePtr);

	/*
	 * Assign dynamic addresses to slave devices
	 */
	Status = XI3c_DynaAddrAssign(InstancePtr, Dynamic_Addr, I3C_SLAVES_COUNT);

	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Slave information
	 */

	for (Index = 0; Index < I3C_SLAVES_COUNT; Index++) {
		xil_printf("Slave %d details:\n", Index);
		xil_printf("ID  = 0x%lx\n", XI3c_SlaveInfoTable[Index].Id);
		xil_printf("BCR = 0x%d\n", XI3c_SlaveInfoTable[Index].Bcr);
		xil_printf("DCR = 0x%d\n", XI3c_SlaveInfoTable[Index].Dcr);
	}

	/*
	 * Fill data to buffer
	 */

	for (Index = 0; Index < I3C_DATALEN; Index++) {
		TxData[Index] = Index;		/** < Test data */
		RxData[Index] = 0;
	}

	for (Index = 0; Index < I3C_SLAVES_COUNT; Index++) {
		/*
		 * Send
		 */
		XI3c_ResetFifos(InstancePtr);
		Cmd.SlaveAddr = Dynamic_Addr[Index];
		Cmd.NoRepeatedStart = 1;
		Cmd.Tid = 0;
		Cmd.Pec = 0;
		Cmd.CmdType = 1;               	/**< SDR mode */
		Status = XI3c_MasterSendPolled(InstancePtr, &Cmd, TxData, I3C_DATALEN);

		if (Status != XST_SUCCESS) {
			return Status;
		}

		/*
		 * Recv
		 */
		XI3c_ResetFifos(InstancePtr);
		Cmd.SlaveAddr = Dynamic_Addr[Index];
		Cmd.NoRepeatedStart = 1;
		Cmd.Tid = 0;
		Cmd.Pec = 0;
		Cmd.CmdType = 1;
		Status = XI3c_MasterRecvPolled(InstancePtr, &Cmd, RxData, I3C_DATALEN);

		if (Status != XST_SUCCESS) {
			return Status;
		}

		xil_printf("Rx Data:\n");
		for (Index = 0; Index < I3C_DATALEN; Index++) {
			xil_printf("0x%x  ", RxData[Index]);
		}
		xil_printf("\n");
	}
	return XST_SUCCESS;
}
