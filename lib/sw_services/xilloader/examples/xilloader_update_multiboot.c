/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/

/*****************************************************************************/
/**
 *
 * @file xilloader_update_multiboot_example.c
 *
 * This file illustrates updating multiboot register via IPI command.
 *
 * <pre>
 * MODIFICATION HISTORY:
 *
 * Ver   Who   Date        Changes
 * ----- ---  ----------   ----------------------------------------------------
 * 1.0   bsv  04/17/2022   Initial release
 * 1.1   sk   04/18/2023   Added support for versalnet
 *       ng   06/21/2023   Added support for system device-tree flow
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

static 	XIpiPsu IpiInst;
static int DoIpiTest(void);

int main()
{
	int Status = XST_FAILURE;

	Status = DoIpiTest();
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran Xilloader_Update_Multiboot example\n\r");
	}

	return Status;
}

static int DoIpiTest(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;
	u32 Response = 0U;

/**
 *	Update Multiboot:
 *
 *	Command: Update Multiboot
 *	Reserved[31:25]=0, Security Flag[24], Length[23:16]=2, XilLoader=7, CMD_UPDATE_MULTIBOOT=8
 *	Reserved[31:16]=0, BootMode[15:8], Reserved[7:4]=0, [3:0] - Flash Type - 0: RAW, 1: FS, 2: RAW BP1, 3: RAW BP2
 *	Image location ( In case of SD/eMMC File System - File Number, Remaining cases - PDI Location in the device )
 */
	u32 Payload[] = {0x020708, 0x200, 0x4000000};

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
