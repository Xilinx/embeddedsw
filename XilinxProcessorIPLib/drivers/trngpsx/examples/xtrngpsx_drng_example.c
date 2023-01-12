/***************************************************************************************************
* Copyright (c) 2023 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2023 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 * @file  xtrngpsx_drng_example.c
 *
 * This example demonstrates use of instantiate, reseed, generate functionalities while generating
 * random number using DRNG mode and runs DRBG,Health tests for all the instances
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
 *
 *</pre>
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xparameters.h"
#include "xtrngpsx.h"
#include "xil_util.h"

/************************************ Constant Definitions ***************************************/

#define XTRNGPSX_EXAMPLE_SEEDLIFE			12U
#define XTRNGPSX_EXAMPLE_DFLENMUL			4U
#define XTRNGPSX_EXAMPLE_RESEED_DFLENMUL	3U
#define XTRNGPSX_ENTROPY_SIZE               80U
#define XTRNGPSX_RESEED_ENTROPY_SIZE        64U

const u8 InitEntropy[XTRNGPSX_ENTROPY_SIZE] = {
		0x3AU, 0xBBU, 0xABU, 0x42U, 0x7AU, 0x3AU, 0x57U, 0x63U,
		0x5EU, 0x7BU, 0x06U, 0x4EU, 0xD4U, 0x66U, 0xE6U, 0xBEU,
		0xEAU, 0x25U, 0xFAU, 0xA9U, 0xB2U, 0x41U, 0x43U, 0xBDU,
		0x65U, 0x48U, 0x8EU, 0xE8U, 0xE8U, 0xEDU, 0xBDU, 0x22U,
		0x48U, 0x05U, 0x8BU, 0xFCU, 0x82U, 0x50U, 0x7EU, 0x69U,
		0xA2U, 0xC7U, 0x93U, 0xC6U, 0x89U, 0x61U, 0xC2U, 0xA9U,
		0xACU, 0xBFU, 0x0FU, 0x0CU, 0x44U, 0x07U, 0x2FU, 0xC5U,
		0xB8U, 0x2DU, 0x92U, 0xF9U, 0xA3U, 0xFAU, 0x2BU, 0xF2U,
		0x6FU, 0xF8U, 0x15U, 0x52U, 0x82U, 0xCBU, 0xFCU, 0x7AU,
		0x1AU, 0xC4U, 0xD6U, 0x21U, 0x4FU, 0x21U, 0xBEU, 0x2AU
};

const u8 ReseedEntropy1[XTRNGPSX_RESEED_ENTROPY_SIZE] = {
		0x35U, 0xB0U, 0xADU, 0x67U, 0x43U, 0x18U, 0xD4U, 0xEBU,
		0x7EU, 0x2FU, 0xE8U, 0x6DU, 0x03U, 0xB6U, 0x02U, 0x36U,
		0x85U, 0xB0U, 0x50U, 0x32U, 0xEEU, 0xDEU, 0x03U, 0x57U,
		0x02U, 0x05U, 0x8CU, 0x2EU, 0xEFU, 0x59U, 0xBEU, 0xBBU,
		0x5CU, 0x0FU, 0xADU, 0x01U, 0x02U, 0xE6U, 0xF7U, 0x7DU,
		0x0FU, 0x98U, 0x2AU, 0x50U, 0x2CU, 0x8FU, 0x0AU, 0x90U,
		0x38U, 0x89U, 0x44U, 0xE0U, 0x0BU, 0x45U, 0x4DU, 0xE2U,
		0xB2U, 0xAFU, 0x4EU, 0x3BU, 0x96U, 0xE4U, 0x83U, 0xA8U
};

const u8 ReseedEntropy2[XTRNGPSX_RESEED_ENTROPY_SIZE] = {
		0x42U, 0x8FU, 0xAFU, 0x96U, 0x0FU, 0x92U, 0x7DU, 0xF9U,
		0x31U, 0x3EU, 0xB0U, 0xE1U, 0x07U, 0xFCU, 0xEAU, 0xF8U,
		0x41U, 0x31U, 0xE2U, 0x13U, 0xE0U, 0xFFU, 0xB2U, 0xF8U,
		0x9CU, 0x9AU, 0x8EU, 0xFFU, 0x49U, 0xB0U, 0xA8U, 0x52U,
		0xA9U, 0x9AU, 0xC2U, 0x73U, 0x59U, 0x03U, 0xDBU, 0x8EU,
		0xD2U, 0xEFU, 0x0BU, 0xF6U, 0xFFU, 0xCBU, 0x60U, 0x47U,
		0x2DU, 0xDCU, 0xAAU, 0x4CU, 0x4DU, 0xD4U, 0x8DU, 0x04U,
		0x03U, 0x09U, 0x9AU, 0xEAU, 0x21U, 0x50U, 0xE8U, 0x3DU
};

const u8 PersStr[XTRNGPSX_PERS_STRING_LEN_IN_BYTES] ={
		0xACU, 0xC6U, 0x33U, 0x51U, 0x92U, 0x0EU, 0x05U, 0xC3U,
		0xF9U, 0x16U, 0xA5U, 0xE6U, 0xDBU, 0x20U, 0xCAU, 0x5FU,
		0x4DU, 0x0CU, 0xF9U, 0x3AU, 0x51U, 0x5AU, 0x4BU, 0x21U,
		0xE7U, 0xC8U, 0xD5U, 0xD9U, 0xEBU, 0x87U, 0x06U, 0x87U,
		0x11U, 0x23U, 0xCDU, 0x54U, 0x90U, 0x85U, 0x8FU, 0x41U,
		0x1CU, 0xFAU, 0x31U, 0x24U, 0x16U, 0xC4U, 0x66U, 0x59U
};

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
	xil_printf("Successfully ran TRNGPSX DRNG example\n\r");

	return XST_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief
 * This function tests the functioning of the TRNG in DRNG mode.
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
			.Mode = XTRNGPSX_DRNG_MODE,
			.SeedLife = XTRNGPSX_EXAMPLE_SEEDLIFE,
			.DFLength = XTRNGPSX_EXAMPLE_DFLENMUL
	};

	for (u32 DeviceId = 0U; DeviceId < XPAR_XTRNGPSX_NUM_INSTANCES; DeviceId++) {
	   /*
		* Initialize the TRNGPSX driver so that it's ready to use look up
		* configuration in the config table, then initialize it.
		*/
		Config = XTrngpsx_LookupConfig(DeviceId);
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

		xil_printf("\n\r TRNGPSX example with instance:%d \n\r ", DeviceId);
		Status = XTrngpsx_PreOperationalSelfTests(&Trngpsx);
		if (Status != XST_SUCCESS) {
			xil_printf("KAT Failed, Status: 0x%08x\n\r", Status);
			goto END;
		}

		/* Instantiate to complete initialization */
		Status = XTrngpsx_Instantiate(&Trngpsx, InitEntropy, sizeof(InitEntropy), PersStr, &UsrCfg);
		if (Status != XST_SUCCESS) {
			xil_printf("Instantiate failed, Status: 0x%08x\n\r", Status);
			goto END;
		}

		Status = XTrngpsx_Reseed(&Trngpsx, ReseedEntropy1, XTRNGPSX_EXAMPLE_RESEED_DFLENMUL);
		if (Status != XST_SUCCESS) {
			xil_printf("Reseed failed, Status: 0x%08x\n\r", Status);
			goto END;
		}

		/* Invoke Generate with predication resistance as TRUE */
		Status = XTrngpsx_Generate(&Trngpsx, RandBuf, sizeof(RandBuf), TRUE);
		if (Status != XST_SUCCESS) {
			xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
			goto END;
		}

		xil_printf("Generate 1 Random data:\n\r");
		Trngpsx_PrintBytes(RandBuf, sizeof(RandBuf));

		Status = XTrngpsx_Reseed(&Trngpsx, ReseedEntropy2, XTRNGPSX_EXAMPLE_RESEED_DFLENMUL);
		if (Status != XST_SUCCESS) {
			xil_printf("Reseed failed, Status: 0x%08x\n\r", Status);
			goto END;
		}

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
