/******************************************************************************
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file xpmonpsv_selftest_example.c
* @addtogroup xpmonpsv_v2_0
* @{
*
* This file contains a diagnostic self test function for the XPmonPsv driver.
* The self test function does a simple read/write test of the Alarm Threshold
* Register.
*
*
* @note	None.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0 sd  01/20/19 First release
* 1.2 sd  11/14/19 Update the name of the device id.
* 2.0 sd  04/22/20 Change the file name.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xil_io.h"
#include "xuartpsv.h"
#include "xil_printf.h"
#include "xpmonpsv.h"
#include "xil_assert.h"
#include "sleep.h"

/************************** Constant Definitions ****************************/
#define TESTADDRESS  0x80000000; /* LPD AFI interface */
#define PMON_DEVICE_ID XPAR_XPMONPSV_1_DEVICE_ID


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger.
 */
static XPmonPsv PmonPsvInstance; /* The driver instance for PmonPsv Device */


/************************** Function Prototypes *****************************/

/*****************************************************************************/
/**
*
* Run a self-test on the driver/device. The test
*	- Resets the device,
*	- Writes a value into the Range Registers of Incrementer 0 and reads
*	  it back for comparison.
*	- Resets the device again.
*
*
* @param	InstancePtr is a pointer to the XPmonPsv instance.
*
* @return
*		- XST_SUCCESS if the value read from the Range Register of
*		  Incrementer 0 is the same as the value written.
*		- XST_FAILURE Otherwise
*
* @note		This is a destructive test in that resets of the device are
*		performed. Refer to the device specification for the
*		device status after the reset operation.
*
******************************************************************************/
static u32 XPmonPsv_SelfTest(XPmonPsv *InstancePtr)
{
	u32 Status = XST_FAILURE;
	u32 Regval = 0;
	u32 WriteRequestValue;
	u32 WriteRespValue;
	u32 ReadRequestValue;
	u32 ReadRespValue;
	u32 TestAddress;
	u32 CounterNum;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XPmonPsv_Unlock(InstancePtr);
	XPmonPsv_RequestCounter(InstancePtr,XPMONPSV_R5_DOMAIN, &CounterNum);
	XPmonPsv_ResetCounter(InstancePtr, XPMONPSV_R5_DOMAIN, CounterNum);
	XPmonPsv_SetSrc(InstancePtr, 0x6, XPMONPSV_R5_DOMAIN, CounterNum);
	XPmonPsv_SetPort(InstancePtr, 0x0, XPMONPSV_R5_DOMAIN, CounterNum);
	XPmonPsv_SetMetrics(InstancePtr, 0x1F, XPMONPSV_R5_DOMAIN, CounterNum);
	XPmonPsv_EnableCounters(InstancePtr, XPMONPSV_R5_DOMAIN, CounterNum);

	TestAddress =  TESTADDRESS;
	Xil_Out32( TestAddress, 0x0); /* lpd_afifs_axi_wr_req */
	Xil_Out32( TestAddress, 0x0); /* lpd_afifs_axi_wr_req */

	sleep(1);

	XPmonPsv_GetWriteCounter(InstancePtr,&WriteRequestValue, &WriteRespValue, XPMONPSV_R5_DOMAIN, CounterNum);

	xil_printf("\r\n TestAddress: %x \n", TestAddress);
	xil_printf("\r\n WriteRequestValue: %d WriteRespValue:%d\n", WriteRequestValue, WriteRespValue);

	if (WriteRequestValue == WriteRespValue) {
		Status = XST_SUCCESS;
	}

	XPmonPsv_Lock(InstancePtr);
	return Status;
}
/*****************************************************************************/
/**
*
* This function does a selftest on the PMONPSV device and coresight_apm driver as an
* example.
*
* @param	DeviceId is the XPAR_<PMONPSV_instance>_DEVICE_ID value from
*		xparameters.h.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
*
* @note		None.
*
****************************************************************************/
static u32 XpmonpsvSelfTestExample(u16 DeviceId)
{
	u32 Status;
	XPmonPsv_Config *ConfigPtr;/* Pointer to configuration data */

	/*
	 * Initialize the PmonPsv driver so that it is ready
	 * to use.
	 */
	ConfigPtr = XPmonPsv_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XPmonPsv_CfgInitialize(&PmonPsvInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status =  XPmonPsv_SelfTest(&PmonPsvInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;

}
/*****************************************************************************/
/**
*
* Main function to call the pmonpsv example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	u32 Status;

	/*
	 * Run the example, specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = XpmonpsvSelfTestExample(PMON_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("PmonPsv selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran PmonPsv selftest Example\r\n");
	return XST_SUCCESS;

}
/** @} */
