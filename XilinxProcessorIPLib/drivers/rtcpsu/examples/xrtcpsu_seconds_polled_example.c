/******************************************************************************
* Copyright (C) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/****************************************************************************/
/**
*
* @file     xrtcpsu_seconds_polled_example.c
*
* This file contains an example using the XRtcPsu driver in polled mode.
*
* This function checks the new second reporting feature of the RTC.
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
#define REPETATIONS 10

/************************** Function Prototypes ******************************/

int RtcPsuSecondsPolledExample(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRtcPsu Rtc_Psu;		/* Instance of the RTC Device */

/*****************************************************************************/
/**
*
* Main function to call the Rtc Seconds Polled mode example.
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
	 * Run the Rtc_Psu polled example , specify the the Device ID that is
	 * generated in xparameters.h
	 */
	Status = RtcPsuSecondsPolledExample(RTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RTC Seconds Polled Mode Example Test Failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RTC Seconds Polled Mode Example Test\r\n");
	return XST_SUCCESS;
}
#endif

/*****************************************************************************/
/**
*
* This function does a minimal Seconds test on the XRtcPsu device in polled mode.
*
* This function checks the new second reporting feature of the RTC.
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
int RtcPsuSecondsPolledExample(u16 DeviceId)
{
	int Status;
	XRtcPsu_Config *Config;
	u32 Seconds;

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

	/*
	 * If seconds interrupt was not generated in ISR, then the processor goes
	 * into an infinite loop. This represents a failure case of seconds example.
	 */
	for (Seconds = 1;Seconds <= REPETATIONS; Seconds++) {
		while (!XRtcPsu_IsSecondsEventGenerated(&Rtc_Psu));
		xil_printf("Seconds value is %02d.\n\r",Seconds);
	}

	xil_printf("Seconds feature tested.\n\r");

	return XST_SUCCESS;
}
