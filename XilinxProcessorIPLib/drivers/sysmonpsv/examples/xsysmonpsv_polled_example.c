/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsv_polled_example.c
*
* This file contains a design example using the driver functions
* of the System Monitor driver. The example here shows the
* driver/device in polled mode to check the on-chip temperature and voltages.
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
* 1.0   aad    02/27/19 First release
* 1.1   aad    07/16/19 Added register unlock
* 1.1   aad    07/21/19 Added Temperature measurement
* 2.0   aad    10/29/20 Graceful exit when no supplies enabled
* 3.0   cog    03/25/22 Driver Restructure
* 3.1   cog    04/09/22 Fix supply iterator
* 5.0   se     08/01/24 Added new APIs to enable, set and get averaging for
*                       voltage supplies and temperature satellites.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv.h"
#include "xparameters.h"
#include "xstatus.h"
#include "stdio.h"

/************************** Function Prototypes *****************************/

int SysMonPsvPolledExample();

/****************************************************************************/
/**
*
* Main function that invokes the polled example in this file.
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
*****************************************************************************/
int main(void)
{
	int Status;

	/*
	 * Run the SysMonitor polled example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = SysMonPsvPolledExample();
	if (Status != XST_SUCCESS) {
		xil_printf("Sysmon Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Sysmon Polled Example Test\r\n");
	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the System Monitor device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the System Monitor device driver instance
*	- Read supply configuration.
*	- Read the latest on-chip temperatures and confiured supplies
*
* @return	- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int SysMonPsvPolledExample()
{
	XSysMonPsv InstancePtr;
	;
	float Voltage;
	float TempMin, TempMax;
	int Supply = 0;

	XSysMonPsv_Init(&InstancePtr, NULL);

	if (Supply != (int)EndList) {
		do {
			XSysMonPsv_ReadSupplyProcessed(&InstancePtr, Supply,
						       &Voltage);
			printf("Voltage for %s=%fv \r\n",
			       XSysMonPsv_Supply_Arr[Supply], Voltage);
			Supply++;
		} while (Supply != (int)EndList);
	} else {
		printf("No Supplies Enabled\r\n");
	}

	/* There is no polling mechanism to read the new temperature data */
	XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP_MIN,
				     &TempMin);
	printf("Current Minimum Temperature on the chip = %fC \r\n", TempMin);

	XSysMonPsv_ReadTempProcessed(&InstancePtr, XSYSMONPSV_TEMP_MAX,
				     &TempMax);
	printf("Current Maximum Temperature on the chip = %fC \r\n", TempMax);

	return XST_SUCCESS;
}
