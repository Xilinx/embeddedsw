/***************************************************************************************************
* Copyright (C) 2021 - 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2024 Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
***************************************************************************************************/

/*************************************************************************************************/
/**
 * @file  xtrngpsv_drng_df_example.c
 *
 * This example demonstrates use of instantiate, reseed, generate functionalities while generating
 * random number using DRNG mode (with Derivative Function).
 *
 * MODIFICATION HISTORY:
 *
 *<pre>
 * Ver   Who  Date     Changes
 * ----- ---- -------- ----------------------------------------------------------------------------
 * 1.00  ssc  09/05/21 First release
 * 1.1   ssc  03/24/22 Minor updates related to security best practices
 * 1.2   ssc  08/25/22 Minor fix in print message
 * 1.3   ng   06/30/23 Added support for system device-tree flow
 * 1.4	 ss	  03/12/24 Minor fix to include xparameters header file
 *</pre>
 **************************************************************************************************/

/*************************************** Include Files *******************************************/

#include "xparameters.h"
#include "xtrngpsv.h"

/************************************ Constant Definitions ***************************************/

/*
 * The following constant map to the XPAR parameters created in the xparameters.h file. They are
 * only defined here such that a user can easily change all the needed parameters in one place.
 */
#ifndef SDT
#define TRNGPSV_DEVICE	XPAR_XTRNGPSV_0_DEVICE_ID
#else
#define TRNGPSV_DEVICE	XPAR_XTRNGPSV_0_BASEADDR
#endif

#define EXAMPLE_SEEDLIFE	10U
#define EXAMPLE_DFLENMUL	2U

/************************************** Type Definitions *****************************************/

/*************************** Macros (Inline Functions) Definitions *******************************/

/************************************ Function Prototypes ****************************************/

#ifndef SDT
static int Trngpsv_Drng_DF_Example(u16 DeviceId);
#else
static int Trngpsv_Drng_DF_Example(UINTPTR BaseAddr);
#endif
static void Trngpsv_PrintBytes(u8 *Src, u32 Size);

/************************************ Variable Definitions ***************************************/

XTrngpsv Trngpsv; /* Instance of TRNGPSV */
u8 RandBuf[XTRNGPSV_SEC_STRENGTH_BYTES];

/* Initial entropy contains 256bit seed + 128bit nonce */
const u8 InitEntropy[XTRNGPSV_SEED_LEN_BYTES] = {
		0xE4U, 0xBCU, 0x23U, 0xC5U, 0x08U, 0x9AU, 0x19U, 0xD8U,
		0x6FU, 0x41U, 0x19U, 0xCBU, 0x3FU, 0xA0U, 0x8CU, 0x0AU,
		0x49U, 0x91U, 0xE0U, 0xA1U, 0xDEU, 0xF1U, 0x7EU, 0x10U,
		0x1EU, 0x4CU, 0x14U, 0xD9U, 0xC3U, 0x23U, 0x46U, 0x0AU,
		0x7CU, 0x2FU, 0xB5U, 0x8EU, 0x0BU, 0x08U, 0x6CU, 0x6CU,
		0x57U, 0xB5U, 0x5FU, 0x56U, 0xCAU, 0xE2U, 0x5BU, 0xADU
};

const u8 ReseedEntropy[XTRNGPSV_SEED_LEN_BYTES] = {
		0xFDU, 0x85U, 0xA8U, 0x36U, 0xBBU, 0xA8U, 0x50U, 0x19U,
		0x88U, 0x1EU, 0x8CU, 0x6bU, 0xADU, 0x23U, 0xC9U, 0x06U,
		0x1AU, 0xDCU, 0x75U, 0x47U, 0x76U, 0x59U, 0xACU, 0xAEU,
		0xA8U, 0xE4U, 0xA0U, 0x1DU, 0xFEU, 0x07U, 0xA1U, 0x83U,
		0x2DU, 0xADU, 0x1CU, 0x13U, 0x6FU, 0x59U, 0xD7U, 0x0FU,
		0x86U, 0x53U, 0xA5U, 0xDCU, 0x11U, 0x86U, 0x63U, 0xD6U
};

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

	/* Call the example , specify the device ID that is generated in xparameters.h. */
	xil_printf("******* TRNGPSV example in DRNG (with DF) mode ******\n\r");
	Status = Trngpsv_Drng_DF_Example(TRNGPSV_DEVICE);
	if (Status != XST_SUCCESS) {
		xil_printf("TRNGPSV DRNG (DF) example failed\n\r");
		return XST_FAILURE;
	}
	xil_printf("Successfully ran TRNGPSV DRNG (DF) example\n\r");

	return XST_SUCCESS;
}

/*************************************************************************************************/
/**
 * @brief
 * This function tests the functioning of the TRNG in DRNG mode (with DF)
 *
 * @param	DeviceId is the XPAR_<TRNG_instance>_DEVICE_ID value from xparameters.h.
 *
 * @return
 *		- XST_SUCCESS if example runs successfully.
 *		- XST_FAILURE otherwise.
 *
 ************************************************************************************************/
#ifndef SDT
int Trngpsv_Drng_DF_Example(u16 DeviceId)
#else
int Trngpsv_Drng_DF_Example(UINTPTR BaseAddr)
#endif
{
	int Status = XST_SUCCESS;
	XTrngpsv_Config *Config;
	u32 DFLenMul = EXAMPLE_DFLENMUL;
	XTrngpsv_UsrCfg UsrCfg = {
			.Mode = XTRNGPSV_DRNG,
			.SeedLife = EXAMPLE_SEEDLIFE,
			.PredResistanceEn = XTRNGPSV_FALSE,
			.DFDisable = XTRNGPSV_FALSE,
			.DFLenMul = EXAMPLE_DFLENMUL,
			.InitSeedPresent = XTRNGPSV_TRUE,
			.PersStrPresent = XTRNGPSV_TRUE
	};

	Status = Xil_SMemCpy(&UsrCfg.InitSeed, (UsrCfg.DFLenMul + 1U) * BYTES_PER_BLOCK,
			InitEntropy, sizeof(InitEntropy), sizeof(InitEntropy));
	if (Status != XST_SUCCESS) {
		goto END;
	}

	if (UsrCfg.PersStrPresent == XTRNGPSV_TRUE) {
		Status = Xil_SMemCpy(&UsrCfg.PersString, sizeof(UsrCfg.PersString), PersStr,
				sizeof(PersStr), sizeof(PersStr));
		if (Status != XST_SUCCESS) {
			goto END;
		}
	}

	/*
	 * Initialize the TRNGPSV driver so that it's ready to use look up configuration in the
	 * config table, then initialize it.
	 */
#ifndef SDT
	Config = XTrngpsv_LookupConfig(DeviceId);
#else
	Config = XTrngpsv_LookupConfig(BaseAddr);
#endif
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
			xil_printf("RunHealthTest Failed with CERTF related error, Status: 0x%08x\n\r",
					Status);
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

	/* Reseed with new entropy */
	Status = XTrngpsv_Reseed(&Trngpsv, ReseedEntropy, DFLenMul);
	if (Status != XST_SUCCESS) {
		xil_printf("Reseed failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	/* Invoke Generate twice and print, RandBuf contains random data from last call */
	Status = XTrngpsv_Generate(&Trngpsv, RandBuf, sizeof(RandBuf), XTRNGPSV_FALSE);
	if (Status != XST_SUCCESS) {
		xil_printf("Generate Failed, Status: 0x%08x\n\r", Status);
		goto END;
	}

	xil_printf("Generate 1 Random data:\n\r");
	Trngpsv_PrintBytes(RandBuf, sizeof(RandBuf));

	Status = XTrngpsv_Generate(&Trngpsv, RandBuf, XTRNGPSV_SEC_STRENGTH_BYTES, XTRNGPSV_FALSE);
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
 * This function prints specified number of bytes from an address
 *
 * @param	src is start address from where to print
 *
 * @param	size is size of buffer pointed by src
 *
 * @return	None.
 *
 *************************************************************************************************/
static void Trngpsv_PrintBytes(u8 *Src, u32 Size)
{
	u32 Index;

	for (Index = 0U; Index < Size; Index++) {
		if (Index % 16U == 0U) {
			xil_printf("\n\r");
		}
		xil_printf("0x%02X, ", Src[Index]);
	}
	xil_printf("\n\r");
}
