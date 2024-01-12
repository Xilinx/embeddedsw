/*******************************************************************************
* Copyright (C) 2017 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright 2022-2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
 ******************************************************************************/

/******************************************************************************/
/**
 * @file xaudioformatter_selftest_example.c
 *
 * This file contains a example for using the Audio Formatter hardware device
 * and Audio Formatter driver.
 *
 *
 * <pre> MODIFICATION HISTORY:
 *
 * Ver	Who	Date		Changes
 * ---- ---	--------	-----------------------------------------------
 * 1.0	Dhanunjay	01/12/2024	First release
 * </pre>
 *
 ****************************************************************************/

/***************************** Include Files **********************************/
#include "xparameters.h"
#include "xaudioformatter.h"
#include "xil_printf.h"

/************************** Constant Definitions ******************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef SDT
#ifndef TESTAPP_GEN
#define AUDIO_FORMATTER_DEVICE_ID	XPAR_XAUDIOFORMATTER_0_DEVICE_ID
#endif
#endif
/**************************** Type Definitions *****************************i***/

/************************** Function Prototypes *******************************/
#ifndef SDT
int AFSelfTestExample(u16 DeviceId)
#else
int AFSelfTestExample(UINTPTR BaseAddress);
#endif

/************************** Variable Definitions ******************************/
XAudioFormatter Audioformatter;		/* Instance of the Audio Formatter device */
/******************************************************************************/
/**
 *
 * Main function to call the Self Test example.
 *
 * @param	None.
 *
 * @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful.
 *
 * @note		None.
 *
 *****************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	xil_printf("Audio Formatter Self Test Example \r\n");

	/*
	 * Run the Audio Formatter Self Test example, specify the Device ID that is
	 * generated in xparameters.h
	 */
#ifndef SDT
	Status = AFSelfTestExample(AUDIO_FORMATTER_DEVICE_ID);
#else
	Status = AFSelfTestExample(XPAR_XAUDIO_FORMATTER_0_BASEADDR);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Audio Formatter Self Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran Audio Formatter Self Test Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
 *
 * This function does a minimal test on the Audio Formatter device and
 * driver as a design example.
 *
 * @param	DeviceId is the Device ID of the Audio Formatter Device and is the
 *		XPAR_<Audio Formatter_instance>_DEVICE_ID value from xparameters.h
 *
 * @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
 *
 * @note		None.
 *
 *
 *****************************************************************************/
#ifndef SDT
int AFSelfTestExample(u16 DeviceId)
#else
int AFSelfTestExample(UINTPTR BaseAddress)
#endif
{
	int Status;

	XAudioFormatter_Config *Config;
	/*
	 * Initialize the Audio formatter driver so that it's ready to use
	 * Look up the configuration in the config table, then initialize it.
	 */
#ifndef SDT
	Config = XAudioFormatter_LookupConfig(DeviceId);
#else
	Config = XAudioFormatter_LookupConfig(BaseAddress);
#endif

	if (Config == NULL)
		return XST_FAILURE;

	Status = XAudioFormatter_CfgInitialize(&Audioformatter, Config);
	if (Status != XST_SUCCESS)
		return XST_FAILURE;

	return Status;
}
