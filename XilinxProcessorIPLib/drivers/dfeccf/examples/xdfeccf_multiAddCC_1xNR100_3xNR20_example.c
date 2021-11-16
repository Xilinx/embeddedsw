/******************************************************************************
 * Copyright (C) 2021 Xilinx, Inc.  All rights reserved.
 * SPDX-License-Identifier: MIT
 ******************************************************************************/

/*****************************************************************************/
/**
*
* @file xdfeccf_multiAddCC_1xNR100_3xNR20_example.c
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
* 1.2   dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
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
#define NUM_CARRIER 4
#define NUM_COEFFICIENT 2
static XDfeCcf_Init Init = {
	{
		8, /* [1-16] Sequence length. */
		{ 0, 1, 2, 3 } /* [0-15] CCID sequence */
	},
	0 /* [0,1] Enable gain stage */
};

static XDfeCcf_Coefficients Coeffs0 = {
	231, /* [0-(128|256)] Number of coefficients */
	1, /* [0,1] Select symetric (1) or non-symetric (0) filter */

	/* [Signed real numbers] Array of coefficients, when symmetric only
	   the first (Num+1)/2 coefficients are provided */
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

static XDfeCcf_Coefficients Coeffs1 = {
	120,
	1,
	{ -2,   28,   -3,   -13, 17,    2,    -24,  18,    15,   -35,
	  11,   33,   -42,  -5,  55,    -40,  -33,  75,    -23,  -70,
	  85,   12,   -110, 76,  66,    -142, 40,   132,   -155, -26,
	  201,  -134, -122, 255, -68,   -238, 273,  51,    -357, 233,
	  223,  -454, 114,  437, -495,  -106, 677,  -439,  -450, 917,
	  -223, -967, 1130, 285, -1823, 1290, 1626, -4081, 1375, 17331 }
};

static XDfeCcf_CarrierCfg CarrierCfg0 = {
	0, /* [0-(1<<16)-1] Gain setting for this CC */
	0, /* [0-7] Coefficient set for the complex data on this CC */
	0 /* [0-7] Coefficient set for the real data on this CC */
};
static XDfeCcf_CarrierCfg CarrierCfg1 = { 0, 1, 1 };
static XDfeCcf_CarrierCfg CarrierCfg2 = { 0, 1, 1 };
static XDfeCcf_CarrierCfg CarrierCfg3 = { 0, 1, 1 };

static XDfeCcf_Coefficients *pt_coef[2] = { &Coeffs0, &Coeffs1 };
static XDfeCcf_CarrierCfg *pt_carr[4] = { &CarrierCfg0, &CarrierCfg1,
					  &CarrierCfg2, &CarrierCfg3 };
static const u32 bit_sequence[4] = { 0x55, 0x2, 0x8, 0x20 };
static const s32 CCID_Vals[4] = { 0, 1, 2, 3 };

/****************************************************************************/
/**
*
* This function configures Channel Filter driver for one NR100 carrier and
* three NR20 whit multiAddCC API.
* The example does the following:
*     - initialize driver from reset to activation
*     - configure filters with the pre-calculated coefficients
*     - Get current configuration.
*     - add 4 carriers CCID={0,1,2,3}, 1xNR100(122.88Msps) and 3xNR20(30.72Msps)
*     - Trigger configuration update.
*     - at this point filters are set and ready to operate
*     - close the driver
*
* @return
*		- XST_SUCCESS if the example has completed successfully.
*		- XST_FAILURE if the example has failed.
*
****************************************************************************/
int XDfeCcf_multiAddCC_1xNR100_3xNR20_Example()
{
	/* Configure the Channel Filter for a 'pass-through' */
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf_CCCfg CCCfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_TriggerCfg TriggerCfg = { 0 };
	u32 Shift = 8;
	u32 Status;

	printf("\r\nChannel Filter \"1xNR100 and 3xNR20 Configuration\" with"
	       "multiAddCC Example - Start\r\n");

	/* Initialize libmetal */
	if (XST_SUCCESS != metal_init(&init_param)) {
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
	for (int i = 0; i < NUM_COEFFICIENT; i++) {
		XDfeCcf_LoadCoefficients(InstancePtr, i, Shift, pt_coef[i]);
		printf("set coefficients set#%d done!\n\r", i);
	}

	printf("Get current carrier configuration\n\r");
	XDfeCcf_GetCurrentCCCfg(InstancePtr, &CCCfg);

	printf("Clear Event Status\n\r");
	XDfeCcf_ClearEventStatus(InstancePtr);

	for (int i = 0; i < NUM_CARRIER; i++) {
		Status = XDfeCcf_AddCCtoCCCfg(InstancePtr, &CCCfg, CCID_Vals[i],
					      bit_sequence[i], pt_carr[i]);

		if (Status == XST_SUCCESS) {
			printf("Add CC%d done!\n\r", i);
		} else {
			printf("Add CC%d failed!\n\r", i);
		}
	}

	printf("Set next registers and trigger upload\n\r");
	Status = XDfeCcf_SetNextCCCfgAndTrigger(InstancePtr, &CCCfg);
	if (Status == XST_SUCCESS) {
		printf("XDfeCcf_SetNextCCCfgAndTrigger done!\n\r");
	} else {
		printf("XDfeCcf_SetNextCCCfgAndTrigger failed!\n\r");
	}

	/* Shutdown the block */
	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);

	printf("Channel Filter \"multi AddCC 1xNR100 and 3xNR20 Configuration"
	       "\" Example: Pass\r\n");
	return XST_SUCCESS;
}
