/******************************************************************************
* Copyright (c) 2022 Xilinx, Inc.  All rights reserved.
* Copyright (C) 2022 - 2023 Advanced Micro Devices, Inc. All Rights Reserved.
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
 *       bsv  06/28/2022   Rename and reorganize functions
 * 1.1   sk   04/18/2023   Added support for versalnet
 *       ng   06/21/2023   Added support for system device-tree flow
 * </pre>
 *
 * @note
 *
 *****************************************************************************/

/***************************** Include Files *********************************/
#include <stdio.h>
#include "xil_printf.h"
#include "xparameters.h"
#include "xipipsu.h"
#include "xil_cache.h"

/************************** Constant Definitions *****************************/
#if defined(VERSAL_NET)
#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSX_PMC_0_CH0_MASK
#else
#define TARGET_IPI_INT_MASK	XPAR_XIPIPS_TARGET_PSV_PMC_0_CH0_MASK
#endif


#define IPI_TIMEOUT		(0xFFFFFFFFU)
#define PDI_SRC_ADDR_LOW	(0x1000000U)
#define PDI_SRC_ADDR_HIGH	(0U)
#define XLOADER_PDI_SRC_DDR	(0xFU)
#define HEADER(Len, ModuleId, CmdId)	((Len << 16U) | (ModuleId << 8U) | CmdId)
#define LOAD_PDI_CMD_PAYLOAD_LEN	(3U)
#define XILLOADER_MODULE_ID		(7U)
#define XILLOADER_LOAD_PPDI_CMD_ID	(1U)

/**************************** Type Definitions *******************************/

/***************** Macros (Inline Functions) Definitions *********************/

#ifdef SDT
#define IOMODULE_DEVICE (XPAR_XIOMODULE_0_BASEADDR)
#else
#define IOMODULE_DEVICE (XPAR_IOMODULE_0_DEVICE_ID)
#endif

/************************** Function Prototypes ******************************/
static int XilLoaderLoadPdiTest(void);

/************************** Variable Definitions *****************************/
static 	XIpiPsu IpiInst;

/*****************************************************************************/
/**
* @brief	This function  will initialize IPI interface.
*
* @return   	XST_SUCCESS on success and XST_FAILURE on failure
*
******************************************************************************/
static int IpiInit(void)
{
	int Status = XST_FAILURE;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(IOMODULE_DEVICE);
	if (NULL == IpiCfgPtr) {
		goto END;
	}

	/* Initialize with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(&IpiInst, IpiCfgPtr,
		IpiCfgPtr->BaseAddress);

END:
	return Status;
}

/*****************************************************************************/
/**
* @brief	This is the main function.
*
* @return   	XST_SUCCESS on success and XST_FAILURE on failure
*
******************************************************************************/
int main()
{
	int Status = XST_FAILURE;

	xil_printf("Xilloader_Load_Pdi example started\n\r");

	Xil_DCacheDisable();

	Status = IpiInit();
	if (Status != XST_SUCCESS) {
		goto END;
	}

	Status = XilLoaderLoadPdiTest();
END:
	if (Status == XST_SUCCESS) {
		xil_printf("Successfully ran Xilloader_Load_Pdi example\n\r");
	}
	else {
		xil_printf("Xilloader_Load_Pdi example failed\n\r");
	}

	return Status;
}

/*****************************************************************************/
/**
* @brief	This function runs the LoadPdi test.
*
* @return   	XST_SUCCESS on success and XST_FAILURE on failure
*
******************************************************************************/
static int XilLoaderLoadPdiTest(void)
{
	int Status = XST_FAILURE;

	u32 Response;

/**
*	Load Partial Pdi
*
*	Command: Load Partial Pdi
*	Reserved[31:25]=0	Security Flag[24]	Length[23:16]=3	XilLoader=7	CMD_XILLOADER_LOAD_PPDI=1
*	PdiSrc - 0x1 for QSPI24, 0x2 for QSPI32, 0x8 for OSPI, 0xF for DDR
*	High PDI Address
*       Low PDI Address
*       Pdi should be placed at the PDI address in the PDI source before running this example.
*
*/
	u32 Payload[] = {HEADER(LOAD_PDI_CMD_PAYLOAD_LEN, XILLOADER_MODULE_ID,
		XILLOADER_LOAD_PPDI_CMD_ID), XLOADER_PDI_SRC_DDR,
		PDI_SRC_ADDR_HIGH, PDI_SRC_ADDR_LOW};

	 /* Check if there is any pending IPI in progress */
	Status = XIpiPsu_PollForAck(&IpiInst, TARGET_IPI_INT_MASK, IPI_TIMEOUT);
	if (XST_SUCCESS != Status) {
		xil_printf("IPI Timeout expired\n");
		goto END;
	}

	/**
	 * Send a Message to TEST_TARGET and WAIT for ACK
	 */
	Status = XIpiPsu_WriteMessage(&IpiInst, TARGET_IPI_INT_MASK, Payload,
		sizeof(Payload) / sizeof(u32), XIPIPSU_BUF_TYPE_MSG);
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
