/***************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 * @file  xtrngpsv_hrng_example.c
 *
 * This example demonstrates use of instantiate, reseed, generate
 * functionalities while generating random number using HRNG mode.
 *
 * @note
 *
 * None.
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- -----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 Minor updates related to security best practices
 *
 *</pre>
 **************************************************************************************************/

/***************************** Include Files *****************************************************/

#include "xparameters.h"
#include "xtrngpsv.h"

/************************** Constant Definitions *************************************************/

/*
 * The following constant map to the XPAR parameters created in the xparameters.h file. They are
 * only defined here such that a user can easily change all the needed parameters in one place.
 */
#define TRNGPSV_DEVICE_ID	XPAR_XTRNGPSV_0_DEVICE_ID

#define EXAMPLE_SEEDLIFE	3U

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

static int Trngpsv_Hrng_Example(u16 DeviceId);
static void Trngpsv_PrintBytes(u8 *Src, u32 Size);

/************************************ Variable Definitions ***************************************/

XTrngpsv Trngpsv; /* Instance of TRNGPSV */
u8 RandBuf[XTRNGPSV_SEC_STRENGTH_BYTES];

const u8 PersStr[XTRNGPSV_PERS_STR_LEN_BYTES] = {
		0xB2U, 0x80U, 0x7EU, 0x4CU, 0xD0U, 0xE4U, 0xE2U, 0xA9U,
		0x2FU, 0x1FU, 0x5DU, 0xC1U, 0xA2U, 0x1FU, 0x40U, 0xFCU,
		0x1FU, 0x24U, 0x5DU, 0x42U, 0x61U, 0x80U, 0xE6U, 0xE9U,
		0x71U, 0x05U, 0x17U, 0x5BU, 0xAFU, 0x70U, 0x30U, 0x18U,
		0xBCU, 0x23U, 0x18U, 0x15U, 0xCBU, 0xB8U, 0xA6U, 0x3EU,
		0x83U, 0xB8U, 0x4AU, 0xFEU, 0x38U, 0xFCU, 0x25U, 0x87U
};
/*************************************************************************************************/
/**
 * @brief
 * Main function to call the Trngpsv driver example.
 *
 * @return
 *		- XST_SUCCESS if example ran successfully.
 *		- XST_FAILURE if unsuccessful.
 *
 **************************************************************************************************/
int main(void)
{
	int Status;

	/* Call the example , specify the device ID that is generated in  xparameters.h. */
	xil_printf("********* TRNGPSV example in HRNG mode **********\n\r");
	Status = Trngpsv_Hrng_Example(TRNGPSV_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		xil_printf("TRNGPSV HRNG example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran TRNGPSV HRNG example\n\r");

	return XST_SUCCESS;
}

/***************************************************************************************************/
/**
 * @brief
 * This function tests the functioning of the TRNG in HRNG mode.
 * When used without DF, DFLenMul should be 0 and DFDisable to be TRUE.
 *
 * @param	DeviceId is the XPAR_<TRNG_instance>_DEVICE_ID value from xparameters.h.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- XST_FAILURE otherwise.
 *
 **************************************************************************************************/
int Trngpsv_Hrng_Example(u16 DeviceId)
{
	int Status = XST_SUCCESS;
	XTrngpsv_Config *Config;
	XTrngpsv_UsrCfg UsrCfg = {
			.Mode = XTRNGPSV_HRNG,
			.SeedLife = EXAMPLE_SEEDLIFE,
			.PredResistanceEn = XTRNGPSV_FALSE,
			.DFDisable = XTRNGPSV_TRUE,
			.DFLenMul = 0U,
			.InitSeedPresent =  XTRNGPSV_FALSE,
			.PersStrPresent = XTRNGPSV_TRUE
	};

	if (UsrCfg.PersStrPresent == XTRNGPSV_TRUE) {
		Status = Xil_SMemCpy(&UsrCfg.PersString, sizeof(UsrCfg.PersString), PersStr,
				sizeof(PersStr), sizeof(PersStr));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/*
	 * Initialize the TRNGPSV driver so that it's ready to use look up
	 * configuration in the config table, then initialize it.
	 */
	Config = XTrngpsv_LookupConfig(DeviceId);
	if (NULL == Config) {
		xil_printf("LookupConfig Failed \n\r");
		goto END;
	}

	/* Initialize the TRNGPSV driver so that it is ready to use. */
	Status = XTrngpsv_CfgInitialize(&Trngpsv, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("CfgInitialize Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	Status = XTrngpsv_RunKAT(&Trngpsv);
	if (Status != XST_SUCCESS) {
		if (Status == XTRNGPSV_ERROR_KAT_MISMATCH) {
			xil_printf("KAT Failed with mismatch error, Status: 0x%08x\n\r", Status);
		}
		else {
			xil_printf("KAT Failed, Status: 0x%08x\n\r", Status);
		}
		goto END;
	}

	Status = XTrngpsv_RunHealthTest(&Trngpsv);
	if (Status != XST_SUCCESS) {
		if ((Status == XTRNGPSV_ERROR_CERTF)
				|| (Status == XTRNGPSV_ERROR_CERTF_SW_A5_PATTERN)) {
			xil_printf("RunHealthTest Failed with CERTF related error, Status: 0x%08x\n\r", Status);
		}
		else {
			xil_printf("RunHealthTest Failed, Status: 0x%08x\n\r", Status);
		}
		goto END;
	}

	/* Instantiate to complete initialization and for (initial) reseeding */
	Status = XTrngpsv_Instantiate(&Trngpsv, &UsrCfg);
	if (Status != XST_SUCCESS) {
		xil_printf("Instantiate failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Reseed with DFLenMul of 0 */
	Status = XTrngpsv_Reseed(&Trngpsv, NULL, 0U);
	if (Status != XST_SUCCESS) {
		xil_printf("Instantiate failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Invoke Generate twice and print, RandBuf contains random data from last call */
	Status = XTrngpsv_Generate(&Trngpsv, RandBuf,
			sizeof(RandBuf), XTRNGPSV_FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	xil_printf("Generate 1 Random data:\n\r");
	Trngpsv_PrintBytes(RandBuf, sizeof(RandBuf));

	Status = XTrngpsv_Generate(&Trngpsv, RandBuf,
			sizeof(RandBuf), XTRNGPSV_FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	xil_printf("Generate 2 Random data:\n\r");
	Trngpsv_PrintBytes(RandBuf, sizeof(RandBuf));

	Status = XTrngpsv_Uninstantiate(&Trngpsv);
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
 *************************************************************************************************/
static void Trngpsv_PrintBytes(u8 *Src, u32 Size)
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
