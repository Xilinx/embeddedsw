/******************************************************************************
* Copyright (C) 2012 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xaxipmon_polled_example.c
*
* This file contains a design example showing how to use the driver APIs of the
* AXI Performance Monitor driver in poll mode.
*
*
* @note
*
* Global Clock Counter and Metric Counters are enabled. The Application
* for which Metrics need to be computed should be run and then the Metrics
* collected.
*
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    02/29/12 First release
* 2.00a bss    06/23/12 Updated to support v2_00a version of IP.
* 3.00a bss    09/03/12 Deleted XAxiPmon_SetAgent API to support
*						v2_01a version of IP.
* 3.01a bss	   10/25/12 Deleted XAxiPmon_EnableCountersData API to support
*						new version of IP.
* 6.5   ms    01/23/17 Modified xil_printf statement in main function to
*                      ensure that "Successfully ran" and "Failed" strings are
*                      available in all examples. This is a fix for CR-965028.
* 6.10  ht    06/23/23 Added support for system device-tree flow.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xaxipmon.h"
#include "xparameters.h"
#include "xstatus.h"
#include "stdio.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#define AXIPMON_DEVICE_ID 	XPAR_AXIPMON_0_DEVICE_ID
#endif


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

#ifndef SDT
int AxiPmonPolledExample(u16 AxiPmonDeviceId, u32 *Metrics, u32 *ClkCntHigh,
			 u32 *ClkCntLow);
#else
int AxiPmonPolledExample(UINTPTR BaseAddress, u32 *Metrics, u32 *ClkCntHigh,
			 u32 *ClkCntLow);
#endif

/************************** Variable Definitions ****************************/

static XAxiPmon AxiPmonInst;      /* System Monitor driver instance */

/****************************************************************************/
/**
*
* Main function that invokes the example in this file.
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
	u32 Metrics;
	u32 ClkCntHigh = 0x0;
	u32 ClkCntLow = 0x0;

	/*
	 * Run the AxiPmonitor polled example, specify the Device ID that is
	 * generated in xparameters.h .
	 */
#ifndef SDT
	Status = AxiPmonPolledExample(AXIPMON_DEVICE_ID, &Metrics, &ClkCntHigh,
				      &ClkCntLow);
#else
	Status = AxiPmonPolledExample(XPAR_XAXIPMON_0_BASEADDR, &Metrics, &ClkCntHigh,
				      &ClkCntLow);
#endif

	if (Status != XST_SUCCESS) {
		xil_printf("AXI Performance Monitor Polled example failed\r\n");
		return XST_FAILURE;
	}


	xil_printf("Successfully ran AXI Performance Monitor Polled Example\r\n");
	return XST_SUCCESS;
}


/****************************************************************************/
/**
*
* This function runs a test on the AXI Performance Monitor device using the
* driver APIs.
* This function does the following tasks:
*	- Initiate the AXI Performance Monitor device driver instance
*	- Run self-test on the device
*	- Sets Metric Selector to select Slave Agent Number 1 and first set of
*	Metrics
*	- Enables Global Clock Counter and Metric Counters
*	- Calls Application for which Metrics need to be computed
*	- Disables Global Clock Counter and Metric Counters
*	- Read Global Clock Counter and Metric Counter 0
*
* @param	AxiPmonDeviceId is the XPAR_<AXIPMON_instance>_DEVICE_ID value
*		from xparameters.h.
* @param	Metrics is an user referece variable in which computed metrics
*			will be filled
* @param	ClkCntHigh is an user referece variable in which Higher 64 bits
*			of Global Clock Counter are filled
* @param	ClkCntLow is an user referece variable in which Lower 64 bits
*			of Global Clock Counter are filled
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
#ifndef SDT
int AxiPmonPolledExample(u16 AxiPmonDeviceId, u32 *Metrics, u32 *ClkCntHigh,
			 u32 *ClkCntLow)
#else
int AxiPmonPolledExample(UINTPTR BaseAddress, u32 *Metrics, u32 *ClkCntHigh,
			 u32 *ClkCntLow)
#endif
{
	int Status;
	XAxiPmon_Config *ConfigPtr;
	u8 SlotId = 0x0;
	u16 Range2 = 0x10;	/* Range 2 - 16 */
	u16 Range1 = 0x08;	/* Range 1 - 8 */
	XAxiPmon *AxiPmonInstPtr = &AxiPmonInst;

	/*
	 * Initialize the AxiPmon driver.
	 */
#ifndef SDT
	ConfigPtr = XAxiPmon_LookupConfig(AxiPmonDeviceId);
#else
	ConfigPtr = XAxiPmon_LookupConfig(BaseAddress);
#endif
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	XAxiPmon_CfgInitialize(AxiPmonInstPtr, ConfigPtr,
			       ConfigPtr->BaseAddress);

	/*
	 * Self Test the System Monitor/ADC device
	 */
	Status = XAxiPmon_SelfTest(AxiPmonInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Select Agent and required set of Metrics for a Counter.
	 * We can select another agent,Metrics for another counter by
	 * calling below function again.
	 */

	XAxiPmon_SetMetrics(AxiPmonInstPtr, SlotId, XAPM_METRIC_SET_0,
			    XAPM_METRIC_COUNTER_0);

	/*
	 * Set Incrementer Ranges
	 */
	XAxiPmon_SetIncrementerRange(AxiPmonInstPtr, XAPM_INCREMENTER_0,
				     Range2, Range1);
	/*
	 * Enable Metric Counters.
	 */
	XAxiPmon_EnableMetricsCounter(AxiPmonInstPtr);

	/*
	 * Enable Global Clock Counter Register.
	 */
	XAxiPmon_EnableGlobalClkCounter(AxiPmonInstPtr);

	/*
	 * Application for which Metrics has to be computed should be
	 * called here
	 */

	/*
	 * Disable Global Clock Counter Register.
	 */

	XAxiPmon_DisableGlobalClkCounter(AxiPmonInstPtr);

	/*
	 * Disable Metric Counters.
	 */
	XAxiPmon_DisableMetricsCounter(AxiPmonInstPtr);

	/* Get Metric Counter 0  */
	*Metrics = XAxiPmon_GetMetricCounter(AxiPmonInstPtr,
					     XAPM_METRIC_COUNTER_0);

	/* Get Global Clock Cycles Count in ClkCntHigh,ClkCntLow */
	XAxiPmon_GetGlobalClkCounter(AxiPmonInstPtr, ClkCntHigh, ClkCntLow);


	return XST_SUCCESS;

}
