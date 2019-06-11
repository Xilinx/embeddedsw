/******************************************************************************
*
* Copyright (C) 2017-2018 Xilinx, Inc.  All rights reserved.
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
* @file xrfdc_read_write_example.c
*
* This example uses multiple driver "set" APIs to configure the targeted
* AMS block.
* Subsequently it uses "get" APIs to read back the configurations to ensure
* that the desired configurations are applied.
*
* For DAC it sets the following configurations:
* MixerSettings, QMCSettings, Write Fabricrate, Decoder mode, Output Current
* and Coarse Delay.
*
* For ADC it sets the following configurations:
* MixerSettings, QMCSettings, Read Fabricrate and Threshold Settings.
* This example shows how to change the configurations for ADC
* and DAC using driver functions.
*
* NOTE: The purpose of the example is to show how to use the driver APIs.
* For real user scenarios this example will not be relevant.
*
* For zcu111 board users are expected to define XPS_BOARD_ZCU111 macro
* while compiling this example.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sk     05/15/17 First release
* 1.1   sk     08/09/17 Modified the example to support both Linux and
*                       Baremetal.
*       sk     08/23/17 Add Nyquist Zone test case.
*       sk     09/25/17 Add GetOutput Current test case.
* 2.4   sk     12/11/17 Add test case for DDC and DUC.
* 3.2   sk     03/01/18 Add test case for Multiband.
* 4.0   sd     04/28/18 Add Clock configuration support for ZCU111.
*       sd     05/15/18 Updated Clock configuration for lmk.
* 5.0   sk     07/20/18 Update mixer settings test cases to consider MixerType.
*       sk     08/03/18 For baremetal, add metal device structure for rfdc
*                       device and register the device to libmetal generic bus.
*       mus    08/18/18 Updated to remove xparameters.h dependency for linux
*                       platform.
*       sk     09/07/18 Modified phasecorrection factor as per  QMC Phase
*                       correction factor range in driver
* 5.1   cog    01/29/19 Fixed some comments.
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
#define BUS_NAME        "generic"
#define XRFDC_BASE_ADDR		XPAR_XRFDC_0_BASEADDR
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID
#define RFDC_DEV_NAME    XPAR_XRFDC_0_DEV_NAME
#else
#define BUS_NAME        "platform"
#define RFDC_DEVICE_ID 	0
#endif


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/
#ifdef __BAREMETAL__
#define printf xil_printf
#endif
/************************** Function Prototypes *****************************/

static int RFdcReadWriteExample(u16 SysMonDeviceId);
static int CompareMixerSettings(XRFdc_Mixer_Settings *SetMixerSettings,
								 XRFdc_Mixer_Settings *GetMixerSettings);
static int CompareQMCSettings(XRFdc_QMC_Settings *SetQMCSettings,
								 XRFdc_QMC_Settings *GetQMCSettings);
static int CompareCoarseDelaySettings(XRFdc_CoarseDelay_Settings *SetCoarseDlySettings,
								 XRFdc_CoarseDelay_Settings *GetCoarseDlySettings);
static int CompareThresholdSettings(XRFdc_Threshold_Settings *SetThresholdSettings,
								 XRFdc_Threshold_Settings *GetThresholdSettings);
#ifdef __BAREMETAL__
int register_metal_device(void);
#endif

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */
struct metal_device *device;
struct metal_io_region *io;

#ifdef XPS_BOARD_ZCU111
unsigned int LMK04208_CKin[1][26] = {
		{0x00160040,0x80140320,0x80140321,0x80140322,
		0xC0140023,0x40140024,0x80141E05,0x03300006,0x01300007,0x06010008,
		0x55555549,0x9102410A,0x0401100B,0x1B0C006C,0x2302886D,0x0200000E,
		0x8000800F,0xC1550410,0x00000058,0x02C9C419,0x8FA8001A,0x10001E1B,
		0x0021201C,0x0180033D,0x0200033E,0x003F001F }};
#endif

#ifdef __BAREMETAL__
const metal_phys_addr_t metal_phys[] = {
		XRFDC_BASE_ADDR
};

static struct metal_device metal_dev_table[] = {
	{
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
		.irq_num = 0,
		.irq_info = NULL,
	}
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

	printf("RFdc Read and Write Example Test\r\n");
	/*
	 * Specify the Device ID
	 * generated in xparameters.h.
	 */
	Status = RFdcReadWriteExample(RFDC_DEVICE_ID);
	if (Status != XRFDC_SUCCESS) {
		printf("Read and Write Example Test failed\r\n");
		return XRFDC_FAILURE;
	}

	printf("Successfully ran Read and Write Example\r\n");
	return XRFDC_SUCCESS;
}

#ifdef __BAREMETAL__
/****************************************************************************/
/**
*
* This function registers devices to the libmetal generic bus.
* Before accessing the device with libmetal device operation,
* register the device to a libmetal supported bus. For non-Linux system,
* libmetal only supports "generic" bus to manage memory mapped devices.
*
* @param	None.
*
* @return
*		0 - succeeded, non-zero for failures.
*
* @note		None.
*
*****************************************************************************/
int register_metal_device(void)
{
	unsigned int i;
	int ret;

	for (i = 0; i < 1; i++) {
		device = &metal_dev_table[i];
		printf("registering: %d, name=%s\n", i, device->name);
		ret = metal_register_generic_device(device);
		if (ret)
			return ret;
	}
	return 0;
}
#endif

/****************************************************************************/
/**
*
* This function runs a test on the RFSoC data converter device using the
* driver APIs.
* This function does the following tasks:
*	- Initialize the RFdc device driver instance
*	- Compare Set and Get settings
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
int RFdcReadWriteExample(u16 RFdcDeviceId)
{

	int Status;
	u16 Tile;
	u16 Block;
	XRFdc_Config *ConfigPtr;
	XRFdc *RFdcInstPtr = &RFdcInst;
	u32 SetFabricRate = 8;
	u32 GetFabricRate;
	XRFdc_Mixer_Settings SetMixerSettings = {0};
	XRFdc_Mixer_Settings GetMixerSettings = {0};
	XRFdc_QMC_Settings SetQMCSettings;
	XRFdc_QMC_Settings GetQMCSettings;
	XRFdc_CoarseDelay_Settings SetCoarseDelaySettings;
	XRFdc_CoarseDelay_Settings GetCoarseDelaySettings;
	XRFdc_Threshold_Settings SetThresholdSettings;
	XRFdc_Threshold_Settings GetThresholdSettings;
	u32 SetDecoderMode;
	u32 GetDecoderMode;
	u32 SetNyquistZone;
	u32 GetNyquistZone;
	XRFdc_BlockStatus BlockStatus;
	u32 OutputCurr;
	u8 SetFIFOEnable;
	u8 GetFIFOEnable;
	u32 SetInterpolationFactor;
	u32 GetInterpolationFactor;
	u32 SetDecimationFactor;
	u32 GetDecimationFactor;
	u8 SetCalibrationMode;
	u8 GetCalibrationMode;
	u16 SetInvSinc;
	u16 GetInvSinc;
	u32 GetLinkCM;
	u16 SetFabClkDiv;
	u16 GetFabClkDiv;
	XRFdc_PLL_Settings PLLSettings;
	int ret = 0;
#ifndef __BAREMETAL__
	char DeviceName[NAME_MAX];
#endif

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

	/* Initializes the controller */
	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

#ifdef XPS_BOARD_ZCU111
printf("\n Configuring the Clock \r\n");
#ifdef __BAREMETAL__
	LMK04208ClockConfig(1, LMK04208_CKin);
	LMX2594ClockConfig(1, 3932160);
#else
	LMK04208ClockConfig(12, LMK04208_CKin);
	LMX2594ClockConfig(12, 3932160);
#endif
#endif

#ifdef __BAREMETAL__
	ret = register_metal_device();
	if (ret) {
		printf("%s: failed to register devices: %d\n", __func__, ret);
		return ret;
	}
	ret = metal_device_open(BUS_NAME, RFDC_DEV_NAME, &device);
	if (ret) {
		printf("ERROR: Failed to open device usp_rf_data_converter.\n");
		return XRFDC_FAILURE;
	}
#else
	Status = XRFdc_GetDeviceNameByDeviceId(DeviceName, RFDC_DEVICE_ID);
	if (Status < 0) {
		printf("ERROR: Failed to find rfdc device with device id %d\n",RFDC_DEVICE_ID);
		return XRFDC_FAILURE;
	}
	ret = metal_device_open(BUS_NAME, DeviceName, &device);
	if (ret) {
		printf("ERROR: Failed to open device %s.\n", DeviceName);
		return XRFDC_FAILURE;
	}
#endif
	/* Map RFDC device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		printf("ERROR: Failed to map RFDC regio for %s.\n",
			  device->name);
		return XRFDC_FAILURE;
	}
	RFdcInstPtr->device = device;
	RFdcInstPtr->io = io;

	for (Tile = 0; Tile <4; Tile++) {
		for (Block = 0; Block <4; Block++) {
			/* Check for DAC block Enable */
			if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
				/* Set DAC fabric rate */
				Status = XRFdc_SetFabWrVldWords(RFdcInstPtr, Tile, Block, SetFabricRate);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetFabricRate);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetFabricRate != GetFabricRate) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Set new mixer configurations */
				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2R;
				SetMixerSettings.Freq = 0x0;
				SetMixerSettings.PhaseOffset = 22.56789;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_0P7;
				SetMixerSettings.EventSource = XRFDC_EVNT_SRC_IMMEDIATE;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Set new mixer configurations */
				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2R;
				SetMixerSettings.Freq = -2000;
				SetMixerSettings.PhaseOffset = 22.56789;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_0P7;
				SetMixerSettings.EventSource = XRFDC_EVNT_SRC_IMMEDIATE;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				SetQMCSettings.EnableGain = 0;
				SetQMCSettings.EnablePhase = 0;
				SetQMCSettings.OffsetCorrectionFactor = -4;
				SetQMCSettings.PhaseCorrectionFactor = 26.499;
				SetQMCSettings.EventSource = XRFDC_EVNT_SRC_SYSREF;
				/* Set QMC settings */
				Status = XRFdc_SetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareQMCSettings(&SetQMCSettings, &GetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetCoarseDelaySettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				/* Set Coarse delay settings */
				SetCoarseDelaySettings.CoarseDelay = 2;
				SetCoarseDelaySettings.EventSource = XRFDC_EVNT_SRC_PL;
				Status = XRFdc_SetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetCoarseDelaySettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetCoarseDelaySettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = CompareCoarseDelaySettings(&SetCoarseDelaySettings, &GetCoarseDelaySettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				SetDecoderMode = XRFDC_DECODER_MAX_SNR_MODE;
				Status = XRFdc_SetDecoderMode(RFdcInstPtr, Tile, Block, SetDecoderMode);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetDecoderMode(RFdcInstPtr, Tile, Block, &GetDecoderMode);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetDecoderMode != GetDecoderMode) {
					return XRFDC_FAILURE;
				}
				SetNyquistZone = XRFDC_EVEN_NYQUIST_ZONE;
				Status = XRFdc_SetNyquistZone(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, SetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetNyquistZone(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetNyquistZone != GetNyquistZone) {
					return XRFDC_FAILURE;
				}
				SetNyquistZone = XRFDC_ODD_NYQUIST_ZONE;
				Status = XRFdc_SetNyquistZone(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, SetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetNyquistZone(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetNyquistZone != GetNyquistZone) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetBlockStatus(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &BlockStatus);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				printf("\n DAC%d%d Status \n"
				"DataPathClockStatus - %d \t IsFIFOFlagsEnabled - %d \t IsFIFOFlagsAsserted - %d \r\n", Tile, Block,
						BlockStatus.DataPathClocksStatus, BlockStatus.IsFIFOFlagsEnabled, BlockStatus.IsFIFOFlagsAsserted);

				Status = XRFdc_GetOutputCurr(RFdcInstPtr, Tile, Block, &OutputCurr);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				printf("\n DAC%d%d Output Current is %dmA \n", Tile, Block, OutputCurr);

				SetFIFOEnable = 0x1;
				Status = XRFdc_SetupFIFO(RFdcInstPtr,
					XRFDC_DAC_TILE, Tile, SetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFIFOStatus(RFdcInstPtr,
					XRFDC_DAC_TILE, Tile, &GetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFIFOEnable != GetFIFOEnable)
					return XRFDC_FAILURE;
				SetInterpolationFactor = XRFDC_INTERP_DECIM_OFF;
				Status = XRFdc_SetInterpolationFactor(RFdcInstPtr, Tile, Block,
									SetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, Tile, Block,
									&GetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInterpolationFactor != GetInterpolationFactor)
					return XRFDC_FAILURE;
				SetInterpolationFactor = XRFDC_INTERP_DECIM_2X;
				Status = XRFdc_SetInterpolationFactor(RFdcInstPtr, Tile, Block,
									SetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, Tile, Block,
									&GetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInterpolationFactor != GetInterpolationFactor)
					return XRFDC_FAILURE;
				SetInterpolationFactor = XRFDC_INTERP_DECIM_4X;
				Status = XRFdc_SetInterpolationFactor(RFdcInstPtr, Tile, Block,
									SetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, Tile, Block,
									&GetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInterpolationFactor != GetInterpolationFactor)
					return XRFDC_FAILURE;
				SetInterpolationFactor = XRFDC_INTERP_DECIM_8X;
				Status = XRFdc_SetInterpolationFactor(RFdcInstPtr, Tile, Block,
									SetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInterpolationFactor(RFdcInstPtr, Tile, Block,
									&GetInterpolationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInterpolationFactor != GetInterpolationFactor)
					return XRFDC_FAILURE;
				SetInvSinc = 0x1;
				Status = XRFdc_SetInvSincFIR(RFdcInstPtr, Tile, Block,
												SetInvSinc);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInvSincFIR(RFdcInstPtr, Tile, Block,
												&GetInvSinc);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInvSinc != GetInvSinc)
					return XRFDC_FAILURE;
				SetInvSinc = 0x0;
				Status = XRFdc_SetInvSincFIR(RFdcInstPtr, Tile, Block,
												SetInvSinc);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetInvSincFIR(RFdcInstPtr, Tile, Block,
												&GetInvSinc);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetInvSinc != GetInvSinc)
					return XRFDC_FAILURE;

				SetFabClkDiv = XRFDC_FAB_CLK_DIV1;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;

				SetFabClkDiv = XRFDC_FAB_CLK_DIV16;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;

				SetFabClkDiv = XRFDC_FAB_CLK_DIV4;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_DAC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;
			}

			/* Check if the ADC block is enabled */
			if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
				if (RFdcInstPtr->ADC4GSPS == XRFDC_ADC_4GSPS) {
					if ((Block == 2) || (Block == 3))
						continue;
					else if (Block == 1) {
						if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, 2) == 0)
							continue;
					}
				}
				/* Set ADC fabric rate */
				SetFabricRate = 2;
				Status = XRFdc_SetFabRdVldWords(RFdcInstPtr, Tile, Block, SetFabricRate);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetFabricRate);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetFabricRate != GetFabricRate) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 0x0;
				SetMixerSettings.PhaseOffset = 0x0;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_0P7;
				SetMixerSettings.EventSource = XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 350;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource = XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				SetQMCSettings.EnableGain = 0;
				SetQMCSettings.EnablePhase = 0;
				SetQMCSettings.EventSource = XRFDC_EVNT_SRC_PL;
				Status = XRFdc_SetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}

				/* Compare the settings */
				Status = CompareQMCSettings(&SetQMCSettings, &GetQMCSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				/* Get Threshold settings */
				Status = XRFdc_GetThresholdSettings(RFdcInstPtr, Tile, Block, &SetThresholdSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				SetThresholdSettings.ThresholdOverVal[0] = 8191;
				SetThresholdSettings.ThresholdOverVal[1] = 6000;
				SetThresholdSettings.ThresholdUnderVal[0] = 640;
				SetThresholdSettings.ThresholdUnderVal[1] = 640;
				SetThresholdSettings.UpdateThreshold = XRFDC_UPDATE_THRESHOLD_BOTH;

				/* Set Threshold settings */
				Status = XRFdc_SetThresholdSettings(RFdcInstPtr, Tile, Block, &SetThresholdSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetThresholdSettings(RFdcInstPtr, Tile, Block, &GetThresholdSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = CompareThresholdSettings(&SetThresholdSettings, &GetThresholdSettings);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				SetNyquistZone = XRFDC_EVEN_NYQUIST_ZONE;
				Status = XRFdc_SetNyquistZone(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, SetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetNyquistZone(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetNyquistZone != GetNyquistZone) {
					return XRFDC_FAILURE;
				}
				SetNyquistZone = XRFDC_ODD_NYQUIST_ZONE;
				Status = XRFdc_SetNyquistZone(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, SetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetNyquistZone(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetNyquistZone);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				if (SetNyquistZone != GetNyquistZone) {
					return XRFDC_FAILURE;
				}
				Status = XRFdc_GetBlockStatus(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &BlockStatus);
				if (Status != XRFDC_SUCCESS) {
					return XRFDC_FAILURE;
				}
				printf("\n ADC%d%d Status \n"
				"DataPathClockStatus - %d \t IsFIFOFlagsEnabled - %d \t IsFIFOFlagsAsserted - %d \r\n", Tile, Block, BlockStatus.DataPathClocksStatus,
				BlockStatus.IsFIFOFlagsEnabled, BlockStatus.IsFIFOFlagsAsserted);
				SetFIFOEnable = 0x0;
				Status = XRFdc_SetupFIFO(RFdcInstPtr,
					XRFDC_ADC_TILE, Tile, SetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFIFOStatus(RFdcInstPtr,
					XRFDC_ADC_TILE, Tile, &GetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFIFOEnable != GetFIFOEnable)
					return XRFDC_FAILURE;
				SetFIFOEnable = 0x1;
				Status = XRFdc_SetupFIFO(RFdcInstPtr,
					XRFDC_ADC_TILE, Tile, SetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFIFOStatus(RFdcInstPtr,
					XRFDC_ADC_TILE, Tile, &GetFIFOEnable);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFIFOEnable != GetFIFOEnable)
					return XRFDC_FAILURE;
				SetDecimationFactor = XRFDC_INTERP_DECIM_OFF;
				Status = XRFdc_SetDecimationFactor(RFdcInstPtr, Tile, Block,
									SetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block,
									&GetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetDecimationFactor != GetDecimationFactor)
					return XRFDC_FAILURE;
				SetDecimationFactor = XRFDC_INTERP_DECIM_1X;
				Status = XRFdc_SetDecimationFactor(RFdcInstPtr, Tile, Block,
									SetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block,
									&GetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetDecimationFactor != GetDecimationFactor)
					return XRFDC_FAILURE;
				SetDecimationFactor = XRFDC_INTERP_DECIM_2X;
				Status = XRFdc_SetDecimationFactor(RFdcInstPtr, Tile, Block,
									SetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block,
									&GetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetDecimationFactor != GetDecimationFactor)
					return XRFDC_FAILURE;
				SetDecimationFactor = XRFDC_INTERP_DECIM_4X;
				Status = XRFdc_SetDecimationFactor(RFdcInstPtr, Tile, Block,
									SetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block,
									&GetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetDecimationFactor != GetDecimationFactor)
					return XRFDC_FAILURE;
				SetDecimationFactor = XRFDC_INTERP_DECIM_8X;
				Status = XRFdc_SetDecimationFactor(RFdcInstPtr, Tile, Block,
									SetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetDecimationFactor(RFdcInstPtr, Tile, Block,
									&GetDecimationFactor);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetDecimationFactor != GetDecimationFactor)
					return XRFDC_FAILURE;

				/* Set Calibration Mode 1 */
				SetCalibrationMode = XRFDC_CALIB_MODE1;
				Status = XRFdc_SetCalibrationMode(RFdcInstPtr,
					Tile, Block, SetCalibrationMode);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetCalibrationMode(RFdcInstPtr,
					Tile, Block, &GetCalibrationMode);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetCalibrationMode != GetCalibrationMode)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_SAMPLE_FREQ_BY_FOUR;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 700;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
							&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2C;
				SetMixerSettings.Freq = 1200;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
							&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 2300;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
							&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_MIN_SAMPLE_FREQ_BY_FOUR;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2C;
				SetMixerSettings.Freq = -2500;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
							&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;


				/* Set Calibration Mode 2 */
				SetCalibrationMode = XRFDC_CALIB_MODE2;
				Status = XRFdc_SetCalibrationMode(RFdcInstPtr,
					Tile, Block, SetCalibrationMode);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetCalibrationMode(RFdcInstPtr,
					Tile, Block, &GetCalibrationMode);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetCalibrationMode != GetCalibrationMode)
					return XRFDC_FAILURE;
				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 4500;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
						&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_BYPASS;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_COARSE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2C;
				SetMixerSettings.Freq = 700;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
						&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_R2C;
				SetMixerSettings.Freq = 1200;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
						&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				SetMixerSettings.CoarseMixFreq = XRFDC_COARSE_MIX_OFF;
				SetMixerSettings.MixerType = XRFDC_MIXER_TYPE_FINE;
				SetMixerSettings.MixerMode = XRFDC_MIXER_MODE_C2C;
				SetMixerSettings.Freq = 2300;
				SetMixerSettings.PhaseOffset = -9.0565;
				SetMixerSettings.FineMixerScale = XRFDC_MIXER_SCALE_AUTO;
				SetMixerSettings.EventSource =
						XRFDC_EVNT_SRC_SYSREF;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &SetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE,
					Tile, Block, &GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;

				Status = CompareMixerSettings(&SetMixerSettings,
						&GetMixerSettings);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetLinkCoupling(RFdcInstPtr, Tile, Block, &GetLinkCM);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				printf("\n ADC%d%d: Link Coupling Mode is %d \r\n", Tile, Block, GetLinkCM);

				SetFabClkDiv = XRFDC_FAB_CLK_DIV4;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;

				SetFabClkDiv = XRFDC_FAB_CLK_DIV16;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;

				SetFabClkDiv = XRFDC_FAB_CLK_DIV8;
				Status = XRFdc_SetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, SetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				Status = XRFdc_GetFabClkOutDiv(RFdcInstPtr, XRFDC_ADC_TILE,
						Tile, &GetFabClkDiv);
				if (Status != XRFDC_SUCCESS)
					return XRFDC_FAILURE;
				if (SetFabClkDiv != GetFabClkDiv)
					return XRFDC_FAILURE;
			}
		}
		if (XRFdc_GetNoOfADCBlocks(RFdcInstPtr, Tile) != 0U) {
			XRFdc_GetPLLConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile, &PLLSettings);
			printf("\n ADC%d PLL Configurations:: PLL Enable is %d \tFeedback Divider is %d \tOutputDivider is %d \tReferenceClk Divider is %d \r\n",
				Tile, PLLSettings.Enabled, PLLSettings.FeedbackDivider, PLLSettings.OutputDivider, PLLSettings.RefClkDivider);
		}

		if (XRFdc_GetNoOfDACBlock(RFdcInstPtr, Tile) != 0U) {
			XRFdc_GetPLLConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile, &PLLSettings);
			printf("\n DAC%d PLL Configurations:: PLL Enable is %d \tFeedback Divider is %d \tOutputDivider is %d \tReferenceClk Divider is %d \r\n",
				Tile, PLLSettings.Enabled, PLLSettings.FeedbackDivider, PLLSettings.OutputDivider, PLLSettings.RefClkDivider);
		}
	}

	Tile = 0U;
	printf("=======Default DigitalDataPath Configuration for Tile%d======\r\n", Tile);
	for (Block = 0; Block <4; Block++) {
		/* Check for DAC block Enable */
		if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
			printf("\n DAC DigitalDataPath%d-> Connected I data = %d",
					Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
			printf("\n DAC DigitalDataPath%d-> Connected Q data = %d",
					Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
		}

		/* Check if the ADC block is enabled */
		if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
			printf("\n ADC DigitalDataPath%d-> Connected I data = %d",
					Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
			printf("\n ADC DigitalDataPath%d-> Connected Q data = %d",
					Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
		}
	}
	printf("\n ADC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile));
	printf("\n DAC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));
	printf("\n ============================================\r\n");

	/* ADC-4G Singleband R2C */
	Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_ADC_TILE, Tile, 0x1, XRFDC_MB_DATATYPE_R2C, 0x1);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	printf("=============ADC0-4G SB Configuration R2C==========\r\n");
	for (Block = 0; Block <4; Block++) {
		/* Check if the ADC block is enabled */
		if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
			printf("\n ADC DigitalDataPath%d-> Connected I data = %d",
					Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
			printf("\n ADC DigitalDataPath%d-> Connected Q data = %d",
					Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
		}
		if (Block == 0) {
			if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != 0)
				return XRFDC_FAILURE;
			if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != -1)
				return XRFDC_FAILURE;
		}
	}
	printf("\n ADC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile));
	printf("\n ================================================\r\n");

	if (RFdcInstPtr->ADC_Tile[Tile].NumOfADCBlocks >= 2U) {
		/* ADC-4G Multiband 2x R2C */
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_ADC_TILE, Tile, 0x3, XRFDC_MB_DATATYPE_R2C, 0x1);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		printf("=============ADC0,1-4G MB Configuration R2C==========\r\n");
		for (Block = 0; Block <4; Block++) {
			/* Check if the ADC block is enabled */
			if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
				printf("\n ADC DigitalDataPath%d-> Connected I data = %d",
						Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
				printf("\n ADC DigitalDataPath%d-> Connected Q data = %d",
						Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
			}
			if ((Block == 0) || (Block == 1)) {
				if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != 0)
					return XRFDC_FAILURE;
				if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != -1)
					return XRFDC_FAILURE;
			}
		}
		printf("\n ADC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile));
		printf("\n ================================================\r\n");

		/* ADC-4G Multiband 2x C2C */
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_ADC_TILE, Tile, 0x3, XRFDC_MB_DATATYPE_C2C, 0x3);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		printf("=============ADC0,1-4G MB Configuration C2C==========\r\n");
		for (Block = 0; Block <4; Block++) {
			/* Check if the ADC block is enabled */
			if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
				printf("\n ADC DigitalDataPath%d-> Connected I data = %d",
						Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
				printf("\n ADC DigitalDataPath%d-> Connected Q data = %d",
						Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
			}
			if ((Block == 0) || (Block == 1)) {
				if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != 0)
					return XRFDC_FAILURE;
				if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != 1)
					return XRFDC_FAILURE;
			}
		}
		printf("\n ADC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile));
		printf("\n ================================================\r\n");

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_ADC_TILE, Tile, 0x2, XRFDC_MB_DATATYPE_R2C, 0x2);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		printf("=============ADC0,1-4G SB Configuration R2C==========\r\n");
		for (Block = 0; Block <4; Block++) {
			/* Check if the ADC block is enabled */
			if (XRFdc_IsADCBlockEnabled(RFdcInstPtr, Tile, Block)) {
				printf("\n ADC DigitalDataPath%d-> Connected I data = %d",
						Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
				printf("\n ADC DigitalDataPath%d-> Connected Q data = %d",
						Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block));
			}
			if (Block == 1) {
				if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != 1)
					return XRFDC_FAILURE;
				if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block) != -1)
					return XRFDC_FAILURE;
			}
		}
		printf("\n ADC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_ADC_TILE, Tile));
		printf("\n ================================================\r\n");
	}

	/* DAC Singleband C2R */
	Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x1, XRFDC_MB_DATATYPE_C2R, 0x1);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}

	printf("=============DAC0 SB Configuration C2R==========\r\n");
	for (Block = 0; Block <4; Block++) {
		/* Check for DAC block Enable */
		if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
			printf("\n DAC DigitalDataPath%d-> Connected I data = %d",
					Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
			printf("\n DAC DigitalDataPath%d-> Connected Q data = %d",
					Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
		}
		if (Block == 0) {
			if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != 0)
				return XRFDC_FAILURE;
			if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != -1)
				return XRFDC_FAILURE;
		}
	}
	printf("\n DAC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));
	printf("\n ============================================\r\n");

	if (RFdcInstPtr->DAC_Tile[Tile].NumOfDACBlocks >= 2U) {
		/* DAC Singleband C2C */
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x1, XRFDC_MB_DATATYPE_C2C, 0x3);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		printf("=============DAC0 SB Configuration C2C==========\r\n");
		for (Block = 0; Block <4; Block++) {
			/* Check for DAC block Enable */
			if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
				printf("\n DAC DigitalDataPath%d-> Connected I data = %d",
						Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
				printf("\n DAC DigitalDataPath%d-> Connected Q data = %d",
						Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
			}
			if (Block == 0) {
				if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != 0)
					return XRFDC_FAILURE;
				if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != 1)
					return XRFDC_FAILURE;
			}
		}
		printf("\n DAC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));
		printf("\n ============================================\r\n");

		/* DAC Multiband 2x C2C */
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x3, XRFDC_MB_DATATYPE_C2C, 0x3);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}

		printf("=======DAC0,1 MB 2X Configuration C2C=======\r\n");
		for (Block = 0; Block <4; Block++) {
			/* Check for DAC block Enable */
			if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
				printf("\n DAC DigitalDataPath%d-> Connected I data = %d",
						Block, XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
				printf("\n DAC DigitalDataPath%d-> Connected Q data = %d",
						Block, XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block));
			}
			if ((Block == 0) || (Block == 1)) {
				if (XRFdc_GetConnectedIData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != 0)
					return XRFDC_FAILURE;
				if (XRFdc_GetConnectedQData(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block) != 1)
					return XRFDC_FAILURE;
			}
		}
		printf("\n DAC0 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));
		printf("\n ============================================\r\n");
	}

	if (RFdcInstPtr->DAC_Tile[Tile].NumOfDACBlocks == 4U) {
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0xC, XRFDC_MB_DATATYPE_C2C, 0xC);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC2,3 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0xF, XRFDC_MB_DATATYPE_C2C, 0xF);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC 4X MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x3, XRFDC_MB_DATATYPE_C2C, 0x3);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC0,1 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0xC, XRFDC_MB_DATATYPE_C2C, 0xC);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC2,3 MB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x1, XRFDC_MB_DATATYPE_C2C, 0x1);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x2, XRFDC_MB_DATATYPE_C2C, 0x2);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC0, 1 SB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));

		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x4, XRFDC_MB_DATATYPE_C2C, 0x4);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		Status = XRFdc_MultiBand(RFdcInstPtr, XRFDC_DAC_TILE, Tile, 0x8, XRFDC_MB_DATATYPE_C2C, 0x8);
		if (Status != XRFDC_SUCCESS) {
			return XRFDC_FAILURE;
		}
		printf("\n DAC2, 3 SB Config is %d \r\n", XRFdc_GetMultibandConfig(RFdcInstPtr, XRFDC_DAC_TILE, Tile));
	}

	for (Tile = XRFDC_BLK_ID0; Tile <= XRFDC_BLOCK_ID_MAX; Tile++) {
		if (XRFdc_CheckTileEnabled(RFdcInstPtr, XRFDC_ADC_TILE, Tile) == XRFDC_SUCCESS)
			XRFdc_DumpRegs(RFdcInstPtr, XRFDC_ADC_TILE, Tile);

		if (XRFdc_CheckTileEnabled(RFdcInstPtr, XRFDC_DAC_TILE, Tile) == XRFDC_SUCCESS)
			XRFdc_DumpRegs(RFdcInstPtr, XRFDC_DAC_TILE, Tile);
	}

	return XRFDC_SUCCESS;
}

/****************************************************************************/
/*
*
* This function compares the two mixer settings structures and return 0 if
* same and returns 1 if not same.
*
* @param	SetMixerSettings Pointer to the XRFdc_Mixer_Settings structure.
* @param	GetMixerSettings Pointer to the XRFdc_Mixer_Settings structure.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareMixerSettings(XRFdc_Mixer_Settings *SetMixerSettings,
								 XRFdc_Mixer_Settings *GetMixerSettings)
{
	/* Removed Coarse mix freq check */
	if ((SetMixerSettings->EventSource == GetMixerSettings->EventSource) &&
			((SetMixerSettings->Freq -
					GetMixerSettings->Freq) < 0.1) &&
			((SetMixerSettings->PhaseOffset -
					GetMixerSettings->PhaseOffset) < 0.1) &&
			(SetMixerSettings->FineMixerScale ==
					GetMixerSettings->FineMixerScale) &&
			(SetMixerSettings->MixerMode == GetMixerSettings->MixerMode) &&
			(SetMixerSettings->CoarseMixFreq ==
					GetMixerSettings->CoarseMixFreq) &&
			(SetMixerSettings->MixerType ==
				GetMixerSettings->MixerType))
		return 0;
	else
		return 1;
}

/****************************************************************************/
/*
*
* This function compares the two QMC settings structures and return 0 if
* same and returns 1 if not same.
*
* @param	SetQMCSettings Pointer to the XRFdc_QMC_Settings structure.
* @param	GetQMCSettings Pointer to the XRFdc_QMC_Settings structure.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareQMCSettings(XRFdc_QMC_Settings *SetQMCSettings,
								 XRFdc_QMC_Settings *GetQMCSettings)
{
	if ((SetQMCSettings->EnableGain == GetQMCSettings->EnableGain) &&
			(SetQMCSettings->EnablePhase == GetQMCSettings->EnablePhase) &&
			(SetQMCSettings->EventSource == GetQMCSettings->EventSource) &&
			//TODO (SetQMCSettings->GainCorrectionFactor == GetQMCSettings->GainCorrectionFactor) &&
			(SetQMCSettings->OffsetCorrectionFactor == GetQMCSettings->OffsetCorrectionFactor))
		return 0;
	else
		return 1;
}

/****************************************************************************/
/*
*
* This function compares the two CoarseDelay settings structures and return 0 if
* same and returns 1 if not same.
*
* @param	SetQMCSettings Pointer to the XRFdc_CoarseDelay_Settings structure.
* @param	GetQMCSettings Pointer to the XRFdc_CoarseDelay_Settings structure.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareCoarseDelaySettings(XRFdc_CoarseDelay_Settings *SetCoarseDlySettings,
								 XRFdc_CoarseDelay_Settings *GetCoarseDlySettings)
{
	/* Removed Coarse mix freq check */
	if ((SetCoarseDlySettings->CoarseDelay == GetCoarseDlySettings->CoarseDelay) &&
			(SetCoarseDlySettings->EventSource == GetCoarseDlySettings->EventSource))
		return 0;
	else
		return 1;
}

/****************************************************************************/
/*
*
* This function compares the two Threshold settings structures and return 0 if
* same and returns 1 if not same.
*
* @param	SetQMCSettings Pointer to the XRFdc_CoarseDelay_Settings structure.
* @param	GetQMCSettings Pointer to the XRFdc_CoarseDelay_Settings structure.
*
* @return
*			- 0 if both structures are same.
*			- 1 if both structures are not same.
*
* @note		None
*
*****************************************************************************/
static int CompareThresholdSettings(XRFdc_Threshold_Settings *SetThresholdSettings,
								 XRFdc_Threshold_Settings *GetThresholdSettings)
{
	if ((SetThresholdSettings->ThresholdAvgVal[0] == GetThresholdSettings->ThresholdAvgVal[0]) &&
			(SetThresholdSettings->ThresholdAvgVal[1] == GetThresholdSettings->ThresholdAvgVal[1]) &&
			(SetThresholdSettings->ThresholdMode[0] == GetThresholdSettings->ThresholdMode[0]) &&
			(SetThresholdSettings->ThresholdMode[1] == GetThresholdSettings->ThresholdMode[1]) &&
			(SetThresholdSettings->ThresholdOverVal[0] == GetThresholdSettings->ThresholdOverVal[0]) &&
			(SetThresholdSettings->ThresholdOverVal[1] == GetThresholdSettings->ThresholdOverVal[1]) &&
			(SetThresholdSettings->ThresholdUnderVal[0] == GetThresholdSettings->ThresholdUnderVal[0]) &&
			(SetThresholdSettings->ThresholdUnderVal[1] == GetThresholdSettings->ThresholdUnderVal[1]))
		return 0;
	else
		return 1;
}
