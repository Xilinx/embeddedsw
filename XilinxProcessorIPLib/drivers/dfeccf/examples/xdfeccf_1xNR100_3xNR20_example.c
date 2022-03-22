/******************************************************************************
* Copyright (C) 2021-2022 Xilinx, Inc.  All rights reserved.
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
* 1.2   dc     11/01/21 Add multi AddCC, RemoveCC and UpdateCC
*       dc     11/05/21 Align event handlers
*       dc     11/19/21 Update doxygen documentation
*
* </pre>
* @addtogroup Overview_examples
* @{
*
*****************************************************************************/
/** @cond nocomments */
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

/** @endcond */
/****************************************************************************/
/**
*
* This example configures Channel Filter driver for one NR100 carrier and
* three NR20 and at the end close and release the driver.
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
/** //! [testexample2] */
int XDfeCcf_1xNR100_3xNR20_Example()
{
	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	XDfeCcf_Cfg Cfg;
	XDfeCcf *InstancePtr = NULL;
	XDfeCcf_TriggerCfg TriggerCfg = { 0 };
	u32 Shift = 8;
	XDfeCcf_Status Status;
	u32 Return;
	u32 Index;

	printf("\r\nChannel Filter \"1xNR100 and 3xNR20 Configuration\" "
	       "Example - Start\r\n");

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
	for (Index = 0; Index < NUM_COEFFICIENT; Index++) {
		XDfeCcf_LoadCoefficients(InstancePtr, Index, Shift,
					 pt_coef[Index]);
		printf("set coefficients set#%d done!\n\r", Index);
	}

	for (Index = 0; Index < NUM_CARRIER; Index++) {
		/* Clear event status */
		Status.OverflowCCID = XDFECCF_ISR_CLEAR;
		Status.CCUpdate = XDFECCF_ISR_CLEAR;
		Status.CCSequenceError = XDFECCF_ISR_CLEAR;
		printf("Clear Event Status\n\r");
		XDfeCcf_ClearEventStatus(InstancePtr, &Status);
		Return = XDfeCcf_AddCC(InstancePtr, CCID_Vals[Index],
				       bit_sequence[Index], pt_carr[Index]);

		if (Return == XST_SUCCESS) {
			printf("Add CC%d done!\n\r", Index);
		} else {
			printf("Add CC%d failed!\n\r", Index);
			printf("Channel Filter \"1xNR100 and 3xNR20 "
			       "Configuration\" Example: Fail\r\n");
			return XST_FAILURE;
		}
	}

	/* Shutdown the block */
	XDfeCcf_Deactivate(InstancePtr);
	XDfeCcf_InstanceClose(InstancePtr);

	printf("Channel Filter \"1xNR100 and 3xNR20 Configuration\" Example:"
	       " Pass\r\n");
	return XST_SUCCESS;
}
/** //! [testexample2] */
/** @} */
