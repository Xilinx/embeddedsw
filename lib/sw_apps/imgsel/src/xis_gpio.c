/******************************************************************************
* Copyright (c) 2020 Xilinx, Inc. All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
* @file xis_gpio.c
*
* This is the main file which will contain the gpio initialization
*
*
* @note
*
* None.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  		Date     Changes
* ----- ---- -------- ---------------------------------------------------------
* 1.00  Ana	   10/11/20 	First release
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/
#include "xis_main.h"

#if defined(XIS_UPDATE_A_B_MECHANISM) && defined(XPAR_XGPIOPS_NUM_INSTANCES)
/************************** Constant Definitions *****************************/
/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define GPIO_DEVICE_ID		XPAR_XGPIOPS_0_DEVICE_ID

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/

/************************** Variable Definitions *****************************/
XGpioPs Gpio;	/* The driver instance for GPIO Device. */

/************************** Function Definitions *****************************/
/*****************************************************************************/
/**
 * This function is used to initialize the gpio controller and driver
 *
 * @param	None
 *
 * @return	XST_SUCCESS if pass, otherwise XST_FAILURE.
 *
 *****************************************************************************/
int GpioInit(void)
{
	int Status = XST_FAILURE;
	XGpioPs_Config *ConfigPtr;

	/* Initialize the GPIO driver. */
	ConfigPtr = XGpioPs_LookupConfig(GPIO_DEVICE_ID);
	if (ConfigPtr == NULL) {
		Status = XIS_GPIO_LKP_CONFIG_ERROR;
		goto END;
	}

	Status = XGpioPs_CfgInitialize(&Gpio, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		Status = XIS_GPIO_CONFIG_ERROR;
		goto END;
	}

END:
    return Status;
}

/******************************************************************************/
/**
*
* This function read data from GPIO driver/device with the GPIO
* configured as INPUT.
*
* @param	None
*
* @return	DataRead  where the data read from GPIO Input is returned
*
*
******************************************************************************/
u8 GetGpioStatus(void)
{
	u8 DataRead;

	/* Set the direction for the specified pin to be input. */
	XGpioPs_SetDirectionPin(&Gpio, FW_UPDATE_BUTTON, 0x0U);

	/* Read the state of the data so that it can be  verified. */
	DataRead = XGpioPs_ReadPin(&Gpio, FW_UPDATE_BUTTON);

	return DataRead;
}

#endif /* end of XIS_UPDATE_A_B_MECHANISM */