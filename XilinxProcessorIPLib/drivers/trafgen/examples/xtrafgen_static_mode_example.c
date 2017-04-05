/******************************************************************************
*
* Copyright (C) 2013 - 2014 Xilinx, Inc.  All rights reserved.
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
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
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
/*****************************************************************************/
/**
 *
 * @file xtrafgen_static_mode_example.c
 *
 * This file demonstrates how to use the xtrafgen driver on the Xilinx AXI
 * Traffic Generator core. The AXI Traffic Generator IP is designed to 
 * generate AXI4 traffic which can be used to stress different modules/
 * interconnect connected in the system.
 *
 * This example demonstrates  how to use the Static mode in the Axi Traffic 
 * Genrator.In Static mode the core continously genrates fixed address and 
 * fixed INCR type read and write transfers based on the burst length 
 * configured.
 * 
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.01a adk  03/09/13 First release
 * 2.00a adk  16/09/13 Fixed CR:737291
 * 4.1   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 *       ms   04/05/17 Added tabspace for return statements in functions for
 *                     proper documentation while generating doxygen.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xtrafgen.h"
#include "xparameters.h"
#include "xil_exception.h"

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#define TRAFGEN_DEV_ID	XPAR_XTRAFGEN_0_DEVICE_ID
#define BURST_LEN	255
#define MEM_ADDR	XPAR_AXI_BRAM_CTRL_2_S_AXI_BASEADDR
#define DATA 		0xc0015afe

#undef DEBUG

/************************** Function Prototypes ******************************/
int XTrafGenStaticModeExample(XTrafGen *InstancePtr, u16 DeviceId);
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XTrafGen XTrafGenInstance;

/*****************************************************************************/
/**
*
* Main function
*
* This function is the main entry of the traffic generator test. 
*
* @param        None
*
* @return
*				- XST_SUCCESS if tests pass
*               - XST_FAILURE if fails.
*
* @note         None.
*
******************************************************************************/
int main()
{
	int Status;

	xil_printf("--- Entering main() ---\n\r");

	Status = XTrafGenStaticModeExample(&XTrafGenInstance, TRAFGEN_DEV_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("TrafficGen Static mode Example Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran TrafficGen Static mode Example\n\r");
	xil_printf("--- Exiting main() ---\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* This function demonstrates the usage Traffic Generator
* It does the following:
*       - Set up the output terminal if UART16550 is in the hardware build
*       - Initialize the AXI Traffic Generator device
*	- Fill's the Target memory with required data
*	- Enable the Traffic genration
*	- Disable the Traffic genration
*       - Return test status and exit
*
* @param	InstancePtr is a pointer to the instance of the
*		XTrafGen component.
* @param	DeviceId is Device ID of the Axi Traffic Generator Device,
*
*
* @param	InstancePtr is a pointer to the instance of the
*			XTrafGen component.
* @param	DeviceId is Device ID of the Axi Traffic Generator Device,
*			typically XPAR_<TRAFGEN_instance>_DEVICE_ID value from
*			xparameters.h.
*
* @return
*		-XST_SUCCESS to indicate success
*		-XST_FAILURE to indicate failure
*
******************************************************************************/
int XTrafGenStaticModeExample(XTrafGen *InstancePtr, u16 DeviceId)
{

	XTrafGen_Config *Config;
	int Status = XST_SUCCESS;
	u32 Count;
	int *Addr = MEM_ADDR;
	Count = 0;
	
        /* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif

	/* Initialize the Device Configuration Interface driver */
	Config = XTrafGen_LookupConfig(DeviceId);
	if (!Config) {
		xil_printf("No config found for %d\r\n", DeviceId);
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XTrafGen_CfgInitialize(InstancePtr, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}
	
	/* Check for the Static Mode */
	if(InstancePtr->OperatingMode != XTG_MODE_STATIC) {
		return XST_FAILURE;
	}
	
	/* Fill the BRAM with the data */
	for (Count = 0; Count < (5 * BURST_LEN); Count++) {
		*(Addr + Count*4) = 0;
		if(*(Addr + Count*4) != 0x00) {
			xil_printf("ERROR: Memory Addr 0x%x = 0x%x \t"
					"Expected = 0x00\n\r"
					,(Addr+Count*4),(Addr+Count*4));
			return XST_FAILURE;
		}
	}
	
	/* Set the Required Burst length */
	XTrafGen_SetStaticBurstLen(InstancePtr, BURST_LEN);
	
	/* Disable the traffic genration */
	XTrafGen_StaticDisable(InstancePtr);
	
	/* Check for the Static control DONE Bit resets */
	XTrafGen_SetStaticTransferDone(InstancePtr);
	Status = XTrafGen_IsStaticTransferDone(InstancePtr);
	if (Status != TRUE) {
		xil_printf("Static control reset failed failed\n\r");
		return Status;
	}
	
	/* Enable the traffic genration */
	XTrafGen_StaticEnable(InstancePtr);
	
	/* Disable the traffic genration */
	XTrafGen_StaticDisable(InstancePtr);
	
	/* Wait till data transmission completes */
	do {
		Status = XTrafGen_GetStaticTransferDone(InstancePtr);
	}while(Status != XTG_STREAM_CNTL_TD_MASK);

	
	/* Check for the data 
	 * Note:" 0xc0015afe " is the fixed data that has been given by the core
	 * continuously when ATG is configured in STATIC mode and RUN.
	 * There is no way to change this DATA in SW.
	 */
	for (Count = 0; Count < (BURST_LEN + 1); Count++) {
		if(*(u32 *)(MEM_ADDR + Count*4) != DATA) {
			xil_printf("ERROR: Memory Addr 0x%x = 0x%x \t"
					"Expected = 0xc0015afe\n\r"
					,(Addr + Count*4),(Addr + Count*4));
			return XST_FAILURE;
		}
	}

	for (Count = (BURST_LEN+1); Count < (BURST_LEN + 4); Count++) {
		if(*(Addr + Count*4) != 0x00) {
			xil_printf("ERROR: Memory Addr 0x%x = 0x%x \t"
					"Expected = 0x00\n\r"
					,(Addr + Count*4),(Addr + Count*4));
			return XST_FAILURE;
		}
	}

	return XST_SUCCESS;
}


#ifdef XPAR_UARTNS550_0_BASEADDR
/*****************************************************************************/
/*
*
* Uart16550 setup routine, need to set baudrate to 9600 and data bits to 8
*
* @param        None
*
* @return       None
*
* @note         None.
*
******************************************************************************/
static void Uart550_Setup(void)
{

	XUartNs550_SetBaud(XPAR_UARTNS550_0_BASEADDR,
			XPAR_XUARTNS550_CLOCK_HZ, 9600);

	XUartNs550_SetLineControlReg(XPAR_UARTNS550_0_BASEADDR,
			XUN_LCR_8_DATA_BITS);
}
#endif

