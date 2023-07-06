/******************************************************************************
* Copyright (C) 2013 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xtrafgen_master_streaming_example.c
 *
 * This file demonstrates how to use the xtrafgen driver on the Xilinx AXI
 * Traffic Generator core. The AXI Traffic Generator IP is designed to 
 * generate AXI4 traffic which can be used to stress different modules/
 * interconnect connected in the system. 
 *
 * This example demonstrates how to use Streaming mode in Axi Traffic Genrator
 * When Configured in Master only mode.In this mode the core generates Streaming 
 * Traffic based on the transfer length and transfer count configured.
 * To test this example hardware Must contain a Streaming FIFO and the 
 * Connections To Axi TrafficGen needs to made As shown below
 *                                    ________________
 *	 ____________       _________|AXI_STR_RXD     |
 *	|	     |     |         |                |
 *	|            |_____|         |                |
 *	|  axi_master|               |________________|
 *      |____________|                
 *
 *       Axi TrafficGen                 Axi Stream FIFO 
 *
 *<pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who  Date     Changes
 * ----- ---- -------- -------------------------------------------------------
 * 1.01a adk  03/09/13 First release
 * 2.00a adk  16/09/13 Fixed CR:737291
 * 2.01a adk  15/11/13 Fixed CR:760808
 * 4.1   ms   01/23/17 Modified xil_printf statement in main function to
 *                     ensure that "Successfully ran" and "Failed" strings
 *                     are available in all examples. This is a fix for
 *                     CR-965028.
 *       ms   04/05/17 Added tabspace for return statements in functions for
 *                     proper documentation while generating doxygen and also
 *                     modified filename tag to include the file in doxygen
 *                     examples.
 * </pre>
 *
 * ***************************************************************************
 */

/***************************** Include Files *********************************/
#include "xtrafgen.h"
#include "xparameters.h"
#include "xil_exception.h"

#ifdef XPAR_AXI_FIFO_0_BASEADDR
#include "xllfifo.h"
#endif

#ifdef XPAR_UARTNS550_0_BASEADDR
#include "xuartns550_l.h"       /* to use uartns550 */
#endif

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifndef SDT
#define TRAFGEN_DEV_ID	XPAR_AXI_TRAFFIC_GEN_2_DEVICE_ID
#endif

#ifdef XPAR_V6DDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_V6DDR_0_S_AXI_BASEADDR
#elif XPAR_S6DDR_0_S0_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_S6DDR_0_S0_AXI_BASEADDR
#elif XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#define DDR_BASE_ADDR	XPAR_AXI_7SDDR_0_S_AXI_BASEADDR
#elif XPAR_MIG7SERIES_0_BASEADDR
#define DDR_BASE_ADDR XPAR_MIG_1_BASEADDR
#elif XPAR_MIG_0_BASEADDRESS
#define DDR_BASE_ADDR XPAR_MIG_0_BASEADDRESS
#endif

#ifndef DDR_BASE_ADDR
#warning CHECK FOR THE VALID DDR ADDRESS IN XPARAMETERS.H, \
                        DEFAULT SET TO 0x01000000
#define MEM_BASE_ADDR	0x01000000
#else
#define MEM_BASE_ADDR	(DDR_BASE_ADDR + 0x1000000)
#endif

#define BURST_LEN	255
#define STR_FIFO0_ADDR   XPAR_AXI_FIFO_0_BASEADDR
#undef DEBUG

/************************** Function Prototypes ******************************/
#ifndef SDT
int XTrafGenStremingModeMasterExample(XTrafGen *InstancePtr, u16 DeviceId);
#else
int XTrafGenStremingModeMasterExample(XTrafGen *InstancePtr, UINTPTR BaseAddress);
#endif
#ifdef XPAR_UARTNS550_0_BASEADDR
static void Uart550_Setup(void);
#endif

/************************** Variable Definitions *****************************/
/*
 * Device instance definitions
 */
XTrafGen XTrafGenInstance;
XLlFifo  XLlFifoInstance;


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

#ifndef SDT
	Status = XTrafGenStremingModeMasterExample(&XTrafGenInstance, 
				TRAFGEN_DEV_ID);
#else
	Status = XTrafGenStremingModeMasterExample(&XTrafGenInstance,
				XPAR_XTRAFGEN_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Traffic Gen Streaming Example Test Failed\n\r");
		xil_printf("--- Exiting main() ---\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Traffic Gen Streaming Example\n\r");
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
*       - Initialize the Streaming FIFO device
*	- Set the Desired Transfer Count and Transfer Length
*	- Enable the Traffic Generation on the Core
*	- Check for the Streaming data on the FIFO 
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
*			-XST_SUCCESS to indicate success
*			-XST_FAILURE to indicate failure
*
******************************************************************************/
#ifndef SDT
int XTrafGenStremingModeMasterExample(XTrafGen *InstancePtr, u16 DeviceId)
#else
int XTrafGenStremingModeMasterExample(XTrafGen *InstancePtr, UINTPTR BaseAddress)
#endif
{

	XTrafGen_Config *Config;
	int Status = XST_SUCCESS;
	u32 Len; 
	u32 TransferCnt;
	u32 AtgPacket;
	u32 FifoOcy;
	u32 FifoLen;
	
        /* Initial setup for Uart16550 */
#ifdef XPAR_UARTNS550_0_BASEADDR

	Uart550_Setup();

#endif

	/* Initialize the Device Configuration Interface driver */
#ifndef SDT
	Config = XTrafGen_LookupConfig(DeviceId);
#else
	Config = XTrafGen_LookupConfig(BaseAddress);
#endif
	if (!Config) {
#ifndef SDT
		xil_printf("No config found for %d\r\n", DeviceId);
#endif
		return XST_FAILURE;
	}

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XTrafGen_CfgInitialize(InstancePtr, Config,
				Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Initialization failed\n\r");
		return Status;
	}
	
	/* Check for the Streaming Mode */
	if(InstancePtr->OperatingMode != XTG_MODE_STREAMING) {
		return XST_FAILURE;
	}
	
	/* Initialize the Fifo Instance */
	XLlFifo_Initialize(&XLlFifoInstance , STR_FIFO0_ADDR);
	Status = XLlFifo_Status(&XLlFifoInstance);
	XLlFifo_IntClear(&XLlFifoInstance,0xffffffff);
	Status = XLlFifo_Status(&XLlFifoInstance);
	if(Status != 0x0) {
		xil_printf("\n ERROR : Reset value of ISR0 : 0x%x\t"
			    "Expected : 0x0\n\r",
			    XLlFifo_Status(&XLlFifoInstance));
		return XST_FAILURE;
	}
	
	/* 
	 * Set the Required transaction length 
	 * and required transaction count
	 */
	XTrafGen_ResetStreamingRandomLen(InstancePtr);
	XTrafGen_SetStreamingTransLen(InstancePtr , 3);
	XTrafGen_SetStreamingTransCnt(InstancePtr , 2);
	
	Len = XTrafGen_GetStreamingTransLen(InstancePtr);
	TransferCnt = XTrafGen_GetStreamingTransCnt(InstancePtr);

	/* 
	 * Calculate the ATG data that is sent on the 
	 * CORE when Streaming is Enabled 
	 */
	AtgPacket = (Len +1) * TransferCnt;
		
	/* Enable the traffic generation */
	XTrafGen_StreamEnable(InstancePtr);
	
	FifoOcy = XLlFifo_iRxOccupancy(&XLlFifoInstance);
	if(FifoOcy != AtgPacket) {
		xil_printf("\n ERROR : Not received complete packets : 0x%x \t"
			"Expected : 0x%x \n\r",
			XLlFifo_iRxOccupancy(&XLlFifoInstance), AtgPacket);
		return XST_FAILURE;
	}
	
	FifoLen = XLlFifo_iRxGetLen(&XLlFifoInstance);
	if(FifoLen != (AtgPacket*4/TransferCnt)) {
		xil_printf("\n ERROR : Not received complete bytes : 0x%x \t"
			"Expected : 0x%x \n\n\r",
			XLlFifo_iRxGetLen(&XLlFifoInstance),Len);
		return XST_FAILURE;
	}
	while(XLlFifo_iRxGetLen(&XLlFifoInstance)) {
		xil_printf("Received packet DATA: 0x%x \n\r",
				XLlFifo_RxGetWord(&XLlFifoInstance));
	}
	
	if(XLlFifo_iRxOccupancy(&XLlFifoInstance) != 0) {
		xil_printf("\n ERROR : RDFO is not becoming Empty : 0x%x \t"
				"Expected : 0x0 \n\n\r",
				XLlFifo_iRxOccupancy(&XLlFifoInstance));
		return XST_FAILURE;
	}
	
	if(XLlFifo_iRxGetLen(&XLlFifoInstance) != 0) {
		xil_printf("\n ERROR : RLR is not becoming Empty : 0x%x \t"
				"Expected : 0x0 \n\n\r",
				XLlFifo_iRxGetLen(&XLlFifoInstance));
		return XST_FAILURE;
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

