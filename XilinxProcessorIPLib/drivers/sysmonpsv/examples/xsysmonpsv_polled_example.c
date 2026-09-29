/******************************************************************************
* Copyright (C) 2016 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xsysmonpsv_polled_example.c
*
* @brief Demonstrates the polled-mode usage of the System Monitor driver.
*
* This file contains a design example that uses the driver functions of the
* System Monitor driver. The example shows the driver and device in polled
* mode to check the on-chip temperature and voltages.
*
* @note        This example assumes that a STDIO device exists in the system.
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
* 5.2   se     08/24/25 Microblaze support added and processed values are
*                       printed on milli scale.
* 5.3   se     03/13/26 Fix secure mode and PCSR re-lock in SDT flow
*       se     03/20/26 Use static for driver instance structs
* 5.4   vn     09/29/26 Align Doxygen comments with AMD standards
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xsysmonpsv_hw.h"
#include "xsysmonpsv.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/
#if defined(XSYSMONPSV_SECURE_MODE)
#ifndef SDT
	/** IPI base address. */
	#define XSYSMONPSV_IPI_BASE_ADDR XPAR_XIPIPSU_0_BASEADDR
#else
	/** IPI base address. */
	#define XSYSMONPSV_IPI_BASE_ADDR XPAR_XIPIPSU_0_BASEADDR
#endif
#endif

/************************** Function Prototypes *****************************/
/** @cond INTERNAL */
int SysMonPsvPolledExample();
/** @endcond */

/****************************************************************************/
/**
*
* @brief Invokes the polled example in this file.
*
* @return      - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
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
* @brief Runs a test on the System Monitor device using the driver APIs.
*
* This function performs the following tasks:
*	- Initiates the System Monitor device driver instance.
*	- Reads the supply configuration.
*	- Reads the latest on-chip temperatures and configured supplies.
*
* @return      - XST_SUCCESS if the example has completed successfully.
*              - XST_FAILURE if the example has failed.
*
****************************************************************************/
int SysMonPsvPolledExample()
{
	static XSysMonPsv Instance;
	XSysMonPsv *InstancePtr = &Instance;
	float Voltage;
	float TempMin, TempMax;
	int Supply = 0;
	int Status;

#if defined(XSYSMONPSV_SECURE_MODE)
#ifndef SDT
	InstancePtr->IpiDeviceId = XPAR_XIPIPSU_0_DEVICE_ID;
#else
	InstancePtr->IpiBaseAddress = XSYSMONPSV_IPI_BASE_ADDR;
#endif
#endif

	Status = XSysMonPsv_Init(InstancePtr, NULL);
	if (Status != XST_SUCCESS) {
		xil_printf("XSysMonPsv_Init failed: %d\r\n", Status);
		return XST_FAILURE;
	}
	xil_printf("Entering the SysMon Polled Example\r\n");

	if (Supply != (int)EndList) {
		do {
			XSysMonPsv_ReadSupplyProcessed(InstancePtr, Supply,
						       &Voltage);
			xil_printf("\tVoltage for %s\t= %d mV \r\n",
				   XSysMonPsv_Supply_Arr[Supply],
				   (int)(Voltage * XSYSMONPSV_MILLI_SCALE));
			Supply++;
		} while (Supply != (int)EndList);
	} else {
		xil_printf("No Supplies Enabled\r\n");
	}

	/* There is no polling mechanism to read the new temperature data */
	XSysMonPsv_ReadTempProcessed(InstancePtr, XSYSMONPSV_TEMP_MIN,
				     &TempMin);
	xil_printf("\tCurrent Minimum Temperature on the chip = %d mDeg C \r\n",
		   (int)(TempMin * XSYSMONPSV_MILLI_SCALE));

	XSysMonPsv_ReadTempProcessed(InstancePtr, XSYSMONPSV_TEMP_MAX,
				     &TempMax);
	xil_printf("\tCurrent Maximum Temperature on the chip = %d mDeg C \r\n",
		   (int)(TempMax * XSYSMONPSV_MILLI_SCALE));

	return XST_SUCCESS;
}
