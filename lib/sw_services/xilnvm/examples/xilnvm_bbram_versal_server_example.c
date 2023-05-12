/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
* @file xilnvm_bbram_versal_example.c
* @addtogroup xnvm_apis XilNvm APIs
* @{
* This file illustrates how to program AES key of Versal BBRAM. The key provided
* by XNVM_BBRAM_AES_KEY macro is written to the BBRAM
* This example is supported for Versal devices.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   mmd     06/06/19 First release
* 2.0   kal     09/30/19 Renamed example to xilnvm_bbram_versal_example
*       kal     01/03/20 Added "Successfully ran" golden string when success
*       kal     05/12/20 Adds Zeroization before every BBRAM write.
* 2.1	am	10/10/20 Changed function return type and type of
*			 status variable from u32 to int
*
* </pre>
*
*
* User configurable parameters for BBRAM
* ------------------------------------------------------------------------------
* #define 		XNVM_BBRAM_AES_KEY
* "0000000000000000000000000000000000000000000000000000000000000000"
* User should replace zeros with 256 bit AES key in hexadecimal
* string format. Also should set the key length in bits to macro
* XNVM_BBRAM_AES_KEY_LEN_IN_BITS.
*
*
* #define 		XNVM_BBRAM_AES_KEY_LEN_IN_BITS		(256U)
* User should replace this value based on length of the AES Key defined
* by macro XNVM_BBRAM_AES_KEY. Supported values - 256
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xil_util.h"
#include "xstatus.h"
#include "xnvm_bbram.h"
#include "xnvm_utils.h"

/***************** Macros (Inline Functions) Definitions *********************/

/* AES Key */
#define XNVM_BBRAM_AES_KEY \
             "0000000000000000000000000000000000000000000000000000000000000000"

#define XNVM_BBRAM_AES_KEY_LEN_IN_BITS	(256U)
					/**< AES key length in Bits */

/* Key Length of AES Key defined by XNVM_BBRAM_AES_KEY */
#define XNVM_BBRAM_AES_KEY_LEN_IN_BYTES	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 8U)
					/**< AES key length in Bytes */

#define XNVM_BBRAM_AES_KEY_STR_LEN	(XNVM_BBRAM_AES_KEY_LEN_IN_BITS / 4U)
					/**< String length of Key */

/**************************** Type Definitions *******************************/


/************************** Variable Definitions ****************************/

/************************** Function Prototypes ******************************/
static int BbramWriteAesKey(void);

/*****************************************************************************/
/**
*
* Main function to call the Bbram AES key write example function.
*
* @param	None
*
* @return
*		- XST_FAILURE if the BBRAM programming failed.
*
* @note		None
*
******************************************************************************/
int main(void)
{
	int Status = XST_FAILURE;

	xil_printf("BBRAM AES Key writing example for Versal\n\r");

	Status = BbramWriteAesKey();

	if (XST_SUCCESS == Status) {
		xil_printf("Successfully ran Versal BBRAM example....\n\r");
	}
	else {
		xil_printf("BBRAM programming failed with error code = %08x\n\r",
		          Status);
	}

	return 0;
}

/*****************************************************************************/
/**
* BbramWriteAesKey function writes AES key defined by macro XNVM_BBRAM_AES_KEY
* in BBRAM.
*
* @return
*		- XST_SUCCESS if the Aes key write successful
*		- XST_FAILURE if the Aes key write failed
*
* @note		None.
*
******************************************************************************/
/** //! [XNvm BBRAM example] */
static int BbramWriteAesKey(void)
{
	int Status = XST_SUCCESS;
	u8 AesKey[XNVM_BBRAM_AES_KEY_LEN_IN_BYTES];

	/* Validate the key */
	Status = XNvm_ValidateAesKey((char *)XNVM_BBRAM_AES_KEY);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/* Convert key given in macro and assign it to the variable */
	Xil_ConvertStringToHexLE((char *)XNVM_BBRAM_AES_KEY,
				AesKey, XNVM_BBRAM_AES_KEY_LEN_IN_BITS);

	/* Write AES key to BBRAM */
	Status = XNvm_BbramWriteAesKey(AesKey,
				       XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

END:
	return Status;
}

/** //! [XNvm BBRAM example] */
/** @} */
