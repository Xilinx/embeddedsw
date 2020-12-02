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
 * 1.0   ka   01/08/2019   Initial release of Puf_regeneration example
 *       ka   01/13/2020   Added "successfully ran" golden string
 * 1.1   har  03/01/2020   Added ID only regeneration support as default option
 * 1.2   har  07/03/2020   Added XPuf_ShowData and replaced XPUF_ID_LENGTH with
 *                         XPUF_ID_LEN_IN_BYTES for printing PUF ID
 *       am   08/14/2020   Replacing local status variable from u32 to int.
 *       har  09/30/2020   Replaced XPuf_printf with xil_printf
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
 * application.
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
 * #define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)
 * This option should be configured as TRUE to enable Global Variation Filter.
 * It is recommended to always enable this option to ensure entropy. It can
 * be configured as FALSE to disable Global Variation Filter.
 *
 ******************************************************************************/
/***************************** Include Files *********************************/
#include "xpuf.h"
#include "xstatus.h"
#include "xil_printf.h"

/* User Configurable parameters start */
#define XPUF_REGEN_OPTION			(XPUF_REGEN_ID_ONLY)
#define XPUF_READ_HD_OPTION			(XPUF_READ_FROM_RAM)
#define XPUF_CHASH				(0x00000000)
#define XPUF_AUX				(0x00000000)
#define XPUF_SYN_DATA_ADDRESS			(0x00000000)
#define XPUF_GLBL_VAR_FLTR_OPTION	(TRUE)
/* User Configurable parameters end */

#define XPUF_ID_LEN_IN_BYTES			(XPUF_ID_LEN_IN_WORDS * \
							 XPUF_WORD_LENGTH)

/************************** Type Definitions **********************************/

/************************** Function Prototypes ******************************/
static void XPuf_ShowData(const u8* Data, u32 Len);

/************************** Function Definitions *****************************/
int main(void)
{
	int Status = XST_FAILURE;
	XPuf_Data PufData;

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.PufOperation = XPUF_REGEN_OPTION;
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;

	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufData.Chash = XPUF_CHASH;
		PufData.Aux = XPUF_AUX;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
	}

	Status = XPuf_Regeneration(&PufData);

	if (Status != XST_SUCCESS) {
		xil_printf("Puf Regeneration example failed with error : %x\r\n",
			Status);
		goto END;
	}

	if (PufData.PufOperation == XPUF_REGEN_ID_ONLY) {
		xil_printf("PUF ID only regeneration is done!!\r\n");
	}
	else {
		xil_printf("PUF On Demand regeneration is done!!\r\n");
	}
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufData.PufID, XPUF_ID_LEN_IN_BYTES);
	xil_printf("Successfully ran Puf Regeneration example!!\r\n");
END:
	return Status;
}

/******************************************************************************/
/**
 *
 * @brief	This function prints the data array.
 *
 * @param	Data - Pointer to the data to be printed.
 * @param	Len  - Length of the data in bytes.
 *
 * @return	None.
 *
 ******************************************************************************/
static void XPuf_ShowData(const u8* Data, u32 Len)
{
	u32 Index;

	for (Index = 0U; Index < Len; Index++) {
		xil_printf("%02x", Data[Index]);
	}
	xil_printf("\r\n");
}
