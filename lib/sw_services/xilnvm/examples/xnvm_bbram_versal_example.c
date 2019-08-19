/******************************************************************************
*
* Copyright (C) 2019 Xilinx, Inc.  All rights reserved.
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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
******************************************************************************/

/*****************************************************************************/
/**
* @file xnvm_bbram_versal_example.c
* @addtogroup xnvm_apis XilNvm APIs
* @{
* This file illustrates how to program AES key of Versal BBRAM. The key provided
* by XNVM_BBRAM_AES_KEY macro is written to the BBRAM
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who     Date     Changes
* ----- ------  -------- ------------------------------------------------------
* 1.0   mmd     06/06/19 First release
* </pre>
*
*
* User configurable parameters for BBRAM
* ------------------------------------------------------------------------------
* #define 		XNVM_BBRAM_AES_KEY
* "0000000000000000000000000000000000000000000000000000000000000000"
* User should replace zeros with 256/128 bit AES key in hexadecimal
* string format. Also should set the key length in bits to macro
* XNVM_BBRAM_AES_KEY_LEN_IN_BITS.
*
*
* #define 		XNVM_BBRAM_AES_KEY_LEN_IN_BITS		(256U)
* User should replace this value based on length of the AES Key defined
* by macro XNVM_BBRAM_AES_KEY. Supported values - 256 or 128
*
******************************************************************************/

/***************************** Include Files *********************************/

#include "xil_io.h"
#include "xil_util.h"
#include "xstatus.h"
#include "xnvm_bbram.h"


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

u8 AesKey[XNVM_BBRAM_AES_KEY_LEN_IN_BYTES];

/************************** Function Prototypes ******************************/
u32 BbramWriteAesKey(void);

/*****************************************************************************/
/**
*
* Main function to call the Bbram AES key Write example function.
*
* @param	None
*
* @return
*		- XST_FAILURE if the Test Failed .
*
* @note		None
*
******************************************************************************/
int main(void)
{
	u32 Status;

#if defined (XSK_ZYNQ_PLATFORM) || defined (XSK_MICROBLAZE_PLATFORM)
	xil_printf("This example will not work for this platform\n\r");
#endif

	xil_printf("BBRAM AES Key writing example for Versal\n\r");

	Status = BbramWriteAesKey();

	if (XST_SUCCESS == Status) {
		xil_printf("Key programmed successfully to BBRAM....\n\r");
	}
	else {
		xil_printf("BBRAM programming exit with error code = %08x\n\r",
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
*		- XST_SUCCESS if the Aes decryption was successful
*		- XST_FAILURE if the Aes decryption failed
*
* @note		None.
*
******************************************************************************/
/** //! [XNvm BBRAM example] */
u32 BbramWriteAesKey(void)
{
	u32 Status = XST_SUCCESS;

	/* Validate the key */
	Status = XNvm_ValidateAesKey((char *)XNVM_BBRAM_AES_KEY);
	if(Status != XST_SUCCESS) {
		goto END;
	}

	/* Convert key given in macro and assign it to the variable */
	Xil_ConvertStringToHexLE((char *)XNVM_BBRAM_AES_KEY,
				&(AesKey[0]), XNVM_BBRAM_AES_KEY_LEN_IN_BITS);

	/* Write AES key to BBRAM */
	Status = XNvm_BbramWriteAesKey(AesKey,
				       XNVM_BBRAM_AES_KEY_LEN_IN_BYTES);

END:
	return Status;
}

/** //! [XNvm BBRAM example] */
/** @} */
