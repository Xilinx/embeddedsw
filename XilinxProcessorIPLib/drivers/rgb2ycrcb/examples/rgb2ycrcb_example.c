/******************************************************************************
*
* Copyright (C) 2014 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/

/*****************************************************************************/
/**
*
* @file rgb2ycrcb_example.c
*
* This file demonstrates how to use Xilinx RGB to YCrCb Color Space Converter
* (RGB2YCRCB) driver on Xilinx RGB to YCrCb Color Space Converter (RGB2YCRCB)
* core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.00 adk   07/07/14 First release.
* </pre>
*
******************************************************************************/

#include "xrgb2ycrcb.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define XRGB_DEVICE_ID		XPAR_RGB2YCRCB_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int Rgb2YCrCb_Update_Example(u16 DeviceId);

/************************** Variable Definitions *****************************/

XRgb2YCrCb	XRgbInstance;	/**< Instance of the RGB2YCRCB core */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* Main function to call the Rgb2YCrCb example.
*
* @param	None.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if it is unsuccessful
*
* @note		None.
*
******************************************************************************/
int main(void)
{

	int Status;

	/* Call the Rgb2YCrCb update example, specify the parameters generated
	 * in xparameters.h
	 */
	Status = Rgb2YCrCb_Update_Example(XRGB_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("RGB2YCRCB driver example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran RGB2YCRCB driver example.\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* RGB to YCrCb Color Space Converter Register Update Example.
* This function provides an example of the process used to update the
* coefficient and offset registers in the RGB2YCrCb core. .
*
* @param	DeviceId is the unique device id of the RGB2YCRCB core.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if it is un-successful
*
* @note		None.
*
****************************************************************************/
int Rgb2YCrCb_Update_Example(u16 DeviceId)
{

	struct XRgb2YCrCb_Coef_Inputs CoefIn;
	struct XRgb2YCrCb_Coef_Outputs CoefOut;
	XRgb2YCrCb_Config *Config;
	int Status;

	/* Initialize the RGB2YCrCb driver so that it's ready to use look up
	 * the configuration in the config table, then initialize it.
	 */
	Config = XRgb2YCrCb_LookupConfig(DeviceId);
	if(NULL == Config){
		return XST_FAILURE;
	}

	Status = XRgb2YCrCb_CfgInitialize(&XRgbInstance, Config,
						Config->BaseAddress);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/* Enable the RGB2YCRCB core */
	XRgb2YCrCb_Enable(&XRgbInstance);

	/* Disable register updates. This is the default operating mode for
	 * the RGB2YCRCB core and allows registers to be updated without
	 * effecting the core's behavior.
	 */
	XRgb2YCrCb_RegUpdateDisable(&XRgbInstance);

	/* Setup CoefIn for XRGB_STANDARD_ITU_601_SD, 16_to_240_for_TV
	 * and data width of 8-bits.
	 * enum Standards are:
	 *	0 = XRGB_STANDARD_ITU_601_SD
	 *	1 = XRGB_STANDARD_ITU_709_NTSC
	 *	2 = XRGB_STANDARD_ITU_709_PAL
	 *	3 = XRGB_STANDARD_YUV.
	 */
	XRgb2YCrCb_Select_Standard(&XRgbInstance, XRGB_STANDARD_ITU_601_SD,
				XRGB_TV_16_TO_240, (u32)XRGB_DATA_WIDTH_8,
					&CoefIn);

	/* Translate into RGB2YCrCb core coefficients */
	XRgb2YCrCb_Coefficient_Translation(&XRgbInstance,
				&CoefIn, &CoefOut,(u32)XRGB_DATA_WIDTH_8);

	/* Program the new RGB2YCrCb core coefficients */
	XRgb2YCrCb_SetCoefs(&XRgbInstance, CoefIn.ACoef, CoefIn.BCoef,
				CoefIn.CCoef, CoefIn.DCoef);

	/* Set output range */
	XRgb2YCrCb_Select_OutputRange(&XRgbInstance, XRGB_TV_16_TO_240);

	/* Set the Active Columns and Rows */
	XRgb2YCrCb_SetActiveSize(&XRgbInstance, 720, 1280);

	/* Set the offsets
	 * For 8-bit color:  Valid range = [0, 255]
	 * For 10-bit color: Valid range = [0, 1023]
	 * For 12-bit color: Valid range = [0, 4095]
	 * For 16-bit color: Valid range = [0, 65535]
	 */
	XRgb2YCrCb_SetYOffset(&XRgbInstance, 16);
	XRgb2YCrCb_SetCbOffset(&XRgbInstance, 128);
	XRgb2YCrCb_SetCrOffset(&XRgbInstance, 128);

	/* Set the Clip/Clamp
	 * For 8-bit color:  Valid range = [0,   255]
	 * For 10-bit color: Valid range = [0,  1023]
	 * For 12-bit color: Valid range = [0,  4095]
	 * For 16-bit color: Valid range = [0, 65535]
	 */
	XRgb2YCrCb_SetYMax(&XRgbInstance, 240);
	XRgb2YCrCb_SetYMin(&XRgbInstance, 16);
	XRgb2YCrCb_SetCbMax(&XRgbInstance, 240);
	XRgb2YCrCb_SetCbMin(&XRgbInstance, 16);
	XRgb2YCrCb_SetCrMax(&XRgbInstance, 240);
	XRgb2YCrCb_SetCrMin(&XRgbInstance, 16);

	/* Enable register updates. This mode will cause the coefficient and
	 * offset registers internally to the RGB2YCRCB core to automatically
	 * be updated on the next start-of-frame (SOF).
	 */
	XRgb2YCrCb_RegUpdateEnable(&XRgbInstance);

	return XST_SUCCESS;
}
