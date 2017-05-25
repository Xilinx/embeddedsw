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
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.0   sk     05/15/17 First release
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xrfdc.h"
#include "xstatus.h"

/************************** Constant Definitions ****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define RFDC_DEVICE_ID 	XPAR_XRFDC_0_DEVICE_ID


/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions ********************/

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

/************************** Variable Definitions ****************************/

static XRFdc RFdcInst;      /* RFdc driver instance */

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

	xil_printf("RFdc Read and Write Example Test\r\n");
	/*
	 * Run the RFdc Ericsson use case example, specify the Device ID that is
	 * generated in xparameters.h.
	 */
	Status = RFdcReadWriteExample(RFDC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("Read and Write Example Test failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Read and Write Example\r\n");
	return XST_SUCCESS;
}


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
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
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
	u32 OutputCurrent = 32;
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

	/* Initialize the RFdc driver. */
	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XST_FAILURE;
	}

	/* Initializes the controller */
	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	for (Tile = 0; Tile <4; Tile++) {
		for (Block = 0; Block <4; Block++) {
			/* Check for DAC block Enable */
			if (XRFdc_IsDACBlockEnabled(RFdcInstPtr, Tile, Block)) {
				/* Set DAC fabric rate */
				Status = XRFdc_SetFabWrVldWords(RFdcInstPtr, Tile, Block, SetFabricRate);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetFabricRate);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (SetFabricRate != GetFabricRate) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				/* Set new mixer configurations */
				SetMixerSettings.CoarseMixFreq = 0x0;	// Coarse mix OFF
				SetMixerSettings.Freq = 3500;	//MHz
				SetMixerSettings.FineMixerMode = 0x2;	//Complex to Real
				SetMixerSettings.PhaseOffset = 22.5;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				SetQMCSettings.EnableGain = 0;
				SetQMCSettings.EnablePhase = 0;
				SetQMCSettings.OffsetCorrectionFactor = -4;
				SetQMCSettings.PhaseCorrectionFactor = 26.5;
				/* Set QMC settings */
				Status = XRFdc_SetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				/* Compare the settings */
				Status = CompareQMCSettings(&SetQMCSettings, &GetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetCoarseDelaySettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				/* Set Coarse delay settings */
				SetCoarseDelaySettings.CoarseDelay = 2;
				Status = XRFdc_SetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &SetCoarseDelaySettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetCoarseDelaySettings(RFdcInstPtr, XRFDC_DAC_TILE, Tile, Block, &GetCoarseDelaySettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = CompareCoarseDelaySettings(&SetCoarseDelaySettings, &GetCoarseDelaySettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_SetOutputCurrent(RFdcInstPtr, Tile, Block, OutputCurrent);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				SetDecoderMode = XRFDC_DECODER_MAX_SNR_MODE;
				Status = XRFdc_SetDecoderMode(RFdcInstPtr, Tile, Block, SetDecoderMode);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetDecoderMode(RFdcInstPtr, Tile, Block, &GetDecoderMode);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (SetDecoderMode != GetDecoderMode) {
					return XST_FAILURE;
				}
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
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetFabRdVldWords(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetFabricRate);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				if (SetFabricRate != GetFabricRate) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				SetMixerSettings.CoarseMixFreq = 0x0; 	//CoarseMix OFF
				SetMixerSettings.Freq = 3500; 	//MHz
				SetMixerSettings.FineMixerMode = 0x2;	// Complex to real
				SetMixerSettings.PhaseOffset = 14.06;
				/* Set Mixer settings */
				Status = XRFdc_SetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				/* Compare the settings */
				Status = CompareMixerSettings(&SetMixerSettings, &GetMixerSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				SetQMCSettings.EnableGain = 0;
				SetQMCSettings.EnablePhase = 0;
				Status = XRFdc_SetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &SetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				Status = XRFdc_GetQMCSettings(RFdcInstPtr, XRFDC_ADC_TILE, Tile, Block, &GetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}

				/* Compare the settings */
				Status = CompareQMCSettings(&SetQMCSettings, &GetQMCSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				/* Get Threshold settings */
				Status = XRFdc_GetThresholdSettings(RFdcInstPtr, Tile, Block, &SetThresholdSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				SetThresholdSettings.ThresholdOverVal[0] = 8191;
				SetThresholdSettings.ThresholdOverVal[1] = 6000;
				SetThresholdSettings.ThresholdUnderVal[0] = 640;
				SetThresholdSettings.ThresholdUnderVal[1] = 640;
				SetThresholdSettings.UpdateThreshold = XRFDC_UPDATE_THRESHOLD_BOTH;

				/* Set Threshold settings */
				Status = XRFdc_SetThresholdSettings(RFdcInstPtr, Tile, Block, &SetThresholdSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = XRFdc_GetThresholdSettings(RFdcInstPtr, Tile, Block, &GetThresholdSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
				Status = CompareThresholdSettings(&SetThresholdSettings, &GetThresholdSettings);
				if (Status != XST_SUCCESS) {
					return XST_FAILURE;
				}
			}
		}
	}


	return XST_SUCCESS;
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
			(SetMixerSettings->Freq == GetMixerSettings->Freq) &&
			(SetMixerSettings->FineMixerMode == GetMixerSettings->FineMixerMode) &&
			(SetMixerSettings->CoarseMixFreq == GetMixerSettings->CoarseMixFreq))
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
