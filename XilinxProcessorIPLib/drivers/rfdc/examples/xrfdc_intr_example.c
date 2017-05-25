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
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xrfdc.h"
#include "xstatus.h"
#include "xil_cache.h"
#include <metal/irq.h>

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define RFDC_IRQ_VECT_ID		XPS_FPGA0_INT_ID

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

/************************** Function Prototypes *****************************/

static int RFdcFabricRateExample(u16 SysMonDeviceId);
static int CompareFabricRate(u32 SetDecoderMode, u32 GetDecoderMode);
int init_irq();
int sys_init();
void RFdcHandler(void *CallBackRef, u32 Type, int Tile_Id,
						u32 Block_Id, u32 StatusEvent);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */
XScuGic InterruptController;

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

	xil_printf("RFdc Fabric Interrupt Example Test\r\n");
	/*
	 * Run the RFdc Fabric Rate Settings example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = RFdcFabricRateExample(RFDC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf(" RFdc Fabric Interrupt Example Test failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("\n Successfully ran RFdc Fabric Interrupt Example Test\r\n");
	return XST_SUCCESS;
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
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
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
	int ret;

	/* Initialize the RFdc driver. */
	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}
	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if (sys_init()) {
		xil_printf("ERROR: Failed to initialize system\n");
		return -1;
	}
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
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block,
					&GetFabricRate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	Status = CompareFabricRate(SetFabricRate, GetFabricRate);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	ret =  metal_irq_register(RFDC_IRQ_VECT_ID,
				(metal_irq_handler)XRFdc_IntrHandler, NULL, RFdcInstPtr);
	xil_printf("registered IPI interrupt.\n");
	if (ret) {
		xil_printf("\n failed to register interrupt handler \r\n");
	}

	/* Configurations to start FIFO on DAC/Stim gen and ADC/Capture */
	Xil_Out32(0xA0400050, 0x8000);
	Xil_Out32(0xA0000050, 0x8000);

	return XST_SUCCESS;
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

/**
 * @brief sys_init() - Register libmetal devices.
 *        This function register the libmetal generic bus, and then
 *        register the IPI, shared memory descriptor and shared memory
 *        devices to the libmetal generic bus.
 *
 * @return 0 - succeeded, non-zero for failures.
 */
int sys_init()
{
	struct metal_init_params metal_param = METAL_INIT_DEFAULTS;

	if (init_irq()) {
		xil_printf("Failed to initialize interrupt\n");
	}
	/** Register the device */
	metal_init(&metal_param);


	return 0;
}

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
		xil_printf("\n %x Interrupt occurred for ADC%d%d \r\n",
						StatusEvent, Tile_Id, Block_Id);
	} else {
		xil_printf("\n %x Interrupt occurred for DAC%d%d \r\n",
								StatusEvent, Tile_Id, Block_Id);
	}
}
