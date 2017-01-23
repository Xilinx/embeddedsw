/******************************************************************************
*
* Copyright (C) 2005 - 2015 Xilinx, Inc.  All rights reserved.
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
/*****************************************************************************/
/**
* @file xgpio_tapp_example.c
*
* This file contains a example for using AXI GPIO hardware and driver.
* This example assumes that there is a UART Device or STDIO Device in the
* hardware system.
*
* @note
*
* None
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver	Who  Date	  Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a sv   04/15/05 Initial release for TestApp integration.
* 3.00a sv   11/21/09 Updated to use HAL Processor APIs.
* 3.01a bss  04/18/13 Removed incorrect Documentation lines.(CR #701641)
* 4.1   lks  11/18/15 Updated to use canonical xparameters and
*		      clean up of the comments and code for CR 900381
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/

#include "xparameters.h"
#include "xgpio.h"
#include "stdio.h"
#include "xstatus.h"
#include "xil_printf.h"

/************************** Constant Definitions ****************************/

/*
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye.  This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY	  1000000

/* The following constant is used to determine which channel of the GPIO is
 * used if there are 2 channels supported in the GPIO.
 */
#define LED_CHANNEL 1

#define LED_MAX_BLINK	0x1	/* Number of times the LED Blinks */

#define GPIO_BITWIDTH	16	/* This is the width of the GPIO */

#define printf xil_printf	/* A smaller footprint printf */

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#ifndef TESTAPP_GEN
#define GPIO_OUTPUT_DEVICE_ID	XPAR_GPIO_0_DEVICE_ID
#define GPIO_INPUT_DEVICE_ID	XPAR_GPIO_0_DEVICE_ID
#endif /* TESTAPP_GEN */

/**************************** Type Definitions ******************************/


/***************** Macros (Inline Functions) Definitions *******************/


/************************** Function Prototypes ****************************/

int GpioOutputExample(u16 DeviceId, u32 GpioWidth);

int GpioInputExample(u16 DeviceId, u32 *DataRead);

void GpioDriverHandler(void *CallBackRef);


/************************** Variable Definitions **************************/

/*
 * The following are declared globally so they are zeroed and so they are
 * easily accessible from a debugger
 */
XGpio GpioOutput; /* The driver instance for GPIO Device configured as O/P */
XGpio GpioInput;  /* The driver instance for GPIO Device configured as I/P */

/*****************************************************************************/
/**
* Main function to call the example. This function is not included if the
* example is generated from the Peripheral Tests in SDK.
*
* @param	None
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None
*
******************************************************************************/
#ifndef TESTAPP_GEN
int main(void)
{
	int Status;
	u32 InputData;

	Status = GpioOutputExample(GPIO_OUTPUT_DEVICE_ID, GPIO_BITWIDTH);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio tapp Example Failed\r\n");
		  return XST_FAILURE;
	}

	Status = GpioInputExample(GPIO_INPUT_DEVICE_ID, &InputData);
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio tapp Example Failed\r\n");
		  return XST_FAILURE;
	}

	printf("Data read from GPIO Input is  0x%x \n\r", (int)InputData);

	xil_printf("Successfully ran Gpio tapp Example\r\n");
	return XST_SUCCESS;
}
#endif


/*****************************************************************************/
/**
*
* This function does a minimal test on the GPIO device configured as OUTPUT
* and driver as a example.
*
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		xparameters.h
* @param	GpioWidth is the width of the GPIO
*
* @return
*		- XST_SUCCESS if successful
*		- XST_FAILURE if unsuccessful
*
* @note		None
*
****************************************************************************/
int GpioOutputExample(u16 DeviceId, u32 GpioWidth)
{
	volatile int Delay;
	u32 LedBit;
	u32 LedLoop;
	int Status;

	/*
	 * Initialize the GPIO driver so that it's ready to use,
	 * specify the device ID that is generated in xparameters.h
	 */
	 Status = XGpio_Initialize(&GpioOutput, DeviceId);
	 if (Status != XST_SUCCESS)  {
		  return XST_FAILURE;
	 }

	 /* Set the direction for all signals to be outputs */
	 XGpio_SetDataDirection(&GpioOutput, LED_CHANNEL, 0x0);

	 /* Set the GPIO outputs to low */
	 XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL, 0x0);

	 for (LedBit = 0x0; LedBit < GpioWidth; LedBit++)  {

		for (LedLoop = 0; LedLoop < LED_MAX_BLINK; LedLoop++) {

			/* Set the GPIO Output to High */
			XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL,
						1 << LedBit);

#ifndef __SIM__
			/* Wait a small amount of time so the LED is visible */
			for (Delay = 0; Delay < LED_DELAY; Delay++);

#endif
			/* Clear the GPIO Output */
			XGpio_DiscreteClear(&GpioOutput, LED_CHANNEL,
						1 << LedBit);


#ifndef __SIM__
			/* Wait a small amount of time so the LED is visible */
			for (Delay = 0; Delay < LED_DELAY; Delay++);
#endif

		  }

	 }

	 return XST_SUCCESS;

}


/******************************************************************************/
/**
*
* This function  performs a test on the GPIO driver/device with the GPIO
* configured as INPUT
*
* @param	DeviceId is the XPAR_<GPIO_instance>_DEVICE_ID value from
*		xparameters.h
* @param	DataRead is the pointer where the data read from GPIO Input is
*		returned
*
* @return
*		- XST_SUCCESS if the Test is successful
*		- XST_FAILURE if the test is not successful
*
* @note	  	None.
*
******************************************************************************/
int GpioInputExample(u16 DeviceId, u32 *DataRead)
{
	 int Status;

	 /*
	  * Initialize the GPIO driver so that it's ready to use,
	  * specify the device ID that is generated in xparameters.h
	  */
	 Status = XGpio_Initialize(&GpioInput, DeviceId);
	 if (Status != XST_SUCCESS) {
		  return XST_FAILURE;
	 }

	 /* Set the direction for all signals to be inputs */
	 XGpio_SetDataDirection(&GpioInput, LED_CHANNEL, 0xFFFFFFFF);

	 /* Read the state of the data so that it can be  verified */
	 *DataRead = XGpio_DiscreteRead(&GpioInput, LED_CHANNEL);

	 return XST_SUCCESS;

}

