/******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file xrfdc_gen3_clocked_example.c
*
* This file contains an example for Gen 3 using the rfdc hardware and
* RFSoC Data Converter driver.
* This example does some writes to the hardware to do some sanity checks
* and does a reset to restore the original settings.
*
* Users are expected to have a programmed CLK-104 module if using this
* example for ZCU208/216.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 8.1   cog    06/29/20 First release.
*       cog    07/03/20 The metal_phys parameter is baremetal only.
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

	printf("RFdc Gen 3 Clocked Example\r\n");
	/*
	 * Run the RFdc fabric rate example, specify the Device ID that is
	 * generated in xparameters.h.
	 */

	Status = ClockedExample(RFDC_DEVICE_ID);
	if (Status != XRFDC_SUCCESS) {
		printf("RFdc Gen 3 Clocked failed\r\n");
		return XRFDC_FAILURE;
	}

	printf("Successfully ran RFdc Gen 3 Clocked Example\r\n");
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
	XRFdc_Distribution_Settings Distribution_Settings;
	u32 ADCClockedTile;
	u32 DACClockedTile;
	u32 ADCSource;
	u32 DACSource;

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
	if(RFdcInstPtr->RFdc_Config.IPType < XRFDC_GEN3){
		printf("ERROR: Running a Gen 3 example on a Gen 1/2 board\n");
		return XRFDC_FAILURE;
	}

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
			if (RFdcInstPtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
				if ((Block == 2) || (Block == 3))
					continue;
				else if (Block == 1) {
					if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, 2) == 0)
						continue;
				}
			}
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
		/*Reset IP - the clock distribution will bring the tiles up again*/
		XRFdc_WriteReg16(RFdcInstPtr, 0x0, 0x4, 1);

		printf("\n Configuring the Clock \r\n");
		/* This takes a reference clock of 245.76 MHz into ADC1/DAC1 uses PLL to increase to 2211.84 GHz
		   and distrubutes the ful rate clock to all other ADC/DAC tiles.*/
		memset(&Distribution_Settings, 0, sizeof(Distribution_Settings));
		if(XRFdc_IsHighSpeedADC(RFdcInstPtr, Tile) == XRFDC_ENABLED) { /*ZCU208*/
			DACClockedTile = XRFDC_TILE_ID0;
			DACSource = XRFDC_CLK_DST_TILE_228;
		} else { /*ZCU216*/
			DACClockedTile = XRFDC_TILE_ID2;
			DACSource = XRFDC_CLK_DST_TILE_230;
		}
		ADCClockedTile = XRFDC_TILE_ID2;
		ADCSource = XRFDC_CLK_DST_TILE_226;

		for (int i =0;i<4;i++) {
			Distribution_Settings.DAC[i].SourceTile                   = DACSource;
			Distribution_Settings.ADC[i].SourceTile                   = ADCSource;
			Distribution_Settings.DAC[i].PLLSettings.SampleRate       = 2211.84;
			Distribution_Settings.ADC[i].PLLSettings.SampleRate       = 2211.84;
			if(i == DACClockedTile){
				Distribution_Settings.DAC[i].PLLEnable                    = 1;
				Distribution_Settings.DAC[i].PLLSettings.RefClkFreq       = 245.76;
				Distribution_Settings.DAC[i].DistributedClock             = XRFDC_DIST_OUT_OUTDIV;
			} else {
				Distribution_Settings.DAC[i].PLLEnable                    = 0;
				Distribution_Settings.DAC[i].PLLSettings.RefClkFreq       = 2211.84;
				Distribution_Settings.DAC[i].DistributedClock             = XRFDC_DIST_OUT_NONE;
			}
			if(i == ADCClockedTile){
				Distribution_Settings.ADC[i].PLLEnable                    = 1;
				Distribution_Settings.ADC[i].PLLSettings.RefClkFreq       = 245.76;
				Distribution_Settings.ADC[i].DistributedClock             = XRFDC_DIST_OUT_OUTDIV;
			} else {
				Distribution_Settings.ADC[i].PLLEnable                    = 0;
				Distribution_Settings.ADC[i].PLLSettings.RefClkFreq       = 2211.84;
				Distribution_Settings.ADC[i].DistributedClock             = XRFDC_DIST_OUT_NONE;
			}
		}
		Status = XRFdc_SetClkDistribution(RFdcInstPtr, &Distribution_Settings);
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
