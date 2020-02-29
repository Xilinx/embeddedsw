/******************************************************************************
*
* Copyright (C) 2019 - 2020 Xilinx, Inc.  All rights reserved.
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
 * @file xilpuf_regeneration_example.c
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
 *	#define XPUF_READ_OPTION	(XPUF_READ_FROM_RAM)
 *						(or)
 *					(XPUF_READ_FROM_EFUSE_CACHE)
 *	This selects the location from where the helper data must be read by the
 *	application. This option must be configured if XPUF_KEK_GENERATE_OPTION
 *	is configured as XPUF_REGEN_ON_DEMAND.
 *
 *	#define XPUF_RAM_CHASH		(0x00000000)
 *	CHASH value should be supplied if XPUF_READ_OPTION is configured as
 *	XPUF_READ_FROM_RAM
 *
 *	#define XPUF_RAM_AUX		(0x00000000)
 *	AUX value should be supplied if XPUF_READ_OPTION is configured as
 *	XPUF_READ_FROM_RAM
 *
 *	#define XPUF_SYN_DATA_ADDRES	(0x00000000)
 *	Address of syndrome data should be supplied if XPUF_READ_OPTION is
 *	configured as XPUF_READ_FROM_RAM.
 ******************************************************************************/
/***************************** Include Files *********************************/

#include "xpuf.h"


/* Configurable parameters */
#define XPUF_REGEN_OPTION			(XPUF_REGEN_ON_DEMAND)
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
	int Idx;
	u32 Status = XST_FAILURE;

	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.ReadOption = XPUF_READ_OPTION;
	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.PufOperation = XPUF_REGEN_OPTION;

	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_RAM_CHASH;
		PufData.Aux = XPUF_RAM_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
	}

	Status = XPuf_Regeneration(&PufData);

	if (Status != XST_SUCCESS) {
		xPuf_printf(XPUF_DEBUG_INFO,
			"Puf Regeneration example failed with error : %x\r\n",
			 Status);
		goto END;
	}

	if (PufData.PufOperation == XPUF_REGEN_ID_ONLY) {
		xPuf_printf(XPUF_DEBUG_INFO,
		"PUF ID only regeneration is done!!\r\n");
		xPuf_printf(XPUF_DEBUG_INFO,"PUF ID : ");
		for (Idx = 0; Idx < XPUF_ID_LENGTH; Idx++) {
			xPuf_printf(XPUF_DEBUG_INFO, "%02x", PufData.PufID[Idx]);
		}
		xPuf_printf(XPUF_DEBUG_INFO, "\r\n");
	}
	else {
		xPuf_printf(XPUF_DEBUG_INFO,
			"PUF On Demand regeneration is done!!\r\n");
	}
	xPuf_printf(XPUF_DEBUG_INFO,
		"Successfully ran Puf Regeneration example!!\r\n");
END:
	return Status;
}
