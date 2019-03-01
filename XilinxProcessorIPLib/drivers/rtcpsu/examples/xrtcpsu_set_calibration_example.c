/******************************************************************************
*
* Copyright (C) 2015 - 2019 Xilinx, Inc.  All rights reserved.
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
* XILINX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
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
* @file     xrtcpsu_set_calibration_example.c
*
* This file contains an example using the XRtcPsu driver.
*
* This function updates the calibration register value.
*
* @note
* If the device does not work properly, the example may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  kvn 05/12/15 First Release
* 1.6	tjs 09/17/18 Fixed compilation warnings
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xrtcpsu.h"		/* RTCPSU device driver */
#include "xil_printf.h"
#include <stdio.h>

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define RTC_DEVICE_ID              XPAR_XRTCPSU_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

int RtcPsuSetCalibrationExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRtcPsu Rtc_Psu;		/* Instance of the RTC Device */
XRtcPsu_Config *Config;

/*****************************************************************************/
/**
*
* Main function to call the Rtc set calibration example.
*
* @param	None
*
* @return	XST_SUCCESS if successful, otherwise XST_FAILURE.
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the Rtc_Psu set calibration example , specify the the Device ID
	 * that is generated in xparameters.h
	 */
	Status = RtcPsuSetCalibrationExample(RTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Set Calibration Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Set Calibration Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal set Calibration test on the XRtcPsu device.
*
* This function updates the Calibration register value.
*
* @param	DeviceId is the unique device id from hardware build.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None.
*
****************************************************************************/
int RtcPsuSetCalibrationExample(u16 DeviceId)
{
	int Status;
	u32 NetworkTime;
	XRtcPsu_DT dt1;
	u32 OscillatorFreq;

	/*
	 * Initialize the RTC driver so that it's ready to use.
	 * Look up the configuration in the config table, then initialize it.
	 */
	Config = XRtcPsu_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XRtcPsu_CfgInitialize(&Rtc_Psu, Config, Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Check hardware build. */
	Status = XRtcPsu_SelfTest(&Rtc_Psu);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("Enter Crystal oscillator frequency : ");
#if defined(__aarch64__)
	scanf("%d", &OscillatorFreq);
#else
	scanf("%ld", &OscillatorFreq);
#endif

	xil_printf("\n\rEnter Internet / Network Time YEAR:MM:DD HR:MM:SS : ");
#if defined(__aarch64__)
	scanf("%d %d %d %d %d %d", &dt1.Year, &dt1.Month, &dt1.Day,
			&dt1.Hour, &dt1.Min, &dt1.Sec);
#else
	scanf("%ld %ld %ld %ld %ld %ld", &dt1.Year, &dt1.Month, &dt1.Day,
			&dt1.Hour, &dt1.Min, &dt1.Sec);
#endif
	xil_printf("%d %d %d %d %d %d\n\r",dt1.Year,dt1.Month,dt1.Day,dt1.Hour,dt1.Min,dt1.Sec);

	NetworkTime = XRtcPsu_DateTimeToSec(&dt1);

	xil_printf("\n\rOld Calibration value : %08x\tCrystal Frequency : %08x\n\r",
			Rtc_Psu.CalibrationValue,Rtc_Psu.OscillatorFreq);

	XRtcPsu_CalculateCalibration(&Rtc_Psu,NetworkTime,OscillatorFreq);

	Status = XRtcPsu_CfgInitialize(&Rtc_Psu, Config, Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("New Calibration value : %08x\tCrystal Frequency : %08x\n\r",
			Rtc_Psu.CalibrationValue,Rtc_Psu.OscillatorFreq);

	return XST_SUCCESS;
}
