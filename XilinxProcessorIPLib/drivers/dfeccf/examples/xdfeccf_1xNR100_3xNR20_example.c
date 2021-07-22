/******************************************************************************
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_1xNR100_3xNR20_example.c
*
* This file contains an example which sets Channel Filter driver for one NR100
* carrier and three NR20.
*
* <pre>
*
* MODIFICATION HISTORY:
*
* Ver   Who    Date     Changes
* ----- -----  -------- -----------------------------------------------------
* 1.1   dc     07/21/21 Add and reorganise examples
*
* </pre>
*
*****************************************************************************/

/***************************** Include Files ********************************/
#include "xdfeccf_examples.h"

/************************** Constant Definitions ****************************/
/**************************** Type Definitions ******************************/
/***************** Macros (Inline Functions) Definitions ********************/
/************************** Function Prototypes *****************************/
/************************** Variable Definitions ****************************/
#define Num_Carr 4
#define Num_coef 2
XDfeCcf_Init Init = { { 8, { 0, 1, 2, 3 } }, 0 };

XDfeCcf_Coefficients Coeffs0 = {
	231,
	1,
	{ -7,   -2,    3,    -5,    5,    -4,   2,    2,    -5,   6,     -5,
	  1,    4,     -8,   9,     -7,   2,    5,    -11,  13,   -10,   2,
	  8,    -16,   18,   -13,   1,    12,   -22,  24,   -16,  1,     16,
	  -29,  30,    -20,  0,     22,   -37,  39,   -24,  -2,   30,    -48,
	  48,   -28,   -5,   40,    -61,  59,   -33,  -9,   51,   -76,   72,
	  -38,  -15,   66,   -95,   87,   -43,  -22,  84,   -117, 105,   -49,
	  -32,  107,   -144, 125,   -54,  -46,  135,  -176, 149,  -59,   -63,
	  170,  -216,  178,  -64,   -87,  216,  -267, 214,  -68,  -120,  276,
	  -334, 261,   -72,  -166,  361,  -428, 325,  -75,  -237, 490,   -571,
	  425,  -78,   -356, 711,   -825, 607,  -80,  -604, 1198, -1427, 1079,
	  -81,  -1462, 3275, -4981, 6195, 26133 }
};

XDfeCcf_Coefficients Coeffs1 = {
	120,
	1,
	{ -2,   28,   -3,   -13, 17,    2,    -24,  18,    15,   -35,
	  11,   33,   -42,  -5,  55,    -40,  -33,  75,    -23,  -70,
	  85,   12,   -110, 76,  66,    -142, 40,   132,   -155, -26,
	  201,  -134, -122, 255, -68,   -238, 273,  51,    -357, 233,
	  223,  -454, 114,  437, -495,  -106, 677,  -439,  -450, 917,
	  -223, -967, 1130, 285, -1823, 1290, 1626, -4081, 1375, 17331 }
};

XDfeCcf_CarrierCfg CarrierCfg0 = { 1, 0, 0, 1, 0, 0, 0 };
XDfeCcf_CarrierCfg CarrierCfg1 = { 1, 0, 1, 1, 0, 1, 1 };
XDfeCcf_CarrierCfg CarrierCfg2 = { 1, 0, 2, 1, 0, 1, 1 };
XDfeCcf_CarrierCfg CarrierCfg3 = { 1, 0, 3, 1, 0, 1, 1 };

XDfeCcf_Coefficients *pt_coef[2] = { &Coeffs0, &Coeffs1 };
XDfeCcf_CarrierCfg *pt_carr[4] = { &CarrierCfg0, &CarrierCfg1, &CarrierCfg2,
				   &CarrierCfg3 };
const u32 bit_sequence[4] = { 0x55, 0x2, 0x8, 0x20 };
const s32 CCID_Vals[4] = { 0, 1, 2, 3 };

/****************************************************************************/
/**
*
* This function configures Channel Filter driver for one NR100 carrier and
* three NR20.
* The example does the following:
*     - initialize driver from reset to activation
*     - configure filters with the pre-calculated coefficients
*     - add 4 carriers CCID={0,1,2,3}, 1xNR100(122.88Msps) and 3xNR20(30.72Msps)
*     - at this point filters are set and ready to operate
*     - close the driver
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfeCcf_1xNR100_3xNR20_Example()
{
	/* Configure the Channel Filter for a 'pass-through' */
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_TriggerCfg TriggerCfg = { 0 };
	u32 Shift = 8;

	printf("\r\nChannel Filter \"1xNR100 and 3xNR20 Configuration\" Example - Start\r\n");

	/* Initialize libmetal */
	if (0 != metal_init(&init_param)) {
		(void)printf("ERROR: Failed to run metal initialization\n\r");
		return XST_FAILURE;
	}
	metal_set_log_level(METAL_LOG_WARNING);

	/* Initialize the instance of channel filter driver */
	InstancePtr = XDfeCcf_InstanceInit(XDFECCF_NODE1_NAME);

	/* Go through initialization states of the state machine */
	XDfeCcf_Reset(InstancePtr);
	printf("Reset \n\r");
	XDfeCcf_Configure(InstancePtr, &Cfg);
	printf("Load configuration\n\r");
	XDfeCcf_Initialize(InstancePtr, &Init);
	printf("Initialize\n\r");
	XDfeCcf_SetTriggersCfg(InstancePtr, &TriggerCfg);
	printf("Set Triggers\n\r");
	XDfeCcf_Activate(InstancePtr, false);
	printf("Activate\n\r");

	/* Set coefficients and add channel */
	for (int i = 0; i < Num_coef; i++) {
		XDfeCcf_LoadCoefficients(InstancePtr, i, Shift, pt_coef[i]);
		printf("set coefficients set#%d done!\n\r", i);
	}

	for (int i = 0; i < Num_Carr; i++) {
		u32 status;
		printf("Clear Event Status \n\r");
		XDfeCcf_ClearEventStatus(InstancePtr);
		status = XDfeCcf_AddCC(InstancePtr, CCID_Vals[i],
				       bit_sequence[i], pt_carr[i]);

		if (status == XST_SUCCESS) {
			printf("Add CC%d done!\n\r", i);
		}
	}

	/* Shutdown the block */
	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);

	printf("Channel Filter \"1xNR100 and 3xNR20 Configuration\" Example: Pass\r\n");
	return XST_SUCCESS;
}
