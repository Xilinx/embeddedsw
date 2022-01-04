/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilpuf_regeneration_client_example.c
 *
 * This file illustrates PUF regeneration.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   -----------------------------------------------------
 * 1.0   kpt  01/04/2022   Initial release of Puf_regeneration example
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
#include "xpuf_client.h"
#include "xpuf_defs.h"
#include "xstatus.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xpuf_ipi.h"

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
	XPuf_DataAddr PufData __attribute__ ((aligned (64))) = {0U};
	XPuf_PufData PufArr;
	XIpiPsu IpiInst;

	Status = XPuf_InitializeIpi(&IpiInst);
	if  (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XPuf_SetIpi(&IpiInst);
	if (Status != XST_SUCCESS) {
		goto END;
	}

	PufData.ShutterValue = XPUF_SHUTTER_VALUE;
	PufData.RegMode = XPUF_SYNDROME_MODE_4K;
	PufData.PufOperation = XPUF_REGEN_OPTION;
	PufData.ReadOption = XPUF_READ_HD_OPTION;
	PufData.GlobalVarFilter = XPUF_GLBL_VAR_FLTR_OPTION;
	PufData.PufIDAddr = (u64)(UINTPTR)PufArr.PufID;

	if (PufData.ReadOption == XPUF_READ_FROM_RAM) {
		PufArr.Chash = XPUF_CHASH;
		PufArr.Aux = XPUF_AUX;
		PufData.ChashAddr = (u64)(UINTPTR)&PufArr.Chash;
		PufData.AuxAddr = (u64)(UINTPTR)&PufArr.Aux;
		PufData.SyndromeAddr = XPUF_SYN_DATA_ADDRESS;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PufData, sizeof(PufData));
	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));

	Status = XPuf_Regeneration((u64)(UINTPTR)&PufData);

	if (Status != XST_SUCCESS) {
		xil_printf("Puf Regeneration example failed with error : %x\r\n",
			Status);
		goto END;
	}

	Xil_DCacheInvalidateRange((UINTPTR)&PufArr, sizeof(PufArr));

	if (PufData.PufOperation == XPUF_REGEN_ID_ONLY) {
		xil_printf("PUF ID only regeneration is done!!\r\n");
	}
	else {
		xil_printf("PUF On Demand regeneration is done!!\r\n");
	}
	xil_printf("PUF ID : ");
	XPuf_ShowData((u8*)PufArr.PufID, XPUF_ID_LEN_IN_BYTES);
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
