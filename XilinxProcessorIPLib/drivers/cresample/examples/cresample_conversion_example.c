/******************************************************************************
* Copyright (C)2014 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
*
* @file cresample_conversion_example.c
*
* This file contains an example how CRESAMPLE type conversions will be done
* on the device.
*
* @note
*
* None
*
* MODIFICATION HISTORY:
* <pre>
* Ver   Who    Date     Changes
* ----- ------ -------- -----------------------------------------------
* 4.0   adk    03/12/14 First release.
*
* </pre>
******************************************************************************/

/***************************** Include Files *********************************/

#include "xcresample.h"
#include "xparameters.h"
#include "xil_printf.h"
#include "stdio.h"


/************************** Constant Definitions *****************************/

/*
 * The following constants map to the XPAR parameters created in the
 * xparameters.h file. They are defined here such that a user can easily
 * change all the needed parameters in one place.
 */
#define CRESAMPLE_DEVICE_ID		XPAR_CRESAMPLE_0_DEVICE_ID
					/**< CRESAMPLE Device ID */

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

/************************** Function Prototypes ******************************/
int CresampleTypeConversion(u16 DeviceId);

/************************** Variable Definitions *****************************/


XCresample Cresample;		/**<Instance of the Cresample core */

/*****************************************************************************/
/**
*
* Main function to call the example.
*
* @return
*		 	- XST_SUCCESS if successful.
*			- XST_FAILURE if failed.
*
* @note		None.
*
******************************************************************************/
int main(void)
{
	int Status;

	/* Run the Conversion example */
	Status = CresampleTypeConversion(CRESAMPLE_DEVICE_ID);
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		xil_printf("Cresample Coversion Example is Failed\r\n");
		return (XST_FAILURE);
	}
	xil_printf("Successfully ran Cresample Coversion Example\r\n");
	return (XST_SUCCESS);
}

/*****************************************************************************/
/**
*
* This function is to select the type of conversion from menu based on the
* selection conversion will take place.
*
* @param	Cresample is a pointer to the XCresample instance to be worked
*		on.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/

int CresampleTypeConversion(u16 DeviceId)
{

	XHorizontal_Coeffs CoefH =
		{.HCoeff = {{-2,-1,-1.23,0,0.1234,-1.78,0,0.9,0.8,0.0123,0,-1,
			     -1.23,-1.567,-1,0,1.527,1,1.9,0,-1,0,1.89,-2},
		      {-1,-1,0,0,1,0,1,-1,0,1,0,1,0,1,-1,1,-1,1,0,1,0,1,1,1}}};
	XVertical_Coeffs CoefV = {.VCoeff = {{-2,1.1,-1,0,0.1,-2,-1.98,1.98},
			   {0,1,0,1,-2,1,0,1} }};
	u32 HPhases;
	u32 VPhases;
	int Status;
	XCresample_Config *Config;

	/*
	 * Initialize the CRESAMPLE driver so that it's ready to use
	 * Look up the configuration in the config table,
	 * then initialize it.
	 */
	Config = XCresample_LookupConfig(DeviceId);

	/* Checking Config variable */
	if (NULL == Config) {
		return (XST_FAILURE);
	}

	Status = XCresample_CfgInitialize(&Cresample, Config,
						Config->BaseAddress);
	/* Checking status */
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}

	/* For 4:4:4 to 4:2:2 conversion */
	/*
	 * Horizontal filter phase 0 coefficients needs to be set.
	 * NumHTaps should be configured to odd numbers
	 * (3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23).
	 * API used is XCresample_SetHCoefs.
	 */
	if ((Cresample.Config.NumHTaps % 2) != 0) {
	HPhases = 0;
	/* Clear all Horizontal and Vertical coefficients before setting */
	XCresample_Clear_VCoef_Values(&Cresample);
	XCresample_SetHCoefs(&Cresample, &CoefH, HPhases);
	}

	/* For 4:2:2 to 4:4:4 conversion */
	/*
	 * Horizontal filter phase 1 coefficients needs to be set.
	 * NumHTaps should be configured to even numbers
	 * (2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24).
	 * API used is XCresample_SetHCoefs.
	 */
	if ((Cresample.Config.NumHTaps % 2) == 0) {
	HPhases = 1;
	/* Clear all Horizontal and Vertical coefficients before setting */
	XCresample_Clear_VCoef_Values(&Cresample);
	XCresample_SetHCoefs(&Cresample, &CoefH, HPhases);
	}

	/* For 4:2:2 to 4:2:0 conversion */
	/*
	 * Vertical filter phase 0 coefficients needs to be set.
	 * NumVTaps should be configured to even numbers(2, 4, 6, 8).
	 * API used is XCresample_SetVCoefs.
	 */
	if ((Cresample.Config.NumVTaps % 2) == 0) {
	VPhases = 0;
	/* Clear all Horizontal and Vertical coefficients before setting */
	XCresample_Clear_HCoef_Values(&Cresample);
	XCresample_SetVCoefs(&Cresample, &CoefV, VPhases);
	}

	/* For 4:2:0 to 4:2:2 conversion */
	/*
	 * Vertical filter phase 1 coefficients needs to be set.
	 * NumVTaps should be configured to even numbers(2, 4, 6, 8).
	 * API used is XCresample_SetVCoefs.
	 */
	if ((Cresample.Config.NumVTaps % 2) == 0) {
	VPhases = 0;
	/* Clear all Horizontal and Vertical coefficients before setting */
	XCresample_Clear_HCoef_Values(&Cresample);
	XCresample_SetVCoefs(&Cresample, &CoefV, VPhases);
	}

	/* For 4:4:4 to 4:2:0 conversion */
	/*
	 * Vertical filter phase 0 and Horizontal filter phase 0
	 * coefficients needs to be set.
	 * NumVTaps should be configured to even numbers(2, 4, 6, 8).
	 * NumHTaps should be configured to odd numbers
	 * (3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23).
	 * APIs used are XCresample_SetHCoefs and XCresample_SetVCoefs.
	 */
	if (((Cresample.Config.NumVTaps % 2) == 0) &&
			((Cresample.Config.NumHTaps % 2) != 0)) {
	VPhases = 0;
	HPhases = 0;
	XCresample_SetHCoefs(&Cresample, &CoefH, HPhases);
	XCresample_SetVCoefs(&Cresample, &CoefV, VPhases);
	}

	/* For 4:2:0 to 4:4:4 conversion */
	/*
	 * Vertical filter phase 0 and 1 and Horizontal filter phase 1
	 * coefficients needs to be set.
	 * NumVTaps should be configured to even numbers(2, 4, 6, 8).
	 * NumHTaps should be configured to even numbers
	 * (2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24).
	 * APIs used are XCresample_SetHCoefs and XCresample_SetVCoefs.
	 */
	if (((Cresample.Config.NumVTaps % 2) == 0) &&
			((Cresample.Config.NumHTaps % 2) == 0)) {
	VPhases = 2;
	HPhases = 1;
	XCresample_SetHCoefs(&Cresample, &CoefH, HPhases);
	XCresample_SetVCoefs(&Cresample, &CoefV, VPhases);
	}

	return (XST_SUCCESS);

}
