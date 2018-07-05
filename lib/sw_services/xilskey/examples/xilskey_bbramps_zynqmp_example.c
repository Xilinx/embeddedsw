/******************************************************************************
* Copyright (c) 2015 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/


/*****************************************************************************/
/**
*
* @file xilskey_bbramps_zynqmp_example.c
* This file illustrates how to program AES key of ZynqMP BBRAM.
* Ideally example should be excited with status 0x00000000 if not
* there was some error in programming.
*
* @note: No need to include any header file in src of the application.
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 4.0   vns     10/08/15 First release
* 6.7   psl     04/10/19 Fixed IAR warnings.
* 7.0   kpt     09/02/20 Added successfully ran print to the example in case of
*                        Success
*
* </pre>
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xstatus.h"
#include "xilskey_utils.h"
#include "xilskey_bbram.h"

/***************** Macros (Inline Functions) Definitions *********************/

/*
 * This is the 256 bit key to be programmed into BBRAM.
 * This should entered by user in HEX.
 */
#define XSK_ZYNQMP_BBRAMPS_AES_KEY "0000000000000000000000000000000000000000000000000000000000000000"

#define XSK_ZYNQMP_BBRAMPS_AES_KEY_LEN_IN_BYTES		(32)
						/**< AES key length in Bytes */
#define XSK_ZYNQMP_BBRAMPS_AES_KEY_LEN_IN_BITS		(256)
						/**< AES key length in Bits */
#define XSK_ZYNQMP_BBRAMPS_AES_KEY_STR_LEN		(64)
						/**< String length of Key */

/**************************** Type Definitions *******************************/


/************************** Variable Definitions ****************************/

u8 AesKey[XSK_ZYNQMP_BBRAMPS_AES_KEY_LEN_IN_BYTES];

/************************** Function Prototypes ******************************/

/*****************************************************************************/

int main()
{
	u32 Status = XST_SUCCESS;

#if defined (XSK_ZYNQ_PLATFORM) || defined (XSK_MICROBLAZE_PLATFORM)
	xil_printf("This example will not work for this platform\n\r");
#endif

	xil_printf("BBRAM example for ZynqMP\n\r");

	/* Validate the key */
	Status = XilSKey_Efuse_ValidateKey(
					(char *)XSK_ZYNQMP_BBRAMPS_AES_KEY,
					XSK_ZYNQMP_BBRAMPS_AES_KEY_STR_LEN);
	if(Status != XST_SUCCESS) {
		goto END;
	}
	/* Convert key given in macro and assign it to the variable */
	XilSKey_Efuse_ConvertStringToHexLE((char *)XSK_ZYNQMP_BBRAMPS_AES_KEY,
				&(AesKey[0]),
				XSK_ZYNQMP_BBRAMPS_AES_KEY_LEN_IN_BITS);

	Status = XilSKey_ZynqMp_Bbram_Program((u32 *)AesKey);

END:
	if (Status != XST_SUCCESS) {
		xil_printf("ZynqMP BBRAM example failed with Status = %08x\n\r", Status);
	}
	else {
		xil_printf("Successfully ran ZynqMP BBRAM example...");
	}

	return Status;

}
