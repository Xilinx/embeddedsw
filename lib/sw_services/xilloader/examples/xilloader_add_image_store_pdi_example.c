/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilloader_add_image_store_pdi_example.c
 *
 * This file illustrates updating multiboot register via IPI command.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   ----------------------------------------------------
 * 1.0   bsv  04/20/2022   Initial release
 *       bsv  08/18/2022   Fix typo in CmdId
 */
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xipipsu.h"
#include "xil_cache.h"

#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#define IPI_TIMEOUT		(0xFFFFFFFFU)

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
	Add ImageStore PDI
*
*	Command: Add ImageStore PDI
*	Reserved[31:25]=0	Security Flag[24]	Length[23:16]=2	XilLoader=7	CMD_ADD_IMG_STORE_PDI=9
*	High PDI Address
*	Low PDI Address
*
*	This command adds PDI address to the list of Image Store PDIs that are maintained by PLM. During restore or reload of a image,
*	PLM checks this dynamically added list of PDIs first to get the required image and in case of any failure, it falls back to next possible.
*	If no valid entry is present, it uses boot pdi, which is the first entry in the list.
*/
	u32 Payload[] = {0x020709, 0, 0x10000000};

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
