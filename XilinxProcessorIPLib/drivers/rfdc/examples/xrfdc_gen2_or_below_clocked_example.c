/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xrfdc_gen2_or_below_clocked_example.c
*
* This file contains an example for Gen 1/2 using the rfdc hardware and
* RFSoC Data Converter driver.
* This example does some writes to the hardware to do some sanity checks
* and does a reset to restore the original settings.
*
* Users are expected to define XPS_BOARD_ZCU111 macro while compiling
* this example for ZCU111.
*
* Users are expected to have a programmed CLK-103 module if using this
* example for ZCU1275/1285.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 8.1   cog    06/29/20 First release.
*       cog    07/03/20 The metal_phys parameter is baremetal only.
*       cog    07/03/20 The change clock to fit with new reference design.
*       cog    07/03/20 Formatting and text changes.
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#ifdef __BAREMETAL__
#include "xparameters.h"
#endif
#include "xrfdc.h"
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
#define I2CBUS	1
#define XRFDC_BASE_ADDR		XPAR_XRFDC_0_BASEADDR
#define RFDC_DEV_NAME    XPAR_XRFDC_0_DEV_NAME
#else
#define RFDC_DEVICE_ID 	0
#define I2CBUS	12
#endif

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/

static int ClockedExample(u16 SysMonDeviceId);
static int CompareFabricRate(u32 SetFabricRate, u32 GetFabricRate);

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */
struct metal_device *deviceptr = NULL;

#ifdef XPS_BOARD_ZCU111
unsigned int LMK04208_CKin[1][26] = {
		{0x00160040,0x80140320,0x80140321,0x80140322,
		0xC0140023,0x40140024,0x80141E05,0x03300006,0x01300007,0x06010008,
		0x55555549,0x9102410A,0x0401100B,0x1B0C006C,0x2302886D,0x0200000E,
		0x8000800F,0xC1550410,0x00000058,0x02C9C419,0x8FA8001A,0x10001E1B,
		0x0021201C,0x0180033D,0x0200033E,0x003F001F }};
#endif

#ifdef __BAREMETAL__
metal_phys_addr_t metal_phys = XRFDC_BASE_ADDR;
static struct metal_device CustomDev = {
	/* RFdc device */
	.name = RFDC_DEV_NAME,
	.bus = NULL,
	.num_regions = 1,
	.regions = {
		{
			.virt = (void *)XRFDC_BASE_ADDR,
			.physmap = &metal_phys,
			.size = 0x40000,
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

	printf("RFdc Gen 1/2 Clocked Example Test\r\n");
	/*
	 * Run the RFdc fabric rate example, specify the Device ID that is
	 * generated in xparameters.h.
	 */

	Status = ClockedExample(RFDC_DEVICE_ID);
	if (Status != XRFDC_SUCCESS) {
		printf("RFdc Gen 1/2 Clocked Example failed\r\n");
		return XRFDC_FAILURE;
	}

	printf("Successfully ran RFdc Gen 1/2 Clocked Example\r\n");
	return XRFDC_SUCCESS;
}

/****************************************************************************/
/**
*
* This function runs a test on the RFSoC data converter device using the
* driver APIs.
* This function does the following tasks:
*	- Initialize the RFdc device driver instance
*	- Set the fabric width.
*	- Get the fabric width
*	- Compare Set and Get fabric widths.
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
int ClockedExample(u16 RFdcDeviceId)
{

	int Status;
	u16 Tile;
	u16 Block;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;
	u32 ADCSetFabricRate[4];
	u32 DACSetFabricRate[4];
	u32 GetFabricRate;

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

	/* Register & MAP RFDC to Libmetal */
#ifdef __BAREMETAL__
	deviceptr = &CustomDev;
#endif
	Status = XRFdc_RegisterMetal(RFdcInstPtr, RFdcDeviceId, &deviceptr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	/* Initializes the controller */
	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	if(RFdcInstPtr->RFdc_Config.IPType >= XRFDC_GEN3){
		printf("ERROR: Running a Gen 1/2 example on a higher Gen board\n");
		return XRFDC_FAILURE;
	}

#ifdef XPS_BOARD_ZCU111
printf("\n Configuring the Clock \r\n");
	LMK04208ClockConfig(I2CBUS, LMK04208_CKin);
	LMX2594ClockConfig(I2CBUS, 245760);
#endif
	Tile = 0x0;
	for (Block = 0; Block <4; Block++) {
		if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
			Status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE,
							Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if(GetFabricRate > 0) {
				DACSetFabricRate[Block] = GetFabricRate - 1;
			}
			else {
				DACSetFabricRate[Block] = GetFabricRate + 1;
			}
			Status = XRFdc_SetFabWrVldWords(RFdcInstPtr, Tile, Block, DACSetFabricRate[Block]);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			Status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE,
							Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			Status = CompareFabricRate(DACSetFabricRate[Block], GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
		}
		if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
			Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE,
									Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if(GetFabricRate > 0) {
				ADCSetFabricRate[Block] = GetFabricRate - 1;
			} else {
				ADCSetFabricRate[Block] = GetFabricRate + 1;
			}
			Status = XRFdc_SetFabRdVldWords(RFdcInstPtr, Tile, Block, ADCSetFabricRate[Block]);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE,
									Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			Status = CompareFabricRate(ADCSetFabricRate[Block], GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
		}
	}

	Status = XRFdc_Reset(RFdcInstPtr, XRFDC_ADC_TILE, Tile);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	Status = XRFdc_Reset(RFdcInstPtr, XRFDC_DAC_TILE, Tile);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	for (Block = 0; Block <4; Block++) {
		if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
			Status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE,
							Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if (GetFabricRate == DACSetFabricRate[Block]) {
				return XRFDC_FAILURE;
			}
		}
		if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
			Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE,
									Tile, Block, &GetFabricRate);
			if (Status != XRFDC_SUCCESS) {
				return XRFDC_FAILURE;
			}
			if (GetFabricRate == ADCSetFabricRate[Block]) {
				return XRFDC_FAILURE;
			}
		}
	}

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
