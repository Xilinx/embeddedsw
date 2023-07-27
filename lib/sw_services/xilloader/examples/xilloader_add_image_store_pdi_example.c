/******************************************************************************
* Copyright (c) 2018-2022 Xilinx, Inc.  All rights reserved.
* Copyright (c) 2022-2023, Advanced Micro Devices, Inc.  All rights reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilloader_add_image_store_pdi_example.c
 *
 * This file illustrates adding PDI to ImageStore via IPI command.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   ----------------------------------------------------
 * 1.0   bsv  04/20/2022   Initial release
 *       bsv  08/18/2022   Fix typo in CmdId
 * 1.1   sk   03/10/2023   Updated changes to command format
 *       sk   04/18/2023   Added support for versalnet
 * 1.9   ng   06/21/2023   Added support for system device-tree flow
 *       sk   07/26/2023   Updated IPI device ID
 */
#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xipipsu.h"
#include "xil_cache.h"

#ifdef SDT
#define IPI_DEVICE (XPAR_XIPIPSU_0_BASEADDR)
#else
#define IPI_DEVICE (XPAR_XIPIPSU_0_DEVICE_ID)
#endif

#if defined(VERSAL_NET)
#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK
#else
#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#endif

#define IPI_TIMEOUT		(0xFFFFFFFFU)

/* Example defines below, update with required values*/
#define PDI_ID 0x03  /*ID to identify PDI */
#define PDI_SIZE_IN_WORDS 0x4000U  /* PDI Size in words i.e (size/4) */
#define PDI_ADDRESS_LOW 0x00100000U /* PDI Low Address in Memory */
#define PDI_ADDRESS_HIGH 0x00000000U /* PDI High Address in Memory */

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
*	Reserved[31:25]=0	Security Flag[24]	Length[23:16]=4	XilLoader=7	CMD_ADD_IMG_STORE_PDI=9
*	PDI ID
*	High PDI Address
*	Low PDI Address
*	PDI Size( In Words )
*
*	This command adds PDI to the list of Image Store PDIs that are maintained by PLM. During restore or reload of a image,
*	PLM checks this dynamically added list of PDIs first to get the required image and in case of any failure, it falls back to next possible.
*	If no valid entry is present, it uses boot pdi, which is the first entry in the list.
*/
	u32 Payload[] = {0x040709, PDI_ID, PDI_ADDRESS_HIGH, PDI_ADDRESS_LOW, PDI_SIZE_IN_WORDS};

	Xil_DCacheDisable();

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(IPI_DEVICE);
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
