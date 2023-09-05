/***************************************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 * @file  xtrngpsx_ptrng_example.c
 *
 * This example demonstrates use of instantiate, generate functionalities while generating
 * random number using PTRNG mode and runs DRBG,Health tests for all the instances
 * i.e XTrngpsx_PreOperationalSelfTests.
 *
 * @note
 *
 * None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------
 * 1.00  kpt  01/10/23 First release
 *       yog  08/04/23 Removed support for PKI instances
 * 1.01  ng   09/04/23 Added SDT support
 *
 *</pre>
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xparameters.h"
#include "xtrngpsx.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/
#ifndef SDT
#define XTRNGPSX_PMC_DEVICE		0U /**< Device Id for PMC*/
#else
#define XTRNGPSX_PMC_DEVICE		XPAR_XTRNGPSX_0_BASEADDR /**< Device Id for PMC*/
#endif

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int Trngpsx_Example();
static void Trngpsx_PrintBytes(u8 *Src, u32 Size);

/************************************ Variable Definitions ***************************************/

XTrngpsx_Instance Trngpsx; /* Instance of TRNGPSX */
u8 RandBuf[XTRNGPSX_SEC_STRENGTH_IN_BYTES];

/*************************************************************************************************/
/**
 * @brief
 * Main function to call the Trngpsx driver example.
 *
 * @return
 *		- XST_SUCCESS if example ran successfully.
 *		- XST_FAILURE if unsuccessful.
 *
 **************************************************************************************************/
int main(void)
{
	int Status;

	xil_printf("****** TRNGPSX example *******\n\r");
	Status = Trngpsx_Example();
	if (Status != XST_SUCCESS) {
		xil_printf("TRNGPSX example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran TRNGPSX PTRNG example\n\r");

	return XST_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief
 * This function tests the functioning of the TRNG in PTRNG mode.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- XST_FAILURE otherwise.
 *
 ************************************************************************************************/
int Trngpsx_Example()
{
	int Status = XST_SUCCESS;
	XTrngpsx_Config *Config;
	XTrngpsx_UserConfig UsrCfg = {
			.Mode = XTRNGPSX_PTRNG_MODE,
			.SeedLife = XTRNGPSX_USER_CFG_SEED_LIFE,
			.DFLength = XTRNGPSX_USER_CFG_DF_LENGTH,
			.AdaptPropTestCutoff = XTRNGPSX_USER_CFG_ADAPT_TEST_CUTOFF,
			.RepCountTestCutoff = XTRNGPSX_USER_CFG_REP_TEST_CUTOFF,
	};

	/*
	 * Initialize the TRNGPSX driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	Config = XTrngpsx_LookupConfig(XTRNGPSX_PMC_DEVICE);
	if (NULL == Config) {
		xil_printf("LookupConfig Failed \n\r");
		goto END;
	}

	/* Initialize the TRNGPSX driver so that it is ready to use. */
	Status = XTrngpsx_CfgInitialize(&Trngpsx, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("CfgInitialize Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	Status = XTrngpsx_PreOperationalSelfTests(&Trngpsx);
	if (Status != XST_SUCCESS) {
		xil_printf("KAT Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Instantiate to complete initialization */
	Status = XTrngpsx_Instantiate(&Trngpsx, NULL, 0U, NULL, &UsrCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Instantiate failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Invoke Generate twice and print, RandBuf contains random data from last call */
	Status = XTrngpsx_Generate(&Trngpsx, RandBuf, sizeof(RandBuf), FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	xil_printf("Generate 1 Random data:\n\r");
	Trngpsx_PrintBytes(RandBuf, sizeof(RandBuf));

	Status = XTrngpsx_Generate(&Trngpsx, RandBuf, sizeof(RandBuf), FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	xil_printf("Generate 2 Random data:\n\r");
	Trngpsx_PrintBytes(RandBuf, sizeof(RandBuf));

	Status = XTrngpsx_Uninstantiate(&Trngpsx);
	if (Status != XST_SUCCESS) {
		xil_printf("Uninstantiate Failed \n\r");
		goto END;
	}

	Status = XST_SUCCESS;
END:
	return Status;
}

/*************************************************************************************************/
/**
 * @brief
 * This function prints specified number of bytes from an address.
 *
 * @param	src is start address from where to print.
 *
 * @param	size is size of buffer pointed by src.
 *
 * @return	None.
 *
 ************************************************************************************************/
static void Trngpsx_PrintBytes(u8 *Src, u32 Size)
{
	u32 Index;

	for (Index = 0; Index < Size; Index++) {
		if (Index % 16 == 0) {
			xil_printf("\n\r");
		}
		xil_printf("0x%02X, ", Src[Index]);
	}
	xil_printf("\n\r");
}
