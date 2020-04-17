/******************************************************************************
* Copyright (c) 2019 - 2020 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
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
 * #define XPUF_REGEN_OPTION			(XPUF_REGEN_ID_ONLY)
 *							(or)
 *						(XPUF_REGEN_ON_DEMAND)
 * This selects the type of PUF regeneration. It is configured as REGEN_ID_ONLY
 * by default.
 *
 * #define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
 *							(or)
 *						(XPUF_READ_FROM_EFUSE_CACHE)
 * This selects the location from where the helper data must be read by the
 * application. This option must be configured if XPUF_KEY_GENERATE_OPTION
 * is configured as XPUF_REGEN_ON_DEMAND.
 *
 * #define XPUF_CHASH				(0x00000000)
 * The length of CHASH should be 24 bits. It is valid only for PUF regeneration
 * and invalid for PUF registration. CHASH value should be supplied if
 * XPUF_READ_HD_OPTION is configured as XPUF_READ_FROM_RAM.
 *
 * #define XPUF_AUX				(0x00000000)
 * The length of AUX should be 32 bits. It is valid only for PUF regeneration
 * and invalid for PUF registration. AUX value should be supplied if
 * XPUF_READ_HD_OPTION is configured as XPUF_READ_FROM_RAM.
 *
 * #define XPUF_SYN_DATA_ADDRESS		(0x00000000)
 * Address of syndrome data should be supplied if XPUF_READ_HD_OPTION is
 * configured as XPUF_READ_FROM_RAM.
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"

/* User Configurable parameters */
#define XPUF_REGEN_OPTION			(XPUF_REGEN_ID_ONLY)
#define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
#define XPUF_CHASH				(0x00000000)
#define XPUF_AUX				(0x00000000)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000)
#define XPUF_DEBUG_INFO				(1U)

/************************** Type Definitions **********************************/
static XPuf_Data PufData;

/************************** Function Definitions *****************************/
int main()
{
	int Idx;
	u32 Status = XST_FAILURE;

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.PufOperation = XPUF_REGEN_OPTION;
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_CHASH;
		PufData.Aux = XPUF_AUX;
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