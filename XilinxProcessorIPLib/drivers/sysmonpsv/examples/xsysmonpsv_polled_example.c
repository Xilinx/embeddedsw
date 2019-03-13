/******************************************************************************
*
* Copyright (C) 2018-2019 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
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
* 1.0   add    27/2/19 First release
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
#define SYSMONPSV_TIMEOUT	0xFFFFFFFE

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

#define printf xil_printf /* Small foot-print printf function */

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

	printf("\r\nEntering the SysMon Polled Example. \r\n");

	/* Initialize the SysMon driver. */
	ConfigPtr = XSysMonPsv_LookupConfig();
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	XSysMonPsv_CfgInitialize(SysMonInstPtr, ConfigPtr);

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

	printf("Voltage =%fv", Voltage);

	printf("Exiting the SysMon Polled Example. \r\n");

	return XST_SUCCESS;
}
