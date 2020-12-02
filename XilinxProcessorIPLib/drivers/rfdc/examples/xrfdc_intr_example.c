/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
* For zcu111 board users are expected to define XPS_BOARD_ZCU111 macro
* while compiling this example.
*
* This example is design specific, PL-PS Interrupts must be attached and
* The Stimulus/Capture Block device names/addresses may vary.
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
* 2.2   sk     10/18/17 Check for FIFO intr to return success.
* 4.0   sd     04/28/18 Add Clock configuration support for ZCU111.
*       sd     05/15/18 Updated Clock configuration for lmk.
* 5.0   sk     08/03/18 For baremetal, add metal device structure for rfdc
*                       device and register the device to libmetal generic bus.
*       mus    08/18/18 Updated to remove xparameters.h dependency for linux
*                       platform.
* 6.0   cog    02/06/19 Updated for libmetal v2.0 and added configure PLL to
*                       set clock to incompatible rate
*       cog    06/08/19 Linux platform compatibility fixes.
* 7.0   cog    07/25/19 Updated example for new metal register API.
* 7.1   cog    01/24/20 Updated example for Gen3 and libmetal 2.0.
* 8.0   cog    04/09/20 Fixed baremetal compilation bug.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
#endif
#include "xrfdc.h"
#include <metal/irq.h>
#ifdef XPS_BOARD_ZCU111
#include "xrfdc_clk.h"
#endif

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifdef __BAREMETAL__
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#define RFDC_IRQ_VECT_ID                XPS_FPGA0_INT_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_0_DEVICE_ID
#define XRFDC_BASE_ADDR		XPAR_XRFDC_0_BASEADDR
#define STIM_BASE_ADDR	XPAR_STIMULUS_GEN_AXI_S_0_BASEADDR
#define CAP_BASE_ADDR	XPAR_DATA_CAPTURE_AXI_S_0_BASEADDR
#define BUS_NAME        "generic"
#define RFDC_DEV_NAME    XPAR_XRFDC_0_DEV_NAME
#define I2CBUS	1
#else
#define RFDC_DEVICE_ID  0
#define BUS_NAME        "platform"
#define I2CBUS	12
#endif

#define STIM_DEV_NAME    "a0000000.exdes_rfdac_data_bram_stim"
#define CAP_DEV_NAME    "a0400000.exdes_rfadc_data_bram_capture"

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
int register_metal_device(void);
#endif
int sys_init();
void RFdcHandler(void *CallBackRef, u32 Type, int Tile_Id,
						u32 Block_Id, u32 StatusEvent);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */
volatile int InterruptOccured;
#ifdef XPS_BOARD_ZCU111
unsigned int LMK04208_CKin[1][26] = {
		{0x00160040,0x80140320,0x80140321,0x80140322,
		0xC0140023,0x40140024,0x80141E05,0x03300006,0x01300007,0x06010008,
		0x55555549,0x9102410A,0x0401100B,0x1B0C006C,0x2302886D,0x0200000E,
		0x8000800F,0xC1550410,0x00000058,0x02C9C419,0x8FA8001A,0x10001E1B,
		0x0021201C,0x0180033D,0x0200033E,0x003F001F }};
#endif

struct metal_device *deviceptr;
struct metal_device *device_stim;
struct metal_io_region *io_stim;
struct metal_device *device_cap;
struct metal_io_region *io_cap;

#ifdef __BAREMETAL__
XScuGic InterruptController;

const metal_phys_addr_t metal_phys[] = {
		XRFDC_BASE_ADDR,
		STIM_BASE_ADDR,
		CAP_BASE_ADDR
};

static struct metal_device metal_dev_rfdc = {
	/* RFdc device */
	.name = RFDC_DEV_NAME,
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)XRFDC_BASE_ADDR,
			.physmap = &metal_phys[0],
			.size = 0x40000,
			.page_shift = (unsigned)(-1),
			.page_mask = (unsigned)(-1),
			.mem_flags = 0x0,
			.ops = {NULL},
		}
	},
	.node = {NULL},
	.irq_num = 1,
	.irq_info = (void *)RFDC_IRQ_VECT_ID,
};

static struct metal_device metal_dev_stim = {
	/* Stimulus device */
	.name = STIM_DEV_NAME,
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)STIM_BASE_ADDR,
			.physmap = &metal_phys[1],
			.size = 0x10000,
			.page_shift = (unsigned)(-1),
			.page_mask = (unsigned)(-1),
			.mem_flags = 0x0,
			.ops = {NULL},
		}
	},
	.node = {NULL},
	.irq_num = 0,
	.irq_info = NULL,
};

static struct metal_device metal_dev_cap = {
	/* Capture device */
	.name = CAP_DEV_NAME,
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)CAP_BASE_ADDR,
			.physmap = &metal_phys[2],
			.size = 0x10000,
			.page_shift = (unsigned)(-1),
			.page_mask = (unsigned)(-1),
			.mem_flags = 0x0,
			.ops = {NULL},
		}
	},
	.node = {NULL},
	.irq_num = 0,
	.irq_info = NULL,
};

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
	int ret = 0;
	InterruptOccured = 0;
#ifndef __BAREMETAL__
	struct metal_device metal_dev_stim;
	struct metal_device metal_dev_cap;
	int irq;
#else
	int irq = RFDC_IRQ_VECT_ID;
#endif

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;

	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\r\n");
		return XRFDC_FAILURE;
	}

	/* Initialize the RFdc driver. */
	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}
#ifdef __BAREMETAL__
	deviceptr = &metal_dev_rfdc;
#endif
	Status = XRFdc_RegisterMetal(RFdcInstPtr, RFdcDeviceId, &deviceptr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

#ifdef XPS_BOARD_ZCU111
printf("\n Configuring the Clock \r\n");
	LMK04208ClockConfig(I2CBUS, LMK04208_CKin);
	LMX2594ClockConfig(I2CBUS, 3932160);
#endif

	/*
	 * Setup the handler for the RFdc that will be called from the
	 * interrupt context when an RFdc interrupt occurs, specify a pointer to
	 * the RFdc driver instance as the callback reference so the handler is
	 * able to access the instance data
	 */
	XRFdc_SetStatusHandler(RFdcInstPtr, RFdcInstPtr,
				 (XRFdc_StatusHandler) RFdcHandler);

	Status = XRFdc_IntrEnable(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block,
						XRFDC_IXR_FIFOUSRDAT_MASK);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

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

	/* Get interrupt ID from RFDC metal device */
	irq = (intptr_t)RFdcInstPtr->device->irq_info;
	if (irq < 0) {
		printf("ERROR: Failed to request interrupt for %s.\r\n",
			  RFdcInstPtr->device->name);
		return XRFDC_FAILURE;
	}

#ifdef __BAREMETAL__
	ret = metal_xlnx_irq_init();
	if (ret) {
		printf("\n failed to initialise interrupt handler \r\n");
	}

   ret = metal_register_generic_device(&metal_dev_stim);
   if (ret) {
      printf("\n failed to register stim block \r\n");
   } else {
      printf("registered stim block.\r\n");
   }

   ret = metal_register_generic_device(&metal_dev_cap);
   if (ret) {
      printf("\n failed to register cap block \r\n");
   } else {
      printf("registered cap block.\r\n");
   }
#endif
	ret =  metal_irq_register(irq,
				(metal_irq_handler)XRFdc_IntrHandler,
						RFdcInstPtr);
	if (ret) {
		printf("\n failed to register interrupt handler \r\n");
	} else {
		printf("registered IPI interrupt.\r\n");
	}

	metal_irq_enable(irq);

	device_stim = &metal_dev_stim;
	ret = metal_device_open(BUS_NAME, STIM_DEV_NAME, &device_stim);
	if (ret) {
		printf("ERROR: Failed to open device stimulus_gen_axi_s.\r\n");
		return XRFDC_FAILURE;
	}

	/* Map Stimulus device IO region */
	io_stim = metal_device_io_region(device_stim, 0);
	if (!io_stim) {
		printf("ERROR: Failed to map Stimulus regio for %s.\r\n",
			  device_stim->name);
		return XRFDC_FAILURE;
	}

	device_cap = &metal_dev_cap;
	ret = metal_device_open(BUS_NAME, CAP_DEV_NAME, &device_cap);
	if (ret) {
		printf("ERROR: Failed to open device data_capture_axi_s.\r\n");
		return XRFDC_FAILURE;
	}

	/* Map Data Capture device IO region */
	io_cap = metal_device_io_region(device_cap, 0);
	if (!io_cap) {
		printf("ERROR: Failed to map Capture regio for %s.\r\n",
			  device_cap->name);
		return XRFDC_FAILURE;
	}

	/*
	 * Below writes are not generic, they are design specific.
	 * This may not be expected to work as it is with user specific design.
	 * These register writes are related to Stimulus and Capture blocks
	 * which will be used to generate some random data and Capture the
	 * output data respectively.
	 * Following register writes will be used for Configuring
	 * to start FIFO on DAC/Stim gen and ADC/Capture
	 */
	XRFdc_Out32(io_stim, 0x50, 0x8000);
	XRFdc_Out32(io_cap, 0x50, 0x8000);

#ifdef __BAREMETAL__
	if (init_irq()) {
		xil_printf("Failed to initialize interrupt\r\n");
	}
#endif

	printf("Waiting for Interrupt\r\n");
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
			   (Xil_ExceptionHandler)metal_xlnx_irq_isr,
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
		if ((StatusEvent & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U)
			InterruptOccured = 1;
	} else {
		printf("\n %x Interrupt occurred for DAC%d%d \r\n",
								StatusEvent, Tile_Id, Block_Id);
		if ((StatusEvent & XRFDC_IXR_FIFOUSRDAT_MASK) != 0U)
			InterruptOccured = 1;
	}

	/* Disable the interrupt */
	XRFdc_IntrDisable(&RFdcInst, Type, Tile_Id, Block_Id, StatusEvent);
}
