/******************************************************************************
 * Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
 * Copyright (C) 2022 - 2025 Advanced Micro Devices, Inc. All Rights Reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsv_intr_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor driver. The example here shows the
* driver/device in intr mode to check the on-chip temperature and voltages.
*
* @note
*
* This examples also assumes that there is a STDIO device in the system.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   aad    2/7/19   First release
* 1.2   aad    3/19/20  Fixed the interrupt disable flag
* 2.3   aad    9/28/21  Force generate of interrupt.
* 3.0   cog    03/25/21 Driver Restructure
* 4.0   se     11/10/22 Secure and Non-Secure mode integration
* 5.0   se     08/01/24 Added new APIs to enable, set and get averaging for
*                       voltage supplies and temperature satellites.
* 5.1   se     03/03/25 Compiler warnings fixed
* 5.2   se     08/24/25 Microblaze support added and processed values are
*                       printed on milli scale.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xil_printf.h"
#include "xsysmonpsv.h"
#include "sleep.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#if	defined(XSYSMONPSV_SECURE_MODE)
	#define XSYSMONPSV_IPI_INT_ID (XPAR_XIPIPSU_0_INT_ID)
	#define XSYSMONPSV_IPI_DEVICE_ID XPAR_XIPIPSU_0_DEVICE_ID
#endif
/****************************************************************************/
/************************** Function Prototypes *****************************/
int SysMonPsvIntrExample();
void Temp_CallbackFunc(void *data);
void Supply_CallbackFunc(void *data);
int TempInterruptOccured;
int SupplyInterruptOccured;
/****************************************************************************/
/**
 *
 * Main function that invokes the intr example in this file.
 *
 * @return	- XST_SUCCESS if the example has completed successfully.
 *		- XST_FAILURE if the example has failed.
 *
 *****************************************************************************/
int main(void)
{
	int Status;

	Status = SysMonPsvIntrExample();
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon Intr Example Test Failed\r\n");
		return XST_FAILURE;
	} else {
		xil_printf("Successfully Ran Sysmon Intr Example\r\n");
	}
	return XST_SUCCESS;
}

/****************************************************************************/
/**
 *
 * Callback function to notify temperature event.
 *
 * @param	Data is the data needed by the callback function.
 *
 *****************************************************************************/
void Temp_CallbackFunc(void *data)
{
	(void)data;

	TempInterruptOccured = 1;
}

/****************************************************************************/
/**
 *
 * Callback function to notify supply event.
 *
 * @param	Data is the data needed by the callback function.
 *
 *****************************************************************************/
void Supply_CallbackFunc(void *data)
{
	(void)data;

	SupplyInterruptOccured = 1;
}

/****************************************************************************/
/**
 *
 * This function runs a test on the System Monitor device using the
 * driver APIs.
 * This function does the following tasks:
 *	- Initiate the System Monitor device driver instance
 *	- Enable OT interrupt
 *	- Setup call back for interrupts
 *	- Read supply configuration.
 *	- Read the latest on-chip temperatures and confiured supplies
 *
 *
 * @return	- XST_SUCCESS if the example has completed successfully.
 *		- XST_FAILURE if the example has failed.
 *
 ****************************************************************************/
int SysMonPsvIntrExample()
{
	int ret;
	float ProcessedValue;
	XSysMonPsv Instance;
	XSysMonPsv *InstancePtr = &Instance;
#if defined (ARMR5) || defined (__aarch64__)
	XScuGic IntcInst;
#endif
#if defined (PLATFORM_MB)
	XIntc IntcInst;
#endif
	u32 ThresholdValue;
	u32 Val;
	XSysMonPsv_Supply Supply = (XSysMonPsv_Supply)0;

#if defined(XSYSMONPSV_SECURE_MODE)
	InstancePtr->IpiIntrId = XSYSMONPSV_IPI_INT_ID;
	InstancePtr->IpiDeviceId = XSYSMONPSV_IPI_DEVICE_ID;
#endif

	XSysMonPsv_Init(InstancePtr, &IntcInst);
	xil_printf("Entering the SysMon Intr Example\r\n");

	ret = XSysMonPsv_ReadTempProcessed(InstancePtr, XSYSMONPSV_TEMP_MAX,
					   &ProcessedValue);
	if (ret) {
		xil_printf("\tXSysMonPsv_ReadTempProcessed failed\r\n");
	} else {
		xil_printf("\tTemperature Max Processed Value %d mDeg C\r\n",
			   (int)(ProcessedValue * XSYSMONPSV_MILLI_SCALE));
	}

	ret = XSysMonPsv_ReadTempProcessed(InstancePtr, XSYSMONPSV_TEMP_MIN,
					   &ProcessedValue);
	if (ret) {
		xil_printf("\tXSysMonPsv_ReadTempProcessed failed\r\n");
	} else {
		xil_printf("\tTemperature Min Processed Value %d mDeg C\r\n\n",
			   (int)(ProcessedValue * XSYSMONPSV_MILLI_SCALE));
	}

	ret = XSysMonPsv_ReadTempRaw(InstancePtr, XSYSMONPSV_TEMP_MAX, &Val);
	if (ret) {
		xil_printf("\tXSysMonPsv_ReadTempRaw failed\r\n");
	} else {
		xil_printf("\tTemperature Max Raw Value 0x%x\r\n", (unsigned int)Val);
	}

	ret = XSysMonPsv_ReadTempRaw(InstancePtr, XSYSMONPSV_TEMP_MIN, &Val);
	if (ret) {
		xil_printf("\tXSysMonPsv_ReadTempRaw failed\r\n");
	} else {
		xil_printf("\tTemperature Min Raw Value 0x%x\r\n\n", (unsigned int)Val);
	}

	ret = XSysMonPsv_SetTempThresholdUpper(InstancePtr,
					       XSYSMONPSV_TEMP_EVENT, 0x7FFF);
	if (ret) {
		xil_printf("\tXSysMonPsv_SetTempThresholdUpper failed\r\n");
	}

	ret = XSysMonPsv_SetTempThresholdLower(InstancePtr,
					       XSYSMONPSV_TEMP_EVENT, 0x0);
	if (ret) {
		xil_printf("\tXSysMonPsv_SetTempThresholdLower failed\r\n");
	}

	XSysMonPsv_GetTempThresholdUpper(InstancePtr, XSYSMONPSV_TEMP_EVENT,
					 &ThresholdValue);
	xil_printf("\tTemperature Upper Threshold Value = 0x%x\r\n",
		   (unsigned int)ThresholdValue);
	XSysMonPsv_GetTempThresholdLower(InstancePtr, XSYSMONPSV_TEMP_EVENT,
					 &ThresholdValue);
	xil_printf("\tTemperature Lower Threshold Value = 0x%x\r\n\n",
		   (unsigned int)ThresholdValue);

	ret = XSysMonPsv_RegisterDeviceTempOps(InstancePtr, &Temp_CallbackFunc,
					       &Val);
	if (ret) {
		xil_printf("\tXSysMonPsv_RegisterDeviceTempOps failed\r\n");
	}

	/*Set Window Mode*/
	XSysMonPsv_SetTempMode(InstancePtr, 0);

	/* To Trigger Interrupt for temp*/
	TempInterruptOccured = 0;
	xil_printf("\tSet threshold and wait for Temperature Interrupt\r\n");
	XSysMonPsv_SetTempThresholdUpper(InstancePtr, XSYSMONPSV_TEMP_EVENT,
					 0x0);
	do {
		/* It is not mandatory to wait here by sleeping.
		* Sleep here just because of PMC EAM warnings
		* are printed to stdout at the same time with the following
		* printf call. It prevents collision on prints by giving
		* enough time without overlapping of messages.
		*/
		usleep(50000);
	}while(TempInterruptOccured == 0);
	xil_printf("\r\n\tInterrupt Triggered for Temperature\r\n");

	if (Supply != EndList) {
		ret = XSysMonPsv_ReadSupplyProcessed(InstancePtr, Supply,
						     &ProcessedValue);
		if (ret) {
			xil_printf("\tXSysMonPsv_ReadSupplyProcessed(%d) failed\r\n",
			       Supply);
		} else {
			xil_printf("\tSupply %s Processed Value %d mV\r\n",
				   XSysMonPsv_Supply_Arr[Supply],
			           (int)(ProcessedValue * XSYSMONPSV_MILLI_SCALE));
		}

		ret = XSysMonPsv_ReadSupplyRaw(InstancePtr, Supply, &Val);
		if (ret) {
			xil_printf("\tXSysMonPsv_ReadSupplyRaw(%d) failed\r\n",
			       Supply);
		} else {
			xil_printf("\tSupply %s Raw Value 0x%x\r\n\n",
			       XSysMonPsv_Supply_Arr[Supply], (unsigned int)Val);
		}

		/* Reset the thresholds to their saturation levels */
		XSysMonPsv_SetSupplyThresholdLower(InstancePtr, Supply, 0x0);
		XSysMonPsv_SetSupplyThresholdUpper(InstancePtr, Supply,
						   0x7fff);
		XSysMonPsv_GetSupplyThresholdUpper(InstancePtr, Supply,
						   &ThresholdValue);
		xil_printf("\tSupply Upper Threshold Value = 0x%x\r\n",
		       (unsigned int)ThresholdValue);
		XSysMonPsv_GetSupplyThresholdLower(InstancePtr, Supply,
						   &ThresholdValue);
		xil_printf("\tSupply Lower Threshold Value = 0x%x\r\n\n",
		       (unsigned int)ThresholdValue);

		ret = XSysMonPsv_RegisterSupplyOps(
			InstancePtr, Supply, &Supply_CallbackFunc, &Supply);
		if (ret) {
			xil_printf("\tXSysMonPsv_RegisterSupplyOps failed\r\n");
		}

		SupplyInterruptOccured = 0;
		XSysMonPsv_EnableVoltageEvents(InstancePtr, Supply, 0);

		/* To Trigger Interrupt for supply*/
		xil_printf("\tSet threshold and wait for Supply Interrupt\r\n");
		XSysMonPsv_SetSupplyThresholdUpper(InstancePtr, Supply, 0);
		do {
			/* It is not mandatory to wait here by sleeping.
			* Sleep here just because of PMC EAM warnings
			* are printed to stdout at the same time with the following
			* printf call. It prevents collision on prints by giving
			* enough time without overlapping of messages.
			*/
			usleep(50000);
		}while(SupplyInterruptOccured == 0);
		xil_printf("\r\n\tInterrupt Triggered for Supply\r\n");
	}
	return ret;
}
