/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_low_level_example.c
* @brief Demonstrates usage of the GPIO low-level driver and hardware device.
*
* This file contains a design example using the General Purpose I/O (GPIO) low
* level driver and hardware device. It only uses channel 1 of a GPIO device.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date	 Changes
* ----- ---- -------- -----------------------------------------------
* 1.00b jhl  04/29/02 First release
* 2.00a jhl  12/30/02 Addition of dual channel capability
* 2.00a sv   04/18/05 Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  11/21/09 Removed the macros XGpio_mSetDataDirection,
*		      XGpio_mGetDataReg and XGpio_mSetDataReg. Users
*		      should use XGpio_WriteReg/XGpio_ReadReg to achieve the
*		      same functionality.
* 4.1   lks  11/18/15 Updated to use canonical xparameters and
*		      clean up of the comments and code for CR 900381
* 4.3   ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.5   sne  06/12/19 Fixed IAR compiler warning.
* 4.10  gm   07/11/23 Added SDT support.
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpio_l.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

#define LED 0x01 /**< Assumes bit 0 of GPIO is connected to an LED. */

/** @name Device parameters
 *
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here so that all the needed
 * parameters can be easily changed in one place.
 * @{
 */
#ifndef SDT
#define GPIO_REG_BASEADDR	XPAR_GPIO_0_BASEADDR /**< GPIO base address. */
#else
/** GPIO base address. */
#define GPIO_REG_BASEADDR	XPAR_XGPIO_0_BASEADDR
#endif
/** @} */

/**
 * @brief LED visibility delay count.
 *
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye. This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY	 1000000

/**
 * @brief LED channel selection.
 *
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are two channels supported.
 */
#define LED_CHANNEL	1

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/


/*****************************************************************************/
/**
* @brief Demonstrates turning on and off an LED using the GPIO low-level driver.
*
* @return      Always 0.
*
* @note        The main function returns an integer to prevent compiler
*              warnings.
*
******************************************************************************/
int main(void)
{
	u32 Data;
	volatile int Delay;

	/*
	 * Set the direction for all signals to be inputs except the LED output
	 */
	XGpio_WriteReg((GPIO_REG_BASEADDR),
		       ((LED_CHANNEL - 1) * XGPIO_CHAN_OFFSET) +
		       XGPIO_TRI_OFFSET, (~LED));


	/* Loop forever blinking the LED */

	while (1) {
		/*
		 * Read the state of the data so that only the LED state can be
		 * modified
		 */
		Data = XGpio_ReadReg(GPIO_REG_BASEADDR,
				     ((LED_CHANNEL - 1) * XGPIO_CHAN_OFFSET) +
				     XGPIO_DATA_OFFSET);


		/* Set the LED to the opposite state such that it blinks */

		if (Data & LED) {

			XGpio_WriteReg((GPIO_REG_BASEADDR),
				       ((LED_CHANNEL - 1) * XGPIO_CHAN_OFFSET) +
				       XGPIO_DATA_OFFSET, Data & ~LED);

		} else {

			XGpio_WriteReg((GPIO_REG_BASEADDR),
				       ((LED_CHANNEL - 1) * XGPIO_CHAN_OFFSET) +
				       XGPIO_DATA_OFFSET, Data | LED);
		}

		/* Wait a small amount of time so that the LED is visible */

		for (Delay = 0; Delay < LED_DELAY; Delay++);
	}

}

