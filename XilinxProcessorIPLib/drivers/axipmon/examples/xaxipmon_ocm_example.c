/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xaxipmon_ocm_example.c
*
* @brief Measures OCM traffic metrics using the AXI Performance Monitor driver.
*
* This file contains a design example that shows how to use the AXI Performance
* Monitor driver APIs. The example measures the following metrics captured at
* OCM of Zynq MP:
*	- Write Transaction Count
*	- Write Byte Count
*	- Read Transaction Count
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
* 6.10  ht     06/23/23 Added support for system device-tree flow.
* 6.14  vlt    01/27/26 Fixed codespell issues
* 6.15  vn     07/30/26 Fix Doxygen warnings
* 6.15  vn   09/08/26 Align Doxygen comments with AMD standards
* 6.15  vn   09/30/26 Update Doxygen comments to AMD standards
* </pre>
*
*****************************************************************************/

/***************************** Include Files *********************************/

#include "xaxipmon.h"
#include "xil_cache.h"
#include "xparameters.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
/** Device ID. */
#define AXIPMON_DEVICE_ID		XPAR_PSU_APM_1_DEVICE_ID
#endif

/** Sampling interval. */
#define SAMPLE_INTERVAL			0x100

/* OCM test Address */
/** @name OCM test addresses
 * @{
 */
/** OCM write test address. */
#define OCM_WRITE_ADDRESS		0xFFFE0000
/** OCM read test address. */
#define OCM_READ_ADDRESS		0xFFFF0000
/** @} */

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *******************************/

int ApmCheck(u32 *BufferPtr, u16 Wtx, u16 Wbc, u16 Rtx, u16 Rbc);
void OcmTransaction();
int ApmMetricConfig(XAxiPmon *ApmInst, u8 slot, u8 Metric1, u8 Metric2);
void ReadMetrics(XAxiPmon *ApmInst, u32 *buffer, u8 Counter1, u8 Counter2);

/************************** Variable Definitions ****************************/

static XAxiPmon  AxiPmonInst; /**< AXI Performance Monitor driver instance. */

u32 MetricsBuffer[6]; /**< Buffer for the collected metrics. */

/****************************************************************************/
/**
*
* @brief Invokes the AXI Performance Monitor OCM example.
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
*****************************************************************************/
int main(void)
{
	XAxiPmon_Config *ConfigPtr = NULL;
	u32 Status;

#ifndef SDT
	ConfigPtr = XAxiPmon_LookupConfig(AXIPMON_DEVICE_ID);
#else
	ConfigPtr = XAxiPmon_LookupConfig(XPAR_PERF_MONITOR_OCM_BASEADDR);
#endif
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
	 * 1 32 bit write transaction from A53 to OCM that is, 4 bytes
	 * 1 read transaction that is, 16 bytes at OCM (128 bit data bus)
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
* @brief Checks the metrics passed to this function are same as collected by
*        APM.
*
* @param[in]   BufferPtr Pointer to buffer containing captured metrics.
* @param[in]   Wtx Write transaction count value to be checked.
* @param[in]   Wbc Write byte count value to be checked.
* @param[in]   Rtx Read transaction count value to be checked.
* @param[in]   Rbc Read byte count value to be checked.
*
* @return
*              - XST_SUCCESS if metrics passed are equal to metrics collected by
*                APM.
*              - XST_FAILURE if any of the metrics passed is not equal to
*                metrics collected by APM.
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
* @brief Generates write and read transactions to OCM and reads metrics
*        captured by APM.
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
* @brief Sets Counter 0 and Counter 1 for collecting the passed metrics.
*
* @param[in]   InstancePtr Pointer to APM instance.
* @param[in]   slot APM slot.
* @param[in]   Metric1 Metric to be captured by Counter 1.
* @param[in]   Metric2 Metric to be captured by Counter 2.
*
* @return
*              - XST_SUCCESS on success.
*              - XST_FAILURE on failure.
*
*****************************************************************************/
int ApmMetricConfig(XAxiPmon *InstancePtr, u8 slot, u8 Metric1, u8 Metric2)
{
	int Status;

	/* Reset Metric counter and Global counters */
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
* @brief Reads metrics from APM counters to buffer.
*
* @param[in]   InstancePtr Pointer to APM instance.
* @param[out]  BufferPtr Pointer to buffer.
* @param[in]   Counter1 Counter number.
* @param[in]   Counter2 Counter number.
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
