/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
*
* @file xpmonpsv_selftest.c
* @addtogroup xpmonpsv_v1_0
* @{
*
* This file contains a diagnostic self test function for the XpsvPmon driver.
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
#define PMON_DEVICE_ID XPAR_PSU_CORESIGHT_LPD_ATM_DEVICE_ID


/**************************** Type Definitions ******************************/

/***************** Macros (Inline Functions) Definitions ********************/

/************************** Variable Definitions ****************************/
/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger.
 */
static XpsvPmon PsvPmonInstance; /* The driver instance for Psvpmon Device */


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
* @param	InstancePtr is a pointer to the XpsvPmon instance.
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
static s32 XpsvPmon_SelfTest(XpsvPmon *InstancePtr)
{
	s32 Status;
	u32 Regval = 0;
	u32 WriteRequestValue;
	u32 WriteRespValue;
	u32 ReadRequestValue;
	u32 ReadRespValue;
	u32 TestAddress;
	u32 CounterNum;

	Xil_AssertNonvoid(InstancePtr != NULL);
	Xil_AssertNonvoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	XpsvPmon_Unlock(InstancePtr);
	XpsvPmon_RequestCounter(InstancePtr,XPMONPSV_R5_DOMAIN, &CounterNum);
	XpsvPmon_ResetCounter(InstancePtr, XPMONPSV_R5_DOMAIN, CounterNum);
	XpsvPmon_SetSrc(InstancePtr, 0x6, XPMONPSV_R5_DOMAIN, CounterNum);
	XpsvPmon_SetPort(InstancePtr, 0x0, XPMONPSV_R5_DOMAIN, CounterNum);
	TestAddress =  TESTADDRESS;
	XpsvPmon_SetMetrics(InstancePtr, 0x1F, XPMONPSV_R5_DOMAIN, CounterNum);
	XpsvPmon_EnableCounters(InstancePtr, XPMONPSV_R5_DOMAIN, CounterNum);

	Xil_Out32( TestAddress, 0x0); /* lpd_afifs_axi_wr_req */
	Regval = Xil_In32(TestAddress);
	Xil_Out32( TestAddress, 0x0); /* lpd_afifs_axi_wr_req */
	Regval = Xil_In32(TestAddress);

	sleep(1);

	XpsvPmon_GetWriteCounter(InstancePtr,&WriteRequestValue, &WriteRespValue, XPMONPSV_R5_DOMAIN, CounterNum);
	XpsvPmon_GetReadCounter(InstancePtr,&ReadRequestValue, &ReadRespValue, XPMONPSV_R5_DOMAIN, CounterNum);

	xil_printf("\r\n TestAddress: %x \n", TestAddress);
	xil_printf("\r\n WriteRequestValue: %d WriteRespValue:%d\n", WriteRequestValue, WriteRespValue);
	xil_printf("\r\n ReadRequestValue: %d ReadRespValue:%d\n", ReadRequestValue, ReadRespValue);

	Status = XST_SUCCESS;
	if (WriteRequestValue != WriteRespValue)
		Status = XST_FAILURE;

	if (ReadRequestValue != ReadRespValue)
		Status = XST_FAILURE;

	XpsvPmon_Lock(InstancePtr);
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
	XPmonpsv_Config *ConfigPtr;/* Pointer to configuration data */

	/*
	 * Initialize the PmonPsv driver so that it is ready
	 * to use.
	 */
	ConfigPtr = XpsvPmon_LookupConfig(DeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	Status = XpsvPmon_CfgInitialize(&PsvPmonInstance, ConfigPtr, ConfigPtr->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status =  XpsvPmon_SelfTest(&PsvPmonInstance);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

}
s32 main(void)
{
	u32 Status;

	/*
	 * Run the example, specify the device ID that is generated in
	 * xparameters.h.
	 */
	Status = XpmonpsvSelfTestExample(PMON_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Pmonpsv selftest Example Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Pmonpsv selftest Example\r\n");
	return XST_SUCCESS;

}
/** @} */
