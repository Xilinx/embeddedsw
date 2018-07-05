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
*
* @file ycrcb2rgb_example.c
*
* This file demonstrates how to use Xilinx YCrCb to RGB Color Space Converter
* (YCrCb2RGB) driver on Xilinx YCrCb to RGB Color Space Converter (YCrCb2RGB)
* core.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 7.00 adk   07/14/14 First release.
* </pre>
*
******************************************************************************/

#include "xycrcb2rgb.h"
#include "xparameters.h"

/************************** Constant Definitions *****************************/

/**
* The following constants map to the XPAR parameters created in the
* xparameters.h file. They are defined here such that a user can easily
* change all the needed parameters in one place.
*/
#define XYCC_DEVICE_ID		XPAR_YCRCB2RGB_0_DEVICE_ID

/**************************** Type Definitions *******************************/


/***************** Macros (Inline Functions) Definitions *********************/


/************************** Function Prototypes ******************************/

int YCrCb2Rgb_Update_Example(u16 DeviceId);

/************************** Variable Definitions *****************************/

XYCrCb2Rgb	XYccInstance;	/**< Instance of the YCRCB2RGB core */

/************************** Function Definitions *****************************/


/*****************************************************************************/
/**
*
* Main function to call the YCrCb2RGB example.
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

	/* Call the YCrCb2RGB update example, specify the parameters generated
	 * in xparameters.h
	 */
	Status = YCrCb2Rgb_Update_Example(XYCC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("YCrCb2RGB driver example failed.\n\r");
		return XST_FAILURE;
	}

	xil_printf("Successfully ran YCrCb2RGB driver example.\n\r");

	return XST_SUCCESS;
}

/*****************************************************************************/
/**
*
* YCrCb to RGB Color Space Converter Register Update Example.
* This function provides an example of the process used to update the
* coefficient and offset registers in the YCRCB2RGB core.
*
* @param	DeviceId is the unique device id of the YCRCB2RGB core.
*
* @return
*		- XST_SUCCESS if successful,
*		- XST_FAILURE if it is un-successful
*
* @note		None.
*
****************************************************************************/
int YCrCb2Rgb_Update_Example(u16 DeviceId)
{

	struct XYCrCb2Rgb_Coef_Inputs CoefIn;
	struct XYCrCb2Rgb_Coef_Outputs CoefOut;
	struct XYCrCb2Rgb_Coefficients Coef;
	XYCrCb2Rgb_Config *Config;
	int Status;

	/* Initialize the CrCb2RGB driver so that it's ready to use look up
	 * the configuration in the config table then initialize it.
	 */
	Config = XYCrCb2Rgb_LookupConfig(DeviceId);
	if(NULL == Config){
		return XST_FAILURE;
	}

	Status = XYCrCb2Rgb_CfgInitialize(&XYccInstance, Config,
						Config->BaseAddress);
	if(Status != XST_SUCCESS){
		return XST_FAILURE;
	}

	/* Enable the YCRCB2RGB core */
	XYCrCb2Rgb_Enable(&XYccInstance);

	/* Disable register updates. This is the default operating mode for
	 * the YCRCB2RGB core and allows registers to be updated without
	 * effecting the core's behavior.
	 */
	XYCrCb2Rgb_RegUpdateDisable(&XYccInstance);

	/* Setup CoefIn for XYCC_STANDARD_ITU_601_SD, 16_to_240_for_TV
	 * and data width of 8-bits.
	 * enum XYcc_Standards are:
	 *	0 = XYCC_STANDARD_ITU_601_SD
	 *	1 = XYCC_STANDARD_ITU_709_NTSC
	 *	2 = XYCC_STANDARD_ITU_709_PAL
	 *	3 = XYCC_STANDARD_YUV.
	 */
	XYCrCb2Rgb_Select_Standard(&XYccInstance, XYCC_STANDARD_ITU_601_SD,
				XYCC_TV_16_TO_240, (u32)XYCC_DATA_WIDTH_8,
					&CoefIn);

	/* Translate into YCRCB2RGB core coefficients */
	XYCrCb2Rgb_Coefficient_Translation(&XYccInstance,
				&CoefIn, &CoefOut,(u32)XYCC_DATA_WIDTH_8,
				(u32)XYCC_DATA_WIDTH_8);

	/* Store coefficient values */
	Coef.ACoef = CoefIn.ACoef;
	Coef.ACoef = CoefIn.ACoef;
	Coef.ACoef = CoefIn.ACoef;
	Coef.ACoef = CoefIn.ACoef;

	/* Program the new YCRCB2RGB core coefficients */
	XYCrCb2Rgb_SetCoefs(&XYccInstance, &Coef);

	/* Set output range */
	XYCrCb2Rgb_Select_OutputRange(&XYccInstance, XYCC_TV_16_TO_240);

	/* Set the Active Columns and Rows */
	XYCrCb2Rgb_SetActiveSize(&XYccInstance, 720, 1280);

	/* Set the offsets
	 * For 8-bit color:  Valid range = [0, 255]
	 * For 10-bit color: Valid range = [0, 1023]
	 * For 12-bit color: Valid range = [0, 4095]
	 * For 16-bit color: Valid range = [0, 65535]
	 */
	XYCrCb2Rgb_SetROffset(&XYccInstance, 16);
	XYCrCb2Rgb_SetGOffset(&XYccInstance, 128);
	XYCrCb2Rgb_SetBOffset(&XYccInstance, 128);

	/* Set the Clip/Clamp
	 * For 8-bit color:  Valid range = [0,   255]
	 * For 10-bit color: Valid range = [0,  1023]
	 * For 12-bit color: Valid range = [0,  4095]
	 * For 16-bit color: Valid range = [0, 65535]
	 */
	XYCrCb2Rgb_SetRGBMax(&XYccInstance, 240);
	XYCrCb2Rgb_SetRGBMin(&XYccInstance, 16);

	/* Enable register updates. This mode will cause the coefficient and
	 * offset registers internally to the YCRCB2RGB core to automatically
	 * be updated on the next start-of-frame (SOF).
	 */
	XYCrCb2Rgb_RegUpdateEnable(&XYccInstance);

	return XST_SUCCESS;
}
