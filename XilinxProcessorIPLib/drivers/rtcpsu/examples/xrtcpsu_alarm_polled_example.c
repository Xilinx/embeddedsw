/******************************************************************************
*
* Copyright (C) 2015 Xilinx, Inc.  All rights reserved.
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
* @file     xrtcpsu_alarm_polled_example.c
*
* This file contains an example using the XRtcPsu driver in polled mode.
*
* This function sets alarm for a specified time from the current time.
*
* @note
* If the device does not work properly, the example may hang.
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 1.00  kvn 05/12/15 First Release
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"	/* SDK generated parameters */
#include "xrtcpsu.h"		/* RTCPSU device driver */
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define RTC_DEVICE_ID              XPAR_XRTCPSU_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/
#define ALARM_PERIOD 10U

/************************** Function Prototypes ******************************/

int RtcPsuAlarmPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRtcPsu Rtc_Psu;		/* Instance of the RTC Device */

/*****************************************************************************/
/**
*
* Main function to call the Rtc Alarm Polled mode example.
*
* @param	None
*
* @return	XST_SUCCESS if succesful, otherwise XST_FAILURE
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;

	/*
	 * Run the Rtc_Psu polled example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = RtcPsuAlarmPolledExample(RTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Alarm Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Alarm Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal Alarm test on the XRtcPsu device in polled mode.
*
* This function sets one time alarm from the current time to a specified time.
*
* @param	DeviceId is the unique device id from hardware build.
*
* @return	XST_SUCCESS if successful, XST_FAILURE if unsuccessful
*
* @note
* This function polls the RTC, it may hang if the hardware is not
* working correctly.
*
****************************************************************************/
int RtcPsuAlarmPolledExample(u16 DeviceId)
{
	int Status;
	XRtcPsu_Config *Config;
	u32 CurrentTime, AlarmTime;
	XRtcPsu_DT dt0;

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

	xil_printf("\n\rDay Convention : 0-Fri, 1-Sat, 2-Sun, 3-Mon, 4-Tue, 5-Wed, 6-Thur\n\r");
	xil_printf("Current RTC time is..\n\r");
	CurrentTime = XRtcPsu_GetCurrentTime(&Rtc_Psu);
	XRtcPsu_SecToDateTime(CurrentTime,&dt0);
	xil_printf("YEAR:MM:DD HR:MM:SS \t %04d:%02d:%02d %02d:%02d:%02d\t Day = %d\n\r",
			dt0.Year,dt0.Month,dt0.Day,dt0.Hour,dt0.Min,dt0.Sec,dt0.WeekDay);

	CurrentTime = XRtcPsu_GetCurrentTime(&Rtc_Psu);
	AlarmTime = CurrentTime + ALARM_PERIOD;
	XRtcPsu_SetAlarm(&Rtc_Psu,AlarmTime,0U);

	/*
	 * If Alarm was not generated, then the processor goes into an infinite
	 * loop. This represents a failure case of alarm example.
	 */
	while (!XRtcPsu_IsAlarmEventGenerated(&Rtc_Psu));
	xil_printf("Alarm generated.\n\r");

	return XST_SUCCESS;
}
