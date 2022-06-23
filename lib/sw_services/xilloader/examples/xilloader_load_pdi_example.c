/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilloader_load_pdi_example.c
 *
 * This file illustrates loading partial pdi via IPI command.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   ----------------------------------------------------
 * 1.0   bsv  06/23/2022   Initial release
 */
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xipipsu.h"
#include "xil_cache.h"

#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define IPI_TIMEOUT		(0xFFFFFFFFU)
#define PDI_SRC_ADDR_LOW	(0x1000000U)
#define PDI_SRC_ADDR_HIGH	(0U)


static int DoIpiTest(void);

static 	XIpiPsu IpiInst;

int main()
{
	int Status = XST_FAILURE;

	Status = DoIpiTest();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran Xilloader_Add_ImageStore_Pdi example\n\r");
	}

	return Status;
}

static int DoIpiTest(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;
	u32 Response;

/**
	Load Partial Pdi
*
*	Command: Load Partial Pdi
*	Reserved[31:25]=0	Security Flag[24]	Length[23:16]=3	XilLoader=7	CMD_XILLOADER_LOAD_PPDI=1
*	PdiSrc - 0x1 for QSPI24, 0x2 for QSPI32, 0x8 for OSPI, 0xF for DDR
*	High PDI Address
*       Low PDI Address
*       Pdi should be placed at the PDI address in the PDI source before running this example.
*
*/
	u32 Payload[] = {0x030701, 0xF, PDI_SRC_ADDR_HIGH, PDI_SRC_ADDR_LOW};

	Xil_DCacheDisable();

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(0U);
	if (NULL == IpiCfgPtr) {
		goto END;
	}

	/* Initialize with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
		IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		goto END;
	}

	/**
	 * Send a Message to TEST_TARGET and WAIT for ACK
	 */
	 Status = XIpiPsu_WriteMessage(&IpiInst, TARGET_IPI_INT_MASK, Payload,
		sizeof(Payload)/sizeof(u32), XIPIPSU_BUF_TYPE_MSG);
	if (Status != XST_SUCCESS) {
		xil_printf("Writing to IPI request buffer failed\n");
		goto END;
	}

	Status = XIpiPsu_TriggerIpi(&IpiInst, TARGET_IPI_INT_MASK);
	if (Status != XST_SUCCESS) {
		xil_printf("IPI trigger failed\n");
		goto END;
	}


	/* Wait until current IPI interrupt is handled by target module */
	Status = XIpiPsu_PollForAck(&IpiInst, TARGET_IPI_INT_MASK, IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		xil_printf("IPI Timeout expired\n");
		goto END;
	}

	Status = XIpiPsu_ReadMessage(&IpiInst, TARGET_IPI_INT_MASK, &Response,
		sizeof(Response)/sizeof(u32), XIPIPSU_BUF_TYPE_RESP);
	if (XST_SUCCESS != Status) {
		xil_printf("Reading from IPI response buffer failed\n");
		goto END;
	}

	Status = (int)Response;

END:
	return Status;
}
