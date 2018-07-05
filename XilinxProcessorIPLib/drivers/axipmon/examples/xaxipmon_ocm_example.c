/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xaxipmon_ocm_example.c
*
* This file contains a design example showing how to use the driver APIs of the
* AXI Performance Monitor driver to measure the following metrics captured at
* OCM of Zynq MP:
*	- Write Transcation Count
*	- Write Byte Count
*	- Read Transcation Count
*	- Read Byte Count
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.00a bss    04/01/15 First release
* 6.5   sg     11/02/16 Updated device ID to OCM PSU APM ID
*       ms     01/23/17 Modified xil_printf statement in main function to
*                       ensure that "Successfully ran" and "Failed" strings are
*                       available in all examples. This is a fix for CR-965028.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xaxipmon.h"
#include "xil_cache.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define AXIPMON_DEVICE_ID		XPAR_PSU_APM_1_DEVICE_ID

/* Sampling interval */
#define SAMPLE_INTERVAL			0x100

/* OCM test Address */
#define OCM_WRITE_ADDRESS		0xFFFE0000
#define OCM_READ_ADDRESS		0xFFFF0000

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *******************************/

int ApmCheck(u32 *BufferPtr, u16 Wtx, u16 Wbc, u16 Rtx, u16 Rbc);
void OcmTransaction();
int ApmMetricConfig(XAxiPmon *ApmInst, u8 slot, u8 Metric1, u8 Metric2);
void ReadMetrics(XAxiPmon *ApmInst, u32 *buffer, u8 Counter1, u8 Counter2);

/************************** Variable Definitions ****************************/

static XAxiPmon  AxiPmonInst;

u32 MetricsBuffer[6];

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
	XAxiPmon_Config *ConfigPtr = NULL;
	u32 Status;

	ConfigPtr = XAxiPmon_LookupConfig(AXIPMON_DEVICE_ID);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XAxiPmon_CfgInitialize(&AxiPmonInst, ConfigPtr,
									ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Xil_DCacheDisable();

	OcmTransaction();

	xil_printf("OCM APM Monitor results\n");

	/*
	 * 1 32 bit write transaction from A53 to OCM i.e, 4 bytes
	 * 1 read trasaction i.e, 16 bytes at OCM (128 bit data bus)
	 * A53 -> Interconnect -> APM -> OCM
	 */
	Status = ApmCheck(MetricsBuffer, 1, 4, 1, 16);

	XAxiPmon_DisableMetricsCounter(&AxiPmonInst);
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran AXI Performance Monitor OCM \
						Example\r\n");
		return XST_SUCCESS;
	} else {
		xil_printf("AXI Performance Monitor OCM example Failed\r\n");
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Checks the metrics passed to this function are same as collected by APM.
*
* @param	BufferPtr is pointer to buffer containing captured metrics.
* @param	Wtx is Write transaction count value to be checked.
* @param	Wbc is Write byte count value to be checked.
* @param	Rtx is Read transaction count value to be checked.
* @param	Rbc is Read byte count value to be checked.
*
* @return
*		- XST_SUCCESS if metrics passed are equal to metrics collected by APM.
*		- XST_FAILURE if any of the metrics passed is not equal to metrics
*						collected by APM.
*
* @note		None.
*
*****************************************************************************/
int ApmCheck(u32 *BufferPtr, u16 Wtx, u16 Wbc, u16 Rtx, u16 Rbc)
{
	int Errors = 0;

	xil_printf("Write Transaction Count	: %d\r\n", BufferPtr[0]);
	xil_printf("Write Byte Count		: %d\r\n", BufferPtr[1]);
	xil_printf("Read Transaction Count	: %d\r\n", BufferPtr[3]);
	xil_printf("Read Byte Count			: %d\r\n", BufferPtr[4]);

	if (Wtx != BufferPtr[0]) {
		xil_printf("write tx count fail\r\n");
		Errors = Errors + 1;
	}

	if (Wbc != BufferPtr[1]) {
		xil_printf("write byte count fail\r\n");
		Errors = Errors + 1;
	}

	if (Rtx != BufferPtr[3]) {
		xil_printf("read tx count fail\n");
		Errors = Errors + 1;
	}

	if (Rbc != BufferPtr[4]) {
		xil_printf("read byte count fail\r\n");
		Errors = Errors + 1;
	}

	if (Errors > 0) {
		return XST_FAILURE;
	} else {
		return XST_SUCCESS;
	}
}

/****************************************************************************/
/**
*
* Generates write and read transcations to OCM and reads metrics captured
* by APM.
*
* @param	None.
*
* @return	None.
*
* @note		None.
*
*****************************************************************************/
void OcmTransaction()
{

	ApmMetricConfig(&AxiPmonInst, 0, XAPM_METRIC_SET_0, XAPM_METRIC_SET_2);

	/* write to OCM */
	Xil_Out32(OCM_WRITE_ADDRESS, 0xBAADFACE);

	ReadMetrics(&AxiPmonInst, &MetricsBuffer[0], XAPM_METRIC_COUNTER_0,
				XAPM_METRIC_COUNTER_1);

	ApmMetricConfig(&AxiPmonInst, 0, XAPM_METRIC_SET_1, XAPM_METRIC_SET_3);

	/* read from OCM */
	Xil_In32(OCM_READ_ADDRESS);

	ReadMetrics(&AxiPmonInst, &MetricsBuffer[3], XAPM_METRIC_COUNTER_0,
				XAPM_METRIC_COUNTER_1);
}

/****************************************************************************/
/**
* Sets Counter 0 and Counter 1 for collecting the passed metrics
*
* @param    InstancePtr is pointer to APM instance.
* @param    Slot is APM slot.
* @param    Metric1 is Metric to be captured by Counter 1
* @param    Metric2 is Metric to be captured by Counter 2
*
* @return
*       - XST_SUCCESS on success.
*       - XST_FAILURE on failure.
*
* @note     None.
*
*****************************************************************************/
int ApmMetricConfig(XAxiPmon *InstancePtr, u8 slot, u8 Metric1, u8 Metric2)
{
	int Status;

	/* reset Metric conter and Global counters */
	XAxiPmon_ResetMetricCounter(InstancePtr);
	XAxiPmon_ResetGlobalClkCounter(InstancePtr);

	Status = XAxiPmon_SetMetrics(InstancePtr, slot, Metric1,
								 XAPM_METRIC_COUNTER_0);
	if (Status == XST_FAILURE) {
		return XST_FAILURE;
	}
	Status = XAxiPmon_SetMetrics(InstancePtr, slot, Metric2,
								XAPM_METRIC_COUNTER_1);
	if (Status == XST_FAILURE) {
		return XST_FAILURE;
	}

	/* Enable Metric counter */
	XAxiPmon_EnableMetricsCounter(InstancePtr);

	/* Start Metric counter */
	XAxiPmon_StartCounters(InstancePtr, SAMPLE_INTERVAL);

	return XST_SUCCESS;
}

/****************************************************************************/
/**
*
* Reads metrics from APM counters to buffer.
*
* @param    InstancePtr is pointer to APM instance.
* @param    BufferPtr is pointer to buffer.
* @param    Counter1 is counter number.
* @param    Counter2 is counter number.
*
* @return	None.
*
* @note     None.
*
*****************************************************************************/
void ReadMetrics(XAxiPmon *InstancePtr, u32 *BufferPtr, u8 Counter1,
				u8 Counter2)
{
	/* Stop Counters */
	XAxiPmon_StopCounters(InstancePtr);

	BufferPtr[0] = XAxiPmon_GetMetricCounter(InstancePtr, Counter1);
	BufferPtr[1] = XAxiPmon_GetMetricCounter(InstancePtr, Counter2);
}
