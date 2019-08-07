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
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMANGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*
*
*
******************************************************************************/
/*****************************************************************************/
/**
 *
 * @file xpuf_registration_example.c
 *
 * This file illustrates PUF registration.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date     Changes
 * ----- ---  -------- -------------------------------------------------------
 * 1.0	kal   01/08/2019 Initial release of Puf_registration example
 *
 * @note
 *
 ******************************************************************************/

/***************************** Include Files *********************************/

#include "xpuf.h"
#include "xil_util.h"

/************************** Constant Definitions ****************************/

#define XPUF_4K_SYNDROME_MODE			(0U)
#define XPUF_12K_SYNDROME_MODE			(1U)
#define XPUF_DEBUG_INFO					(1U)

static XPuf_Data PufData;

/************************** Type Definitions ********************************/

/* Error codes */
typedef enum {
	XPUF_PARAMETER_NULL_ERROR = (0x2001U),
	XPUF_STRING_INVALID_ERROR = (0x2002U),
}XPufErrocodes;

/************************** Function Prototypes ******************************/

/************************** Function Definitions *****************************/

int main()
{
	u32 Status = XST_FAILURE;
	u32 Subindex;
	u8 *Buffer;
	u32 SynIndex;
	u32 Idx;

	PufData.RegMode = XPUF_4K_SYNDROME_MODE;
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	Status = XPuf_Puf_Registration(&PufData);

	if (Status == XPUF_ERROR_SYNDROME_DATA_OVERFLOW) {
		xPuf_printf(XPUF_DEBUG_INFO,
		"PUF Registration completed with an overflow:0x%08x\r\n",
		Status);
		goto ENDF;
	}
	else if ((Status != XST_SUCCESS)) {
		xPuf_printf(XPUF_DEBUG_INFO,
		"PUF Registration Failed:0x%08x\r\n", Status);
		goto ENDF;
	} else {
		xPuf_printf(XPUF_DEBUG_INFO,
		"PUF Registration Completed:0x%08x\r\n", Status);
	}

	xPuf_printf(XPUF_DEBUG_INFO, "PUF Syndrome data Start!!!\r\n");

	for (SynIndex = 0; SynIndex < XPUF_4K_PUF_SYN_LEN_IN_WORDS; SynIndex++) {

		Buffer = (u8*) &(PufData.SyndromeData[SynIndex]);
		for (Subindex = 0; Subindex < 4; Subindex++) {
			xPuf_printf(XPUF_DEBUG_INFO,"%02x", Buffer[Subindex]);
		}
	}
	xPuf_printf(XPUF_DEBUG_INFO, "\r\n");
	xPuf_printf(XPUF_DEBUG_INFO, "PUF Syndrome data End!!!\r\n");
	xPuf_printf(XPUF_DEBUG_INFO, "AUX-%08x\r\n", PufData.Aux);
	xPuf_printf(XPUF_DEBUG_INFO, "CHASH -%08x\r\n", PufData.Chash);

	xPuf_printf(XPUF_DEBUG_INFO,"PUF ID : ");
	for (Idx = 0; Idx < 8; Idx++) {
		xPuf_printf(XPUF_DEBUG_INFO, "%02x", PufData.PufID[Idx]);
	}
	xPuf_printf(XPUF_DEBUG_INFO, "\r\n");

	xPuf_printf(XPUF_DEBUG_INFO,
		"Puf Registration example run successfully!!");

ENDF :
	return Status;
}
