/******************************************************************************
* Copyright (C) 2015 - 2021 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file     xrtcpsu_set_time_example.c
*
* This file contains an example using the XRtcPsu driver.
*
* This function updates the current time.
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

int RtcPsuSetTimeExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRtcPsu Rtc_Psu;		/* Instance of the RTC Device */
XRtcPsu_Config *Config;

/*****************************************************************************/
/**
*
* Main function to call the Rtc Set time example.
*
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
	 * Run the Rtc_Psu set time example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = RtcPsuSetTimeExample(RTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Set time Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Set time Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal set time test on the XRtcPsu device.
*
* This function updates the current time to a specified time.
*
* @param	DeviceId is the unique device id from hardware build.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note		None.
*
****************************************************************************/
int RtcPsuSetTimeExample(u16 DeviceId)
{
	int Status;
	u32 CurrentTime, DesiredTime,LastSetTime;
	XRtcPsu_DT dt1,dt2,dt3;

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

	xil_printf("Day Convention : 0-Fri, 1-Sat, 2-Sun, 3-Mon, 4-Tue, 5-Wed, 6-Thur\n\r");
	xil_printf("Last set time for RTC is..\n\r");
	LastSetTime = XRtcPsu_GetLastSetTime(&Rtc_Psu);
	XRtcPsu_SecToDateTime(LastSetTime,&dt1);
	xil_printf("YEAR:MM:DD HR:MM:SS \t %04d:%02d:%02d %02d:%02d:%02d\t Day = %d\n\r",
			dt1.Year,dt1.Month,dt1.Day,dt1.Hour,dt1.Min,dt1.Sec,dt1.WeekDay);

	xil_printf("Current RTC time is..\n\r");
	CurrentTime = XRtcPsu_GetCurrentTime(&Rtc_Psu);
	XRtcPsu_SecToDateTime(CurrentTime,&dt2);
	xil_printf("YEAR:MM:DD HR:MM:SS \t %04d:%02d:%02d %02d:%02d:%02d\t Day = %d\n\r",
			dt2.Year,dt2.Month,dt2.Day,dt2.Hour,dt2.Min,dt2.Sec,dt2.WeekDay);

	xil_printf("Enter Desired Current Time YEAR:MM:DD HR:MM:SS : ");
#if defined(__aarch64__)
	scanf("%d %d %d %d %d %d", &dt3.Year, &dt3.Month, &dt3.Day,
			&dt3.Hour, &dt3.Min, &dt3.Sec);
#else
	scanf("%ld %ld %ld %ld %ld %ld", &dt3.Year, &dt3.Month, &dt3.Day,
			&dt3.Hour, &dt3.Min, &dt3.Sec);
#endif
	xil_printf("%d %d %d %d %d %d\n\r",dt3.Year,dt3.Month,dt3.Day,dt3.Hour,dt3.Min,dt3.Sec);

	DesiredTime = XRtcPsu_DateTimeToSec(&dt3);
	XRtcPsu_SetTime(&Rtc_Psu,DesiredTime);

	return XST_SUCCESS;
}
