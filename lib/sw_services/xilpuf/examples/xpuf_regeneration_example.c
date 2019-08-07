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
 * @file xpuf_regeneration_example.c
 *
 * This file illustrates PUF regeneration.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   -----------------------------------------------------
 * 1.0	 ka   01/08/2019   Initial realease of Puf_regeneration example
 *
 * @note
 *
 *
 *
 * User configurable parameters for PUF
 *------------------------------------------------------------------------------
 *	#define 	XPUF_READ_OPTION		(XPUF_READ_FROM_RAM)
 *								(or)
 *							(XPUF_READ_FROM_CACHE)
 *								(or)
 *							(XPUF_READ_FROM_EFUSE)
 *	XPUF_READ_OPTION can be any value among above three options
 *	based on where syndrome data is stored.
 *
 *	#define 	XPUF_RAM_CHASH			(0x00000000)
 *	In case of external memory PUF regeneration, CHASH value should be
 *	supplied along with the SYN_DATA_ADDRESS.
 *
 *	#define 	XPUF_RAM_AUX			(0x00000000)
 *	In case of external memory PUF regeneration, AUX value should be
 *	supplied along with the SYN_DATA_ADDRESS.
 *
 *	#define 	XPUF_SYN_DATA_ADDRESS		(0x00000000)
 *	This is the address from where ROM will take the syndrome data
 *	to regenerate the PUF.
 *
 ******************************************************************************/
/***************************** Include Files *********************************/

#include "xpuf.h"


/* Configurable parameters */
#define XPUF_READ_OPTION			(XPUF_READ_FROM_RAM)
#define XPUF_RAM_CHASH				(0x00000000)
#define XPUF_RAM_AUX				(0x00000000)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000)
#define XPUF_DEBUG_INFO				(1U)

/************************** Type Definitions **********************************/

static XPuf_Data PufData;

/************************** Function Definitions *****************************/

int main()
{
	u32 Status = XST_FAILURE;

	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.ReadOption = XPUF_READ_OPTION;
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;

	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_RAM_CHASH;
		PufData.Aux = XPUF_RAM_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
	}

	Status = XPuf_Puf_Regeneration(&PufData);

	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,
			"Puf Regeneration example failed with error : %x\r\n",
			 Status);
		goto END;
	}
	xPuf_printf(XPUF_DEBUG_INFO,
		"Puf Regeneration example run successfully!!\r\n");

END:
	return Status;
}
