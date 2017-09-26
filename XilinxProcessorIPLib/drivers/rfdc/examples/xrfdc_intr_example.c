/******************************************************************************
*
* Copyright (C) 2017 Xilinx, Inc.  All rights reserved.
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
/****************************************************************************/
/**
*
* @file xrfdc_intr_example.c
*
* For the RFSoC Data Converter, the interrupts are mostly used for error
* reporting.
* The interrupts do not do any data processing. Since they dont do any data
* processing, interrupts are invoked in rare conditions.
* The example here attempts to demonstrate users how an error interrupt can be
* generated. Also once generated how does the processing happen.
* Upon an interrupt, the control reaches to ScuGIC interrupt handler.
* From there the control is transferred to the libmetal isr handling which
* then calls the driver interrupt handler. Users are expected to register
* their callbacks with the driver interrupt framework.
* The actual interrupt handling is expected to happen in the user provided
* callback.
*
* This example generates ADC fabric interrupts by writing some incorrect
* fabric data rate based on the read/write clocks.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sk     05/25/17 First release
* 1.1   sk     08/09/17 Modified the example to support both Linux and
*                       Baremetal.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xrfdc.h"
#include <metal/irq.h>

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#define RFDC_IRQ_VECT_ID                XPS_FPGA0_INT_ID
#ifdef __BAREMETAL__
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#else
#define BUS_NAME        "platform"
#define RFDC_DEV_NAME    "a0000000.usp_rf_data_converter"
#define STIM_DEV_NAME    "a8000000.stimulus_gen_axi_s"
#define CAP_DEV_NAME    "a4000000.data_capture_axi_s"
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/

static int RFdcFabricRateExample(u16 SysMonDeviceId);
static int CompareFabricRate(u32 SetDecoderMode, u32 GetDecoderMode);
#ifdef __BAREMETAL__
int init_irq();
#endif
int sys_init();
void RFdcHandler(void *CallBackRef, u32 Type, int Tile_Id,
						u32 Block_Id, u32 StatusEvent);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */
volatile int InterruptOccured;
#ifdef __BAREMETAL__
XScuGic InterruptController;
#endif

/****************************************************************************/
/**
*
* Main function that invokes the polled example in this file.
*
* @param	None.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note		None.
*
*****************************************************************************/
int main(void)
{

	int Status;

	printf("RFdc Fabric Interrupt Example Test\r\n");
	/*
	 * Run the RFdc Fabric Rate Settings example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = RFdcFabricRateExample(RFDC_DEVICE_ID);
	if (Status != XRFDC_SUCCESS) {
		printf(" RFdc Fabric Interrupt Example Test failed\r\n");
		return XRFDC_FAILURE;
	}

	printf("\n Successfully ran RFdc Fabric Interrupt Example Test\r\n");
	return XRFDC_SUCCESS;
}


/****************************************************************************/
/**
*
* This function runs a test on the RFSoC data converter device using the
* driver APIs.
* This function does the following tasks:
*	- Initialize the RFdc device driver instance
*	- Set the Fabric Rate.
*	- Get the Fabric Rate.
*	- Compare Set and Get Rates
*
* @param	RFdcDeviceId is the XPAR_<XRFDC_instance>_DEVICE_ID value
*		from xparameters.h.
*
* @return
*		- XRFDC_SUCCESS if the example has completed successfully.
*		- XRFDC_FAILURE if the example has failed.
*
* @note   	None
*
****************************************************************************/
int RFdcFabricRateExample(u16 RFdcDeviceId)
{


	int Status;
	u16 Tile = 0U;
	u16 Block = 0U;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;
	u32 SetFabricRate;
	u32 GetFabricRate;
#ifndef __BAREMETAL__
	struct metal_device *device;
	struct metal_io_region *io;
	struct metal_device *device_stim;
	struct metal_io_region *io_stim;
	struct metal_device *device_cap;
	struct metal_io_region *io_cap;
#endif
	int irq = RFDC_IRQ_VECT_ID;
	int ret = 0;

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XRFDC_FAILURE;
	}

	/* Initialize the RFdc driver. */
	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}
	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
#ifndef __BAREMETAL__
	ret = metal_device_open(BUS_NAME, RFDC_DEV_NAME, &device);
	if (ret) {
		printf("ERROR: Failed to open device a0000000.usp_rf_data_converter.\n");
		return XRFDC_FAILURE;
	}

	/* Map RFDC device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		printf("ERROR: Failed to map RFDC regio for %s.\n",
			  device->name);
		return XRFDC_FAILURE;
	}
	RFdcInstPtr->device = device;
	RFdcInstPtr->io = io;
#endif

	/*
	 * Setup the handler for the RFdc that will be called from the
	 * interrupt context when an RFdc interrupt occurs, specify a pointer to
	 * the RFdc driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XRFdc_SetStatusHandler(RFdcInstPtr, RFdcInstPtr,
				 (XRFdc_StatusHandler) RFdcHandler);

	XRFdc_IntrEnable(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block,
						XRFDC_IXR_FIFOUSRDAT_MASK);
	GetFabricRate = 0;
	SetFabricRate = 0x1;
	Status = XRFdc_SetFabRdVldWords(RFdcInstPtr, Tile, Block,
					SetFabricRate);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block,
					&GetFabricRate);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	Status = CompareFabricRate(SetFabricRate, GetFabricRate);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

#ifndef __BAREMETAL__
	/* Get interrupt ID from RFDC metal device */
	irq = (intptr_t)RFdcInstPtr->device->irq_info;
	if (irq < 0) {
		printf("ERROR: Failed to request interrupt for %s.\n",
			  device->name);
		return XRFDC_FAILURE;
	}
#endif
	ret =  metal_irq_register(irq,
				(metal_irq_handler)XRFdc_IntrHandler, RFdcInstPtr->device,
						RFdcInstPtr);
	printf("registered IPI interrupt.\n");
	if (ret) {
		printf("\n failed to register interrupt handler \r\n");
	}

#ifndef __BAREMETAL__
	ret = metal_device_open(BUS_NAME, STIM_DEV_NAME, &device_stim);
	if (ret) {
		printf("ERROR: Failed to open device a8000000.stimulus_gen_axi_s.\n");
		return XRFDC_FAILURE;
	}

	/* Map Stimulus device IO region */
	io_stim = metal_device_io_region(device_stim, 0);
	if (!io) {
		printf("ERROR: Failed to map Stimulus regio for %s.\n",
			  device_stim->name);
		return XRFDC_FAILURE;
	}

	ret = metal_device_open(BUS_NAME, CAP_DEV_NAME, &device_cap);
	if (ret) {
		printf("ERROR: Failed to open device a4000000.data_capture_axi_s.\n");
		return XRFDC_FAILURE;
	}

	/* Map Data Capture device IO region */
	io_cap = metal_device_io_region(device_cap, 0);
	if (!io) {
		printf("ERROR: Failed to map Capture regio for %s.\n",
			  device_cap->name);
		return XRFDC_FAILURE;
	}
#endif
	InterruptOccured = 0;

	/*
	 * Below writes are not generic, they are design specific.
	 * This may not be expected to work as it is with user specific design.
	 * These register writes are related to Stimulus and Capture blocks
	 * which will be used to generate some random data and Capture the
	 * output data respectively.
	 * Following register writes will be used for Configuring
	 * to start FIFO on DAC/Stim gen and ADC/Capture
	 */
#ifdef __BAREMETAL__
	Xil_Out32(0xA4000050, 0x8000);
	Xil_Out32(0xA8000050, 0x8000);
#else
	XRFdc_Out32(io_stim, 0x50, 0x8000);
	XRFdc_Out32(io_cap, 0x50, 0x8000);
#endif

#ifdef __BAREMETAL__
	if (init_irq()) {
		xil_printf("Failed to initialize interrupt\n");
	}
#endif

	/* Wait till interrupt occurs */
	while (InterruptOccured == 0);

	return XRFDC_SUCCESS;
}

/****************************************************************************/
/*
*
* This function compares the two Fabric Rate variables and return 0 if
* same and returns 1 if not same.
*
* @param	SetFabricRate Fabric Rate value set.
* @param	GetFabricRate Fabric Rate value get.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareFabricRate(u32 SetFabricRate, u32 GetFabricRate)
{
	if (SetFabricRate == GetFabricRate)
		return 0;
	else
		return 1;
}
#ifdef __BAREMETAL__
/**
 * @brief init_irq() - Initialize GIC and connect IPI interrupt
 *        This function will initialize the GIC and connect the IPI
 *        interrupt.
 *
 * @return 0 - succeeded, non-0 for failures
 */
int init_irq()
{
	int ret = 0;
	Xil_ExceptionDisable();

	XScuGic_Config *IntcConfig;	/* The configuration parameters of
					 * the interrupt controller */

	/*
	 * Initialize the interrupt controller driver
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return (int)XST_FAILURE;
	}

	ret = XScuGic_CfgInitialize(&InterruptController, IntcConfig,
				       IntcConfig->CpuBaseAddress);
	if (ret != XST_SUCCESS) {
		return (int)XST_FAILURE;
	}

	/*
	 * Register the interrupt handler to the hardware interrupt handling
	 * logic in the ARM processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
			(Xil_ExceptionHandler)XScuGic_InterruptHandler,
			&InterruptController);

	Xil_ExceptionEnable();
	/* Connect IPI Interrupt ID with libmetal ISR */
	XScuGic_Connect(&InterruptController, RFDC_IRQ_VECT_ID,
			   (Xil_ExceptionHandler)metal_irq_isr,
			   (void *)RFDC_IRQ_VECT_ID);

	XScuGic_Enable(&InterruptController, RFDC_IRQ_VECT_ID);

	return 0;
}
#endif

/*****************************************************************************/
/**
*
* Callback handler.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void RFdcHandler (void *CallBackRef, u32 Type, int Tile_Id,
						u32 Block_Id, u32 StatusEvent) {
	if (Type == XRFDC_ADC_TILE) {
		printf("\n %x Interrupt occurred for ADC%d%d \r\n",
						StatusEvent, Tile_Id, Block_Id);
		InterruptOccured = 1;
	} else {
		printf("\n %x Interrupt occurred for DAC%d%d \r\n",
								StatusEvent, Tile_Id, Block_Id);
		InterruptOccured = 1;
	}

	/* Disable the interrupt */
	XRFdc_IntrDisable(&RFdcInst, Type, Tile_Id, Block_Id, StatusEvent);
}
