/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file     xrtcpsu_set_calibration_example.c
*
* This file contains an example using the XRtcPsu driver.
*
* This function calculates new calibration value and updates the
* calibration register value.
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
* 1.8   sg  07/17/19 Update example sequence for finding
*			new calibration values
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xrtcpsu.h"		/* RTCPSU device driver */
#include "xil_printf.h"
#include <stdio.h>
#include "sleep.h"

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

	/* Set RTC time to user input time */
	XRtcPsu_SetTime(&Rtc_Psu,NetworkTime);

	/*
	 * For time accuracy RTC module need to be calibrated at regular interval,
	 * new calibration value calculation requires rtc set time, rtc current time
	 * and system/network time. For reference 10Sec system/network time interval
	 * used for finding new calibration values.
	 */
	sleep(10);
	NetworkTime = NetworkTime+10;

	XRtcPsu_CalculateCalibration(&Rtc_Psu,NetworkTime,OscillatorFreq);

	Status = XRtcPsu_CfgInitialize(&Rtc_Psu, Config, Config->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("New Calibration value : %08x\tCrystal Frequency : %08x\n\r",
			Rtc_Psu.CalibrationValue,Rtc_Psu.OscillatorFreq);

	return XST_SUCCESS;
}
