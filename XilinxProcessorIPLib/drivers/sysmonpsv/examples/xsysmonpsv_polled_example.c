/******************************************************************************
* Copyright (C) 2018 - 2020 Xilinx, Inc.  All rights reserved.
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
* 1.0   add    02/27/19 First release
* 1.1   add    07/16/19 Added register unlock
* 1.1   add    07/21/19 Added Temperature measurement
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

/************************** Constant Definitions ****************************/
#define INTR_0		0
#define SYSMONPSV_TIMEOUT	100000
#define LOCK_CODE		0xF9E8D7C6

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/



/************************** Function Prototypes *****************************/

int SysMonPsvPolledExample();

/************************** Variable Definitions ****************************/

static XSysMonPsv SysMonInst;      /* System Monitor driver instance */

/****************************************************************************/
/**
*
* Main function that invokes the polled example in this file.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None.
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
* @param	None.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note		None
*
****************************************************************************/
int SysMonPsvPolledExample()
{
	int Status, Timeout;
	XSysMonPsv_Config *ConfigPtr;
	u32 IntrStatus;
	u32 RawVoltage;
	float Voltage;
	XSysMonPsv *SysMonInstPtr = &SysMonInst;
	XSysMonPsv_Supply Supply = 0;
	u32 CurrentMin, CurrentMax;
	float TempMin, TempMax;

	printf("\r\nEntering the SysMon Polled Example. \r\n");

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);

	/* Unlock the sysmon register space */
	XSysMonPsv_WriteReg(SysMonInstPtr->Config.BaseAddress + XSYSMONPSV_PCSR_LOCK,
			    LOCK_CODE);

	/* Clear any bits set in the Interrupt Status Register. */
	IntrStatus = XSysMonPsv_IntrGetStatus(SysMonInstPtr);
	XSysMonPsv_IntrClear(SysMonInstPtr, IntrStatus);

	/* Enable New Data Interrupt in the IER0 register.
	 * This MASK covers for Supplies that have a value from 0-31.
	 * For values between 32-63, 64-95, 96-127 and 128-159, use
	 * XSYSMONPSV_IER0_NEW_DATA*_MASK, where * is 1, 2, 3, 4 respectively.
	 * Similarly, use XSYSMON_IER0_ALARM*_mask depending on the alarms to
	 * be enabled.
	 * in the Supply_List of XSysMonPsv_Config. The index of the array
	 * corresponds to the enum XSysMonPsv_Supply from
	 * xsysmonpsv_supplylist.h
	 */
	XSysMonPsv_SetNewDataIntSrc(SysMonInstPtr, Supply,
				    XSYSMONPSV_IER0_NEW_DATA0_MASK);

	XSysMonPsv_IntrEnable(SysMonInstPtr, XSYSMONPSV_IER0_NEW_DATA0_MASK,
			      INTR_0);

	Timeout = 0;
	/* Wait till new data is available */
	do {
		Timeout++;
		if(Timeout > SYSMONPSV_TIMEOUT)
			return XST_FAILURE;
	} while (!(XSysMonPsv_IntrGetStatus(SysMonInstPtr) &
		  XSYSMONPSV_IER0_NEW_DATA0_MASK));

	/* Read the desired Supply from the enabled supplies from the
	 * configuration using enum XSysmonPsv_Supply from
	 * xsysmonpsv_supplylist.h
	 * This enum is generated depending on the configuration selected
	 * for sysmon in the PCW.
	 */

	RawVoltage = XSysMonPsv_ReadSupplyValue(SysMonInstPtr, Supply,
						XSYSMONPSV_VAL);

	Voltage = XSysMonPsv_RawToVoltage(RawVoltage);

	printf("Voltage =%fv \r\n", Voltage);

	/* There is no polling mechanism to read the new temperature data */
	CurrentMin = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr, XSYSMONPSV_VAL_VREF_MIN);
	TempMin = XSysMonPsv_FixedToFloat(CurrentMin);
	printf("Current Minimum Temperature on the chip = %fC \r\n", TempMin);

	CurrentMax = XSysMonPsv_ReadDeviceTemp(SysMonInstPtr, XSYSMONPSV_VAL_VREF_MAX);
	TempMax = XSysMonPsv_FixedToFloat(CurrentMax);
	printf("Current Maximum Temperature on the chip = %fC \r\n", TempMax);



	printf("Exiting the SysMon Polled Example. \r\n");

	return XST_SUCCESS;
}
