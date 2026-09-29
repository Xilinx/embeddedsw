/******************************************************************************
* Copyright (C) 2002 - 2021 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022 - 2026 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xgpio_example.c
* @brief Demonstrates usage of the AXI GPIO driver (XGpio) and hardware device.
*
* This file contains a design example using the AXI GPIO driver (XGpio) and
* hardware device. It only uses channel 1 of a GPIO device. It assumes that
* bit 0 of the GPIO is connected to the LED on the hardware board.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -----------------------------------------------
* 1.00a rmm  03/13/02 First release
* 1.00a rpm  08/04/03 Removed second example and invalid macro calls
* 2.00a jhl  12/15/03 Added support for dual channels
* 2.00a sv   04/20/05 Minor changes to comply to Doxygen and coding guidelines
* 3.00a ktn  11/20/09 Minor changes as per coding guidelines.
* 4.1   lks  11/18/15 Updated to use canonical xparameters and
*		      clean up of the comments and code for CR 900381
* 4.3   sk   09/29/16 Modified the example to make it work when LED_bits are
*                     configured as an output. CR# 958644
*       ms   01/23/17 Added xil_printf statement in main function to
*                     ensure that "Successfully ran" and "Failed" strings
*                     are available in all examples. This is a fix for
*                     CR-965028.
* 4.5  sne   06/12/19 Fixed IAR compiler warning.
* 4.10 gm    07/11/23 Added SDT support.
* 4.14  vn   09/10/26 Align Doxygen comments with AMD standards
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"

/************************** Constant Definitions *****************************/

#define LED 0x01   /**< Assumes bit 0 of GPIO is connected to an LED. */

/** @name Device parameters
 *
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here so that all the needed
 * parameters can be easily changed in one place.
 * @{
 */
#ifndef SDT
#define GPIO_EXAMPLE_DEVICE_ID  XPAR_GPIO_0_DEVICE_ID /**< Device ID. */
#else
#define	XGPIO_AXI_BASEADDRESS	XPAR_XGPIO_0_BASEADDR /**< Base address. */
#endif
/** @} */

/**
 * @brief LED visibility delay count.
 *
 * The following constant is used to wait after an LED is turned on to make
 * sure that it is visible to the human eye. This constant might need to be
 * tuned for faster or slower processor speeds.
 */
#define LED_DELAY     10000000

/**
 * @brief LED channel selection.
 *
 * The following constant is used to determine which channel of the GPIO is
 * used for the LED if there are two channels supported.
 */
#define LED_CHANNEL 1

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/

#ifdef PRE_2_00A_APPLICATION

/** @name Pre-2.00a compatibility macros
 *
 * The following macros allow an application written for an older version of
 * the driver (pre 2.00a) to compile. That version did not have a channel
 * parameter. The channel parameter is fixed as channel 1.
 * @{
 */

/*****************************************************************************/
/**
*
* @brief Sets the I/O direction of all discrete signals for the LED channel.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   DirectionMask Bitmask specifying which discretes are input
*              and which are output. Bits set to 0 are output and bits set
*              to 1 are input.
*
* @note        C-style signature:
*              void XGpio_SetDataDirection(XGpio *InstancePtr,
*                                          u32 DirectionMask)
*
******************************************************************************/
#define XGpio_SetDataDirection(InstancePtr, DirectionMask) \
	XGpio_SetDataDirection(InstancePtr, LED_CHANNEL, DirectionMask)

/*****************************************************************************/
/**
*
* @brief Reads the state of the discretes for the LED channel.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
*
* @return      Current copy of the discretes register.
*
* @note        C-style signature:
*              u32 XGpio_DiscreteRead(XGpio *InstancePtr)
*
******************************************************************************/
#define XGpio_DiscreteRead(InstancePtr) \
	XGpio_DiscreteRead(InstancePtr, LED_CHANNEL)

/*****************************************************************************/
/**
*
* @brief Writes to the discretes register for the LED channel.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   Mask Value to be written to the discretes register.
*
* @note        C-style signature:
*              void XGpio_DiscreteWrite(XGpio *InstancePtr, u32 Mask)
*
******************************************************************************/
#define XGpio_DiscreteWrite(InstancePtr, Mask) \
	XGpio_DiscreteWrite(InstancePtr, LED_CHANNEL, Mask)

/*****************************************************************************/
/**
*
* @brief Sets output discretes to logic 1 for the LED channel.
*
* @param[in]   InstancePtr Pointer to the XGpio instance to operate on.
* @param[in]   Mask Set of bits that are set to 1 in the discrete data
*              register. All other bits in the data register are
*              unaffected.
*
* @note        C-style signature:
*              void XGpio_DiscreteSet(XGpio *InstancePtr, u32 Mask)
*
******************************************************************************/
#define XGpio_DiscreteSet(InstancePtr, Mask) \
	XGpio_DiscreteSet(InstancePtr, LED_CHANNEL, Mask)
/** @} */

#endif

/************************** Function Prototypes ******************************/


/************************** Variable Definitions *****************************/

/**
 * @brief Holds the global GPIO driver instance.
 *
 * The instance is declared globally so it is zeroed and easily accessible
 * from a debugger.
 */
XGpio Gpio;

/*****************************************************************************/
/**
* @brief Demonstrates turning on and off an LED using the GPIO driver.
*
* @return      XST_FAILURE if GPIO initialization fails.
*
* @note        This function does not return if the test is running.
*
******************************************************************************/
int main(void)
{
	int Status;
	volatile int Delay;

	/* Initialize the GPIO driver */
#ifndef SDT
	Status = XGpio_Initialize(&Gpio, GPIO_EXAMPLE_DEVICE_ID);
#else
	Status = XGpio_Initialize(&Gpio, XGPIO_AXI_BASEADDRESS);
#endif
	if (Status != XST_SUCCESS) {
		xil_printf("Gpio Initialization Failed\r\n");
		return XST_FAILURE;
	}

	/* Set the direction for all signals as inputs except the LED output */
	XGpio_SetDataDirection(&Gpio, LED_CHANNEL, ~LED);

	/* Loop forever blinking the LED */

	while (1) {
		/* Set the LED to High */
		XGpio_DiscreteWrite(&Gpio, LED_CHANNEL, LED);

		/* Wait a small amount of time so the LED is visible */
		for (Delay = 0; Delay < LED_DELAY; Delay++);

		/* Clear the LED bit */
		XGpio_DiscreteClear(&Gpio, LED_CHANNEL, LED);

		/* Wait a small amount of time so the LED is visible */
		for (Delay = 0; Delay < LED_DELAY; Delay++);
	}

}
